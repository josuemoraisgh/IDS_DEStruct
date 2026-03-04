#include "data_service.h"
#include "../threading/shared_state.h"
#include "../threading/thread_worker.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <math.h>
#include <algorithm>
#include <cmath>

#define TAMMAXCARACTER 900000

namespace {

///////////////////////////////////////////////////////////////////////////////
// Utilitarios de pre-processamento e decimacao adaptativa.
///////////////////////////////////////////////////////////////////////////////
static constexpr double kPi = 3.14159265358979323846;
static constexpr double kEps = 1e-12;

///////////////////////////////////////////////////////////////////////////////
// Padronização z-score: z = (x - μ) / σ
static qreal standardizeZScore(const qreal value, const qreal vmean, const qreal vstd)
{
    if (vstd < 1e-12) return 0.0; // Evitar divisão por zero
    return (value - vmean) / vstd;
}

///////////////////////////////////////////////////////////////////////////////
static bool isTimeVariableName(const QString &name)
{
    const QString upper = name.trimmed().toUpper();
    return (upper == "TIME") || (upper == "TEMPO");
}

///////////////////////////////////////////////////////////////////////////////
static double sinc(const double x)
{
    if (std::fabs(x) < 1e-14) return 1.0;
    return std::sin(kPi * x) / (kPi * x);
}

///////////////////////////////////////////////////////////////////////////////
static int mirrorIndex(int idx, const int size)
{
    if (size <= 1) return 0;
    while ((idx < 0) || (idx >= size)) {
        if (idx < 0) idx = -idx;
        if (idx >= size) idx = (2 * size - 2) - idx;
    }
    return idx;
}

///////////////////////////////////////////////////////////////////////////////
static QVector<double> centerSignal(const QVector<double> &x)
{
    if (x.isEmpty()) return x;
    double mean = 0.0;
    for (int i = 0; i < x.size(); ++i) mean += x.at(i);
    mean /= static_cast<double>(x.size());

    QVector<double> centered(x.size(), 0.0);
    for (int i = 0; i < x.size(); ++i) centered[i] = x.at(i) - mean;
    return centered;
}

///////////////////////////////////////////////////////////////////////////////
static double autocorrNormalizedAtLag(const QVector<double> &yCentered, int lag)
{
    const int N = yCentered.size();
    if (N <= 1) return 0.0;
    lag = qBound(0, lag, N - 1);

    double den = kEps;
    for (int k = 0; k < N; ++k) den += yCentered.at(k) * yCentered.at(k);

    if (lag == 0) return 1.0;
    double num = 0.0;
    for (int k = lag; k < N; ++k) num += yCentered.at(k) * yCentered.at(k - lag);
    return num / den;
}

///////////////////////////////////////////////////////////////////////////////
int EstimateDFromAutocorr(const QVector<double>& y, double thr = 0.95, int maxLag = 50, int Dmax = 20)
{
    if (y.size() < 4) return 1;
    const QVector<double> yCentered = centerSignal(y);
    const int lagMax = qMin(maxLag, yCentered.size() - 1);
    for (int lag = 1; lag <= lagMax; ++lag) {
        const double rho = autocorrNormalizedAtLag(yCentered, lag);
        if (rho < thr) return qBound(2, lag, Dmax);
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
QVector<double> DesignFIRLowpassHamming(int N = 101, double fc_norm = 0.1)
{
    if (N < 3) N = 3;
    if ((N % 2) == 0) N += 1;

    const double fc = qBound(1e-6, fc_norm, 0.999); // Nyquist = 1.
    const int M = (N - 1) / 2;
    QVector<double> h(N, 0.0);
    double sumH = 0.0;

    for (int n = 0; n < N; ++n) {
        const double m = static_cast<double>(n - M);
        const double ideal = 2.0 * fc * sinc(2.0 * fc * m);
        const double w = 0.54 - 0.46 * std::cos((2.0 * kPi * n) / (N - 1));
        h[n] = ideal * w;
        sumH += h[n];
    }

    if (std::fabs(sumH) < 1e-14) {
        h.fill(0.0);
        h[M] = 1.0;
        return h;
    }
    for (int n = 0; n < N; ++n) h[n] /= sumH; // Normaliza ganho DC.
    return h;
}

///////////////////////////////////////////////////////////////////////////////
QVector<double> FIRFilterMirror(const QVector<double>& x, const QVector<double>& h)
{
    QVector<double> y(x.size(), 0.0);
    if (x.isEmpty() || h.isEmpty()) return y;

    const int N = x.size();
    const int L = h.size();
    const int M = L / 2;

    for (int n = 0; n < N; ++n) {
        double acc = 0.0;
        for (int k = 0; k < L; ++k) {
            const int idx = mirrorIndex(n - (k - M), N);
            acc += h.at(k) * x.at(idx);
        }
        y[n] = acc;
    }
    return y;
}

///////////////////////////////////////////////////////////////////////////////
QVector<double> FIRFiltFiltMirror(const QVector<double>& x, const QVector<double>& h)
{
    if (x.isEmpty() || h.isEmpty()) return x;
    QVector<double> y1 = FIRFilterMirror(x, h);
    std::reverse(y1.begin(), y1.end());
    QVector<double> y2 = FIRFilterMirror(y1, h);
    std::reverse(y2.begin(), y2.end());
    return y2;
}

///////////////////////////////////////////////////////////////////////////////
QVector<double> Decimate(const QVector<double>& x, int D)
{
    if (x.isEmpty()) return QVector<double>();
    D = qMax(1, D);
    if (D <= 1) return x;

    QVector<double> y;
    y.reserve((x.size() + D - 1) / D);
    for (int k = 0; k < x.size(); k += D) y.append(x.at(k));
    return y;
}

///////////////////////////////////////////////////////////////////////////////
static QVector<double> extractRowAsDouble(const JMathVar<qreal> &mat, qint32 row)
{
    QVector<double> out;
    out.reserve(mat.numColunas());
    for (qint32 col = 0; col < mat.numColunas(); ++col) out.append(mat.at(row, col));
    return out;
}

///////////////////////////////////////////////////////////////////////////////
static void buildAdaptiveDecimationCache(SharedState *state)
{
    state->dadosFiltradosPorSaida.clear();
    state->dadosFiltradosDecimacao.clear();

    const qint32 qtSaidas = state->Adj.Dados.variaveis.qtSaidas;
    const qint32 nlinha = state->Adj.Dados.variaveis.valores.numLinhas();
    const qint32 ncoluna = state->Adj.Dados.variaveis.valores.numColunas();
    if ((qtSaidas <= 0) || (nlinha <= 0) || (ncoluna <= 0)) return;

    state->dadosFiltradosPorSaida.resize(qtSaidas);
    state->dadosFiltradosDecimacao.resize(qtSaidas);

    for (qint32 idSaida = 0; idSaida < qtSaidas; ++idSaida) {
        const qint32 D = (idSaida < state->Adj.decimacao.size())
            ? qMax(1, state->Adj.decimacao.at(idSaida))
            : 1;
        state->dadosFiltradosDecimacao[idSaida] = D;

        const QVector<double> yOrig = extractRowAsDouble(state->Adj.Dados.variaveis.valores, idSaida);
        const QVector<double> yCentered = centerSignal(yOrig);
        const double rho1 = autocorrNormalizedAtLag(yCentered, qMin(1, yCentered.size() > 1 ? yCentered.size() - 1 : 0));
        const double rhoD = autocorrNormalizedAtLag(yCentered, qMin(D, yCentered.size() > 1 ? yCentered.size() - 1 : 0));

        if (D <= 1) {
            qDebug().noquote() << QString("Adaptive decimation: D=%1, rho(1)=%2, rho(D)=%3, N: %4 -> %5")
                                  .arg(D)
                                  .arg(rho1, 0, 'g', 8)
                                  .arg(rhoD, 0, 'g', 8)
                                  .arg(ncoluna)
                                  .arg(ncoluna);
            qDebug().noquote() << QString("FIR: len=101, fc=0.8/%1, filtfilt=ON (bypass D=1)").arg(D);
            continue;
        }

        const double fc_norm = 0.8 / static_cast<double>(D);
        const QVector<double> h = DesignFIRLowpassHamming(101, fc_norm);

        QVector<QVector<double> > linhasDecimadas(nlinha);
        qint32 nAfter = -1;
        for (qint32 row = 0; row < nlinha; ++row) {
            const QVector<double> x = extractRowAsDouble(state->Adj.Dados.variaveis.valores, row);
            bool isTime = false;
            if (row < state->Adj.Dados.variaveis.nome.size())
                isTime = isTimeVariableName(state->Adj.Dados.variaveis.nome.at(row));

            const QVector<double> xProc = isTime ? x : FIRFiltFiltMirror(x, h);
            linhasDecimadas[row] = Decimate(xProc, D);
            if (nAfter < 0) nAfter = linhasDecimadas[row].size();
            else nAfter = qMin<qint32>(nAfter, linhasDecimadas[row].size());
        }

        if (nAfter <= 0) continue;

        JMathVar<qreal> dadosDecimados(nlinha, nAfter, 0.0);
        for (qint32 row = 0; row < nlinha; ++row)
            for (qint32 col = 0; col < nAfter; ++col)
                dadosDecimados(row, col) = static_cast<qreal>(linhasDecimadas[row].at(col));

        state->dadosFiltradosPorSaida[idSaida] = dadosDecimados;

        qDebug().noquote() << QString("Adaptive decimation: D=%1, rho(1)=%2, rho(D)=%3, N: %4 -> %5")
                              .arg(D)
                              .arg(rho1, 0, 'g', 8)
                              .arg(rhoD, 0, 'g', 8)
                              .arg(ncoluna)
                              .arg(nAfter);
        qDebug().noquote() << QString("FIR: len=101, fc=0.8/%1, filtfilt=ON").arg(D);
    }
}

///////////////////////////////////////////////////////////////////////////////
static void normalizeDecimationCache(SharedState *state)
{
    for (qint32 idSaida = 0; idSaida < state->dadosFiltradosPorSaida.size(); ++idSaida) {
        if ((idSaida >= state->dadosFiltradosDecimacao.size())
            || (state->dadosFiltradosDecimacao.at(idSaida) <= 1))
            continue;

        JMathVar<qreal> &cache = state->dadosFiltradosPorSaida[idSaida];
        if ((cache.numLinhas() != state->Adj.Dados.variaveis.valores.numLinhas())
            || (cache.numColunas() <= 0))
            continue;

        for (qint32 col = 0; col < cache.numColunas(); ++col) {
            for (qint32 row = 0; row < cache.numLinhas(); ++row) {
                const qreal vmean = state->Adj.Dados.variaveis.Vmean.at(row);
                const qreal vstd = state->Adj.Dados.variaveis.Vstd.at(row);
                cache(row, col) = standardizeZScore(cache.at(row, col), vmean, vstd);
            }
        }
    }
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
DataService::DataService(SharedState *state, ThreadWorker *worker)
    : m_state(state)
    , m_worker(worker)
{
}

///////////////////////////////////////////////////////////////////////////////
/// loadData — DES_Carregar (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void DataService::loadData()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 nlinha = m_state->cVariaveis.size();
    const qint32 qtSaidas = m_state->Adj.Dados.variaveis.qtSaidas;
    bool isOk = false, isOkIni = false, isNumber = false, isNormalizado = false;
    qint32 index = 0, i = 0, j = 0, ncoluna = 0, idSaida = 0;
    QList<QString> nome;
    QString str;
    QList<qreal> posPonto,
        Vsum,        // Soma para calcular média
        Vsum2,       // Soma dos quadrados para calcular desvio padrão
        Vcount,      // Contador de amostras
        mediaY  = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList(),
        mediaY2 = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList();
    QVector<qreal> Mean1, Std1;
    ////////////////////////////////////////////////////////////////////////////
    QStringList strList, lineList;
    QByteArray line, lineMeio, lineDepois;
    QFile file(m_state->fileName);
    qint64 posicaoIni, posicaoFinal;
    ////////////////////////////////////////////////////////////////////////////
    if (!m_state->isCarregar) {
        Mean1 = m_state->Adj.Dados.variaveis.Vmean.toVector();
        Std1 = m_state->Adj.Dados.variaveis.Vstd.toVector();
    }
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread inicializa tamanho do vetor dos dados e do arquivo.
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->index[0] = 0;
        if (m_state->isCarregar) {
            m_state->Adj.Dados.variaveis.nome.clear();
            m_state->Adj.Dados.variaveis.valores.clear();
            m_state->Adj.decimacao.clear();
            m_state->Adj.talDecim.clear();
        }
        m_state->dadosFiltradosPorSaida.clear();
        m_state->dadosFiltradosDecimacao.clear();
        m_state->Adj.Dados.variaveis.Vmean.clear();
        m_state->Adj.Dados.variaveis.Vstd.clear();
        m_state->mediaY  = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList();
        m_state->mediaY2 = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList();
        if (file.open(QFile::ReadOnly)) {
            m_state->tamArquivo = file.size();
            file.close();
        } else
            qDebug() << "Func:loadData - Nao abriu arquivo para ler tamanho";
        emit m_worker->signal_Status(0);
        // Le a variavel sem o QReadWriteLock pois apenas uma thread esta rodando.
        m_state->lock_modeOper_TH.lockForRead();
        isOk = m_state->Adj.modeOper_TH == 2;
        m_state->lock_modeOper_TH.unlock();
        if (isOk) m_state->waitSync.wait(&m_state->mutex);
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    m_state->lock_modeOper_TH.lockForRead();
    isOk = m_state->Adj.modeOper_TH <= 1;
    m_state->lock_modeOper_TH.unlock();
    if (isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    // Monta a divisao para cada thread
    const qint32 thId = m_worker->threadId();
    const qint32 tamCadaTh = ((thId + 1) == m_state->TH_size)
        ? m_state->tamArquivo - (thId * (m_state->tamArquivo / m_state->TH_size))
        : (m_state->tamArquivo / m_state->TH_size);
    const qint32 tamCadaRepet = (tamCadaTh > TAMMAXCARACTER) ? TAMMAXCARACTER : tamCadaTh;
    const qint32 numRepet = (tamCadaTh / tamCadaRepet) + (tamCadaTh % tamCadaRepet ? 1 : 0);
    ////////////////////////////////////////////////////////////////////////////
    // Le do arquivo os dados referentes a esta thread.
    posicaoFinal = thId * (m_state->tamArquivo / m_state->TH_size);
    if (file.open(QFile::ReadOnly)) {
        for (index = 0; index < numRepet; index++) {
            ////////////////////////////////////////////////////////////////////////////
            posicaoIni = posicaoFinal;
            posicaoFinal = (posicaoIni + tamCadaRepet);
            posicaoFinal = posicaoFinal > m_state->tamArquivo
                ? m_state->tamArquivo
                : posicaoFinal > ((thId + 1) * (m_state->tamArquivo / m_state->TH_size))
                    ? ((thId + 1) * (m_state->tamArquivo / m_state->TH_size))
                    : posicaoFinal;
            ////////////////////////////////////////////////////////////////////////////
            m_state->lock_modeOper_TH.lockForRead();
            isOk = m_state->Adj.modeOper_TH <= 1;
            m_state->lock_modeOper_TH.unlock();
            if (isOk) return;
            ////////////////////////////////////////////////////////////////////////////
            m_state->lock_modeOper_TH.lockForRead();
            isOk = m_state->Adj.modeOper_TH == 2;
            m_state->lock_modeOper_TH.unlock();
            m_state->mutex.lock();
            if (isOk) m_state->waitSync.wait(&m_state->mutex);
            m_state->mutex.unlock();
            ////////////////////////////////////////////////////////////////////////////
            file.seek(posicaoIni ? posicaoIni - 1 : 0);
            lineMeio = file.read(posicaoFinal - posicaoIni + (posicaoIni ? 1 : 0));
            lineDepois.clear();
            if (lineMeio.size() ? (lineMeio.right(1) != "\n") && (lineMeio.right(1) != "\0") : true) {
                line = file.read(1);
                while ((line != "\n") && (line != "\0")) {
                    lineDepois.append(line);
                    line = file.read(1);
                }
            }
            if (thId || index) {
                line = lineMeio.left(1);
                while ((line != "\n") && (line != "\0")) {
                    lineMeio.remove(0, 1);
                    line = lineMeio.left(1);
                }
            }
            ////////////////////////////////////////////////////////////////////////////
            line = lineMeio.right(40);
            lineList = QString(lineMeio + lineDepois).remove('\r').split('\n', Qt::SkipEmptyParts);
            ////////////////////////////////////////////////////////////////////////////
            for (i = 0; i < lineList.size(); i++) {
                if (lineList.at(i).size()) {
                    strList = lineList.at(i).split(QString(QChar(32)), Qt::SkipEmptyParts);
                    if (strList.size() ? strList.first().isEmpty() : false)
                        strList.removeFirst();
                    if (strList.size() >= m_state->cVariaveis.size()) {
                        for (j = 0, isNumber = true; (isNumber) && (j < strList.size()); j++)
                            strList.at(j).toDouble(&isNumber);
                        if (j == strList.size()) {
                            for (j = 0; j < nlinha; j++) {
                                posPonto.append((QString().setNum(strList.at(m_state->cVariaveis.at(j)).toDouble(), 'E', 9)).toDouble());
                                if (j < qtSaidas) {
                                    mediaY[j] += posPonto.last();
                                    mediaY2[j] += posPonto.last() * posPonto.last();
                                }
                                if (!m_state->Adj.Dados.variaveis.Vmean.size()) {
                                    if (!isOkIni) {
                                        Vsum.append(posPonto.last());
                                        Vsum2.append(posPonto.last() * posPonto.last());
                                        Vcount.append(1.0);
                                        isOkIni = (Vsum.size() >= nlinha);
                                    } else {
                                        Vsum[j] += posPonto.last();
                                        Vsum2[j] += posPonto.last() * posPonto.last();
                                        Vcount[j] += 1.0;
                                    }
                                } else if (!isNormalizado)
                                    isNormalizado = true;
                            }
                        } else {
                            for (j = 0; j < nlinha; j++) {
                                str = strList.at(m_state->cVariaveis.at(j));
                                QRegularExpression rx("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*),([-+]?\\d*)\\)");
                                QRegularExpressionMatch match = rx.match(str);
                                if (match.hasMatch()) {
                                    m_state->Adj.Dados.variaveis.Vmean.append(match.captured(1).toDouble());
                                    m_state->Adj.Dados.variaveis.Vstd.append(match.captured(2).toDouble());
                                    m_state->Adj.decimacao.append(match.captured(3).toInt());
                                    str.replace(rx, "");
                                } else {
                                    QRegularExpression rx2("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\)");
                                    match = rx2.match(str);
                                    if (match.hasMatch()) {
                                        m_state->Adj.Dados.variaveis.Vmean.append(match.captured(1).toDouble());
                                        m_state->Adj.Dados.variaveis.Vstd.append(match.captured(2).toDouble());
                                        str.replace(rx2, "");
                                    }
                                }
                                str.replace("(", "");
                                str.replace(")", "");
                                nome.append(str);
                            }
                        }
                    }
                }
            }
        }
        file.close();
    } else
        qDebug() << "Func:loadData - Nao abriu arquivo para ler dados";
    lineList.clear();
    ////////////////////////////////////////////////////////////////////////////
    m_state->lock_modeOper_TH.lockForRead();
    isOk = m_state->Adj.modeOper_TH <= 1;
    m_state->lock_modeOper_TH.unlock();
    if (isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread roda para inicializar o index
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->index[0] = 0;
        emit m_worker->signal_Status(1);
        if (m_state->Adj.modeOper_TH == 2) m_state->waitSync.wait(&m_state->mutex);
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread roda por vez em ordem concatenando os dados
    m_state->mutex.lock();
    do {
        if (m_state->index[0] != thId) m_state->justSync.wait(&m_state->mutex);
        else {
            ncoluna = posPonto.size() / nlinha;
            m_state->Adj.Dados.variaveis.valores.append(posPonto, nlinha, ncoluna);
            for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                m_state->mediaY[idSaida] += mediaY.at(idSaida);
                m_state->mediaY2[idSaida] += mediaY2.at(idSaida);
            }
            posPonto.clear();
            m_state->index[0]++;
            if (!Vsum.empty() && !isNormalizado) {
                if (m_state->Adj.Dados.variaveis.Vmean.isEmpty()) {
                    // Inicializa listas com valores calculados acumulados
                    for (j = 0; j < Vsum.size(); ++j) {
                        m_state->Adj.Dados.variaveis.Vmean.append(Vsum.at(j));
                        m_state->Adj.Dados.variaveis.Vstd.append(Vsum2.at(j));
                    }
                    m_state->Vcount = Vcount.toVector();
                } else {
                    // Atualiza somas acumuladas
                    for (j = 0; j < Vsum.size(); ++j) {
                        m_state->Adj.Dados.variaveis.Vmean[j] += Vsum.at(j);
                        m_state->Adj.Dados.variaveis.Vstd[j] += Vsum2.at(j);
                        m_state->Vcount[j] += Vcount.at(j);
                    }
                }
            }
            if (!nome.empty() && (m_state->isCarregar || m_state->Adj.Dados.variaveis.nome.isEmpty()))
                m_state->Adj.Dados.variaveis.nome.append(nome);
            if (m_state->Adj.modeOper_TH == 2) m_state->waitSync.wait(&m_state->mutex);
            m_state->justSync.wakeAll();
        }
    } while (m_state->index[0] < m_state->TH_size);
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    m_state->lock_modeOper_TH.lockForRead();
    isOk = m_state->Adj.modeOper_TH <= 1;
    m_state->lock_modeOper_TH.unlock();
    if (isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread roda para abrir a tela de Normalizacao.
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->index[0] = 0;
        m_state->index[0] = 0;
        //////////////////////////////////////////////////////////////
        if (!isNormalizado) {
            // Finaliza cálculo de média e desvio padrão a partir das somas acumuladas
            if (!m_state->Vcount.isEmpty()) {
                for (j = 0; j < m_state->Adj.Dados.variaveis.Vmean.size(); ++j) {
                    const qreal count = m_state->Vcount.at(j);
                    if (count > 0) {
                        const qreal sum = m_state->Adj.Dados.variaveis.Vmean.at(j);
                        const qreal sum2 = m_state->Adj.Dados.variaveis.Vstd.at(j);
                        const qreal mean = sum / count;
                        const qreal variance = (sum2 / count) - (mean * mean);
                        const qreal std = std::sqrt(std::max(0.0, variance));
                        
                        m_state->Adj.Dados.variaveis.Vmean[j] = mean;
                        m_state->Adj.Dados.variaveis.Vstd[j] = std;
                    }
                }
                m_state->Vcount.clear(); // Limpa contador tempor\u00e1rio
            }
            
            // Calcula a Decimacao (auto D) e define como padrao inicial.
            m_state->Adj.decimacao.clear();
            m_state->Adj.talDecim.clear();
            for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                const QVector<double> ySerie = extractRowAsDouble(m_state->Adj.Dados.variaveis.valores, idSaida);
                const qint32 dAuto = EstimateDFromAutocorr(ySerie, 0.95, 50, 20);
                m_state->Adj.talDecim.append(dAuto);
                m_state->Adj.decimacao.append(dAuto);
            }
            emit m_worker->signal_Status(3);
        } else
            emit m_worker->signal_Status(4);
        emit m_worker->signal_Tam();
        m_state->lock_modeOper_TH.lockForRead();
        isOk = m_state->Adj.modeOper_TH == 2;
        m_state->lock_modeOper_TH.unlock();
        if (isOk) m_state->waitSync.wait(&m_state->mutex);
        m_state->lock_modeOper_TH.lockForWrite();
        m_state->Adj.modeOper_TH = 1; // Finaliza a tarefa colocando a thread em espera
        m_state->lock_modeOper_TH.unlock();
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////
/// normalizeData — slot_DES_Normalizar (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void DataService::normalizeData()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 nlinha = m_state->Adj.Dados.variaveis.valores.numLinhas();
    qint32 index = 0, j = 0, ncoluna = m_state->Adj.Dados.variaveis.valores.numColunas();
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread prepara o cache (filtro+decimacao) por saida antes da normalizacao.
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->index[0] = 0;
        buildAdaptiveDecimationCache(m_state);
        normalizeDecimationCache(m_state);
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    // Padronizando os dados (z-score).
    forever {
        m_state->lock_index[0].lockForWrite();
        index = m_state->index[0]++;
        m_state->lock_index[0].unlock();
        if (index < ncoluna) {
            for (j = 0; j < nlinha; j++) {
                const qreal vmean = m_state->Adj.Dados.variaveis.Vmean.at(j);
                const qreal vstd = m_state->Adj.Dados.variaveis.Vstd.at(j);
                m_state->Adj.Dados.variaveis.valores(j, index) =
                    standardizeZScore(m_state->Adj.Dados.variaveis.valores.at(j, index), vmean, vstd);
            }
        } else break;
    }
    ////////////////////////////////////////////////////////////////////////////
    // Apenas uma thread roda para fechar a tela
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->index[0] = 0;
        emit m_worker->signal_Status(4);
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    m_state->Adj.modeOper_TH = 1; // Finaliza a tarefa
    ////////////////////////////////////////////////////////////////////////////
}

