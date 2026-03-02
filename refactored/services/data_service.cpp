#include "data_service.h"
#include "../threading/shared_state.h"
#include "../threading/thread_worker.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <math.h>

#define TAMMAXCARACTER 900000

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
        Vmaior,
        Vmenor,
        mediaY  = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList(),
        mediaY2 = QVector<qreal>(m_state->Adj.Dados.variaveis.qtSaidas, 0.0f).toList();
    qreal talr  = 0,
          talr2 = 0,
          talMin = 0,
         *valor = NULL;
    QVector<qreal> Max1, Min1;
    ////////////////////////////////////////////////////////////////////////////
    QStringList strList, lineList;
    QByteArray line, lineMeio, lineDepois;
    QFile file(m_state->fileName);
    qint64 posicaoIni, posicaoFinal;
    ////////////////////////////////////////////////////////////////////////////
    if (!m_state->isCarregar) {
        Max1 = m_state->Adj.Dados.variaveis.Vmaior.toVector();
        Min1 = m_state->Adj.Dados.variaveis.Vmenor.toVector();
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
        }
        m_state->Adj.Dados.variaveis.Vmaior.clear();
        m_state->Adj.Dados.variaveis.Vmenor.clear();
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
                                if (!m_state->Adj.Dados.variaveis.Vmaior.size()) {
                                    if (!isOkIni) {
                                        Vmaior.append(posPonto.last());
                                        Vmenor.append(posPonto.last());
                                        isOkIni = (Vmaior.size() >= nlinha);
                                    } else {
                                        if (Vmaior.at(j) < posPonto.last()) Vmaior.replace(j, posPonto.last());
                                        if (Vmenor.at(j) > posPonto.last()) Vmenor.replace(j, posPonto.last());
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
                                    m_state->Adj.Dados.variaveis.Vmaior.append(match.captured(1).toDouble());
                                    m_state->Adj.Dados.variaveis.Vmenor.append(match.captured(2).toDouble());
                                    m_state->Adj.decimacao.append(match.captured(3).toInt());
                                    str.replace(rx, "");
                                } else {
                                    QRegularExpression rx2("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\)");
                                    match = rx2.match(str);
                                    if (match.hasMatch()) {
                                        m_state->Adj.Dados.variaveis.Vmaior.append(match.captured(1).toDouble());
                                        m_state->Adj.Dados.variaveis.Vmenor.append(match.captured(2).toDouble());
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
            if (!Vmaior.empty() && !isNormalizado) {
                if (m_state->Adj.Dados.variaveis.Vmaior.isEmpty()) {
                    m_state->Adj.Dados.variaveis.Vmaior.append(Vmaior);
                    m_state->Adj.Dados.variaveis.Vmenor.append(Vmenor);
                } else
                    for (j = 0; j < nlinha; j++) {
                        if (m_state->Adj.Dados.variaveis.Vmaior.at(j) < Vmaior.at(j))
                            m_state->Adj.Dados.variaveis.Vmaior.replace(j, Vmaior.at(j));
                        if (m_state->Adj.Dados.variaveis.Vmenor.at(j) > Vmenor.at(j))
                            m_state->Adj.Dados.variaveis.Vmenor.replace(j, Vmenor.at(j));
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
            // Calcula a Decimacao
            j = m_state->Adj.Dados.variaveis.valores.numLinhas();
            for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                m_state->Adj.decimacao.append(1);
                m_state->mediaY[idSaida] /= m_state->Adj.Dados.variaveis.valores.numColunas();
                m_state->mediaY2[idSaida] /= m_state->Adj.Dados.variaveis.valores.numColunas();
                for (i = 1; i < m_state->Adj.Dados.variaveis.valores.numColunas(); i++) {
                    talr += 0;
                    talr2 += 0;
                    for (valor = m_state->Adj.Dados.variaveis.valores.begin() + (i * j) + idSaida;
                         valor < m_state->Adj.Dados.variaveis.valores.end(); valor += j) {
                        talr += ((*valor) - m_state->mediaY.at(idSaida)) * (*(valor - (i * j)) - m_state->mediaY.at(idSaida));
                        talr2 += (((*valor) * (*valor)) - m_state->mediaY2.at(idSaida)) * (((*(valor - (i * j))) * (*(valor - (i * j)))) - m_state->mediaY2.at(idSaida));
                    }
                    talr /= m_state->Adj.Dados.variaveis.valores.numColunas();
                    talr2 /= m_state->Adj.Dados.variaveis.valores.numColunas();
                    if ((i == 1) || (talr < talMin) || (talr2 < talMin)) {
                        if (talr < talr2) talMin = talr; else talMin = talr2;
                    } else {
                        m_state->Adj.talDecim.append(i);
                        break;
                    }
                }
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
    // Normalizando os dados (0 a 1).
    forever {
        m_state->lock_index[0].lockForWrite();
        index = m_state->index[0]++;
        m_state->lock_index[0].unlock();
        if (index < ncoluna) {
            for (j = 0; j < nlinha; j++) {
                m_state->Adj.Dados.variaveis.valores(j, index) = (QString("%1").arg(
                    m_state->Adj.Dados.variaveis.valores(j, index) >= m_state->Adj.Dados.variaveis.Vmenor.at(j)
                        ? m_state->Adj.Dados.variaveis.valores(j, index) <= m_state->Adj.Dados.variaveis.Vmaior.at(j)
                            ? 0.99 * ((m_state->Adj.Dados.variaveis.valores.at(j, index) - m_state->Adj.Dados.variaveis.Vmenor.at(j)) / (m_state->Adj.Dados.variaveis.Vmaior.at(j) - m_state->Adj.Dados.variaveis.Vmenor.at(j))) + 0.01
                            : 1
                        : 0.01)).toDouble();
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
