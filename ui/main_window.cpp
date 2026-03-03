#include "main_window.h"
#include "../threading/shared_state.h"
#include "../threading/thread_worker.h"
#include "../core/chromosome_service.h"
#include "../io/data_service.h"
#include "../core/evolution_engine.h"
#include "../interfaces/i_equation_formatter.h"
#include "../interfaces/i_config_persistence.h"
#include "icarregar.h"
#include "xmlreaderwriter.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QApplication>
#include <QScreen>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDateTime>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <limits>
#include <algorithm>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>

Q_DECLARE_METATYPE(QVector<qreal>)
Q_DECLARE_METATYPE(QList<QVector<qreal> >)

///////////////////////////////////////////////////////////////////////////////
inline void reCopy(QVector<qreal> &cr1, const QVector<qreal> &cr2)
{
    qint32 id;
    const qint32 count1 = cr1.size(), count2 = cr2.size();
    if (count1 < count2) cr1 += QVector<qreal>(count2 - count1);
    else if (count1 > count2) cr1.remove(count2, count1 - count2);
    for (id = 0; id < count2; id++) cr1[id] = cr2.at(id);
}

inline void reCopy(QList<QVector<qreal> > &cr1, const QList<QVector<qreal> > &cr2)
{
    qint32 id;
    const qint32 count1 = cr1.size(), count2 = cr2.size();
    if (count1 < count2) cr1 += QVector<QVector<qreal> >(count2 - count1).toList();
    else for (id = count2; id < count1; id++) cr1.removeAt(id);
    for (id = 0; id < count2; id++) reCopy(cr1[id], cr2.at(id));
}

inline bool CmpMaiorCrApt(const Cromossomo &cr1, const Cromossomo &cr2)
{
    return (cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}

///////////////////////////////////////////////////////////////////////////////
class Zoomer : public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas)
        : QwtPlotZoomer(xAxis, yAxis, (QwtPlotCanvas *)canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);
        setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    }
};

///////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(SharedState *state, QWidget *parent)
    : DummyBase(parent)
    , m_state(state)
    , m_eqFormatter(nullptr)
    , m_configPersist(nullptr)
{
    setupUi(this);
    //////////////////////////////////////////////////////////////////////////////
    cr = new ICarregar(this);
    MW_SaidaUsada = 0;
    MW_changeStyle = 0;
    //////////////////////////////////////////////////////////////////////////////
    ini_MW_interface();
    ini_MW_qwtPlot1();
    //////////////////////////////////////////////////////////////////////////////
    // Conexoes UI
    connect(CheckBox1, SIGNAL(stateChanged(int)), this, SLOT(slot_MW_changeCheckBox1(int)));
    connect(CheckBox2, SIGNAL(stateChanged(int)), this, SLOT(slot_MW_changeCheckBox2(int)));
    connect(styleComboBox, SIGNAL(activated(int)), this, SLOT(slot_MW_changeStyle(int)));
    connect(saidaComboBox, SIGNAL(activated(int)), this, SLOT(slot_MW_changeVerSaida(int)));
    connect(actionIni, SIGNAL(triggered()), this, SLOT(slot_MW_IniciarFinalizar()));
    connect(actionParar, SIGNAL(triggered()), this, SLOT(slot_MW_PararContinuar()));
    connect(actionCarDados, SIGNAL(triggered()), this, SLOT(slot_MW_CarDados()));
    connect(cr, SIGNAL(finished(int)), this, SLOT(slot_MW_CarDadosExit()));
    connect(cr, SIGNAL(signal_UL_SalvarArquivo()), this, SLOT(slot_MW_SalvarArquivo()), Qt::DirectConnection);
    connect(cr, SIGNAL(signal_UL_FName(const QString &)), this, SLOT(slot_MW_FName(const QString &)), Qt::DirectConnection);
    connect(actionConfig, SIGNAL(triggered()), dialogConfig, SLOT(open()));
    connect(pbSalvarDadosTxT, SIGNAL(clicked()), this, SLOT(slot_MW_SalvarArquivo()), Qt::DirectConnection);
    connect(pbSalvarDados, SIGNAL(clicked()), this, SLOT(slot_MW_saveAsConfig()), Qt::DirectConnection);
    connect(pbOpenDados, SIGNAL(clicked()), this, SLOT(slot_MW_openConfig()), Qt::DirectConnection);
    connect(pbConcDados, SIGNAL(clicked()), this, SLOT(slot_MW_SalvarArquivo()), Qt::DirectConnection);
    connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));

    // Cria workers — um por core
    for (qint32 i = 0; i < QThread::idealThreadCount(); i++) {
        ThreadWorker *w = new ThreadWorker(m_state, this);

        // Cria services para este worker
        DataService *ds = new DataService(m_state, w);
        ChromosomeService *cs = new ChromosomeService(m_state);
        EvolutionEngine *engine = new EvolutionEngine(m_state, cs, w);

        w->setDataService(ds);
        w->setEvolutionEngine(engine);

        m_workers.append(w);

        // Conecta sinais do worker
        connect(w, SIGNAL(signal_SetStatus(const volatile qint64 &, const QVector<qreal> *, const QList<QVector<qreal> > *, const QList<QVector<qreal> > *, const QVector<Cromossomo> *)),
                this, SLOT(slot_MW_SetStatus(const volatile qint64 &, const QVector<qreal> *, const QList<QVector<qreal> > *, const QList<QVector<qreal> > *, const QVector<Cromossomo> *)), Qt::DirectConnection);
        connect(w, SIGNAL(signal_closed()), this, SLOT(slot_MW_closed()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Parado()), cr, SLOT(slot_UL_Parado()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Parado()), this, SLOT(slot_MW_Parado()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Finalizado()), cr, SLOT(slot_UL_Finalizado()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Finalizado()), this, SLOT(slot_MW_Finalizado()), Qt::QueuedConnection);
        connect(this, SIGNAL(signal_MW_EquacaoEscrita()), w, SLOT(slot_EquacaoEscrita()), Qt::QueuedConnection);
        connect(this, SIGNAL(signal_MW_StatusSetado()), w, SLOT(slot_StatusSetado()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Desenha()), this, SLOT(slot_MW_Desenha()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_EscreveEquacao()), this, SLOT(slot_MW_EscreveEquacao()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Finalizar()), this, SLOT(slot_MW_Finalizar()), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Status(const quint16)), cr, SLOT(slot_UL_Status(const quint16)), Qt::QueuedConnection);
        connect(w, SIGNAL(signal_Tam()), cr, SLOT(slot_UL_Tam()), Qt::QueuedConnection);
    }

    isThClose = false;
    connect(this, SIGNAL(signal_MW_Estado(const quint16 &)), m_workers.at(0), SLOT(slot_Estado(const quint16 &)), Qt::QueuedConnection);
    connect(cr, SIGNAL(signal_UL_Estado(const quint16 &)), m_workers.at(0), SLOT(slot_Estado(const quint16 &)), Qt::QueuedConnection);

    actionParar->setEnabled(false);
    actionIni->setEnabled(false);
    pbSalvarDadosTxT->setEnabled(false);
    saidaComboBox->setEnabled(false);
    enableZoomMode(false);

    QRect r = geometry();
    r.moveCenter(QGuiApplication::primaryScreen()->availableGeometry().center());
    setGeometry(r);
}

///////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::setEquationFormatter(IEquationFormatter *formatter)
{
    m_eqFormatter = formatter;
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::setConfigPersistence(IConfigPersistence *persistence)
{
    m_configPersist = persistence;
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent *event)
{
    bool isOk;
    if (isThClose) event->accept();
    else {
        isOk = m_state->modoOperTH() == 1;
        if (isOk) emit signal_MW_Estado(0);
        else QMessageBox::warning(this, tr("Impossivel fechar"), tr("Finalize as tarefas para poder fecha-lo!"));
        event->ignore();
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_closed()
{
    isThClose = true;
    QMainWindow::close();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_changeStyle(const int &changeStyle)
{
    MW_changeStyle = changeStyle;
    if (!MW_changeStyle) {
        CheckBox1->setDisabled(false);
        CheckBox2->setDisabled(false);
        qwtPlot1->setAxisTitle(QwtPlot::yLeft, tr("Saidas"));
        qwtPlot1->setAxisTitle(QwtPlot::xBottom, tr("Amostras"));
        MW_crv_R->attach(qwtPlot1);
        MW_crv_C->setTitle(tr("Estimado"));
    } else {
        qwtPlot1->setAxisTitle(QwtPlot::yLeft, tr("Quantidade de Residuos"));
        qwtPlot1->setAxisTitle(QwtPlot::xBottom, tr("Valores dos Residuos"));
        MW_crv_C->setTitle(tr("Distribuicao dos Residuos"));
        MW_crv_R->detach();
        CheckBox1->setDisabled(true);
        CheckBox2->setDisabled(true);
    }
    slot_MW_Desenha();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_changeVerSaida(const int &idSaida)
{
    MW_SaidaUsada = idSaida;
    slot_MW_Desenha();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_FName(const QString &fileName)
{
    m_state->fileName = fileName;
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_Parado()
{
    if (this->isActiveWindow()) {
        actionIni->setEnabled(true);
        LEEM->setEnabled(true);
        LEENC->setEnabled(true);
        LEESE->setEnabled(true);
        polRacComboBox->setEnabled(true);
        intRealComboBox->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_Finalizado()
{
    if (this->isActiveWindow()) {
        actionIni->setEnabled(true);
        LEEM->setEnabled(true);
        LEENC->setEnabled(true);
        LEESE->setEnabled(true);
        polRacComboBox->setEnabled(true);
        intRealComboBox->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_IniciarFinalizar()
{
    if (actionIni->text() == QString("Finalizar")) {
        actionIni->setEnabled(false);
        actionIni->setText(QString("Iniciar"));
        emit signal_MW_Estado(1);
        actionParar->setEnabled(false);
        actionCarDados->setEnabled(true);
        LEN->setEnabled(true);
        LEEL->setEnabled(true);
        LEEM->setEnabled(true);
    } else {
        MW_crBest.clear();
        m_state->Adj.isCriado = false;
        MW_iteracoes = 0;
        for (qint32 k = 0; k < TAMPIPELINE; k++) {
            m_state->Adj.vetPop[k].clear();
            m_state->crMut[k].clear();
            m_state->somaSSE[k].clear();
            m_state->index[k] = 0;
            m_state->idParadaJust[k] = false;
        }
        m_state->Adj.iteracoes = 0;
        m_state->Adj.segundos = QDateTime::fromString("M1d1y0000:00:00", "'M'M'd'd'y'yyhh:mm:ss");
        m_state->Adj.Dados.isElitismo = 2;
        if (m_state->Adj.Dados.qtdadeVarAnte == m_state->Adj.Dados.variaveis.valores.numLinhas()) {
            if (!(QMessageBox::question(this, tr("Limpar pontos"), tr("Deseja manter os pontos do Elitismo?"),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes))
                m_state->Adj.Dados.isElitismo = 1;
        } else
            m_state->Adj.Dados.qtdadeVarAnte = m_state->Adj.Dados.variaveis.valores.numLinhas();
        Iniciar();
        slot_MW_PararContinuar();
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::Iniciar()
{
    qint32 i = 0, j = 0, k = 0;
    QString str;
    const qint32 qtSaidas = m_state->Adj.Dados.variaveis.qtSaidas;
    const qint32 tamDados = m_state->Adj.Dados.variaveis.valores.numColunas();

    actionCarDados->setEnabled(false);
    actionParar->setEnabled(true);
    actionIni->setEnabled(false);
    LEN->setEnabled(false);
    LEEL->setEnabled(false);
    textEdit->clear();
    saidaComboBox->clear();

    str.append(QString("Cl:= ") + QString::number(MW_iteracoes) + QString("; Tempo: ") + m_state->Adj.segundos.toString("hh:mm:ss.zzz") + QString("ms\n"));
    for (i = 0; i < m_state->Adj.Dados.variaveis.nome.size(); i++) {
        if (i < m_state->Adj.Dados.variaveis.qtSaidas)
            str.append(m_state->Adj.Dados.variaveis.nome.at(i)
                + QString(": Max = ") + QString::number(m_state->Adj.Dados.variaveis.Vmaior.at(i))
                + QString(", Min = ") + QString::number(m_state->Adj.Dados.variaveis.Vmenor.at(i))
                + QString(", Decimacao = ") + QString::number(m_state->Adj.decimacao.at(i)) + QString("; "));
        else
            str.append(m_state->Adj.Dados.variaveis.nome.at(i)
                + QString(": Max = ") + QString::number(m_state->Adj.Dados.variaveis.Vmaior.at(i))
                + QString(", Min = ") + QString::number(m_state->Adj.Dados.variaveis.Vmenor.at(i)) + QString("; "));
    }
    textEdit->append(str);

    for (k = 0; k < TAMPIPELINE; k++) {
        m_state->crMut[k] += QVector<Cromossomo>(qtSaidas);
        m_state->BufferSR[k] = (QVector<QVector<Cromossomo> >(qtSaidas)).toList();
        m_state->somaSSE[k].fill(0.0f, qtSaidas);
        m_state->index[k] = 0;
        m_state->idParadaJust[k] = false;
        m_state->vcalc[k] = QVector<QVector<qreal> >(qtSaidas).toList();
        m_state->residuos[k] = QVector<QVector<qreal> >(qtSaidas).toList();
        m_state->Adj.isSR[k] = QVector<QList<bool> >(qtSaidas).toList();
        m_state->idChange[k] = QVector<qint32>(qtSaidas);
        m_state->Adj.vetElitismo[k] = QVector<QVector<qint32> >(qtSaidas).toList();
        if (m_state->Adj.Pop.isEmpty())
            m_state->Adj.Pop = QVector<QVector<Cromossomo> >(qtSaidas).toList();
        for (j = 0; j < qtSaidas; j++) {
            m_state->BufferSR[k][j] = QVector<Cromossomo>(10);
            m_state->vcalc[k][j] = QVector<qreal>(tamDados / m_state->Adj.decimacao.at(j));
            m_state->residuos[k][j] = QVector<qreal>(tamDados / m_state->Adj.decimacao.at(j));
        }
    }

    for (i = 0; i < qtSaidas; i++)
        saidaComboBox->addItem(m_state->Adj.Dados.variaveis.nome.at(i));
    MW_bestFeito = QVector<qreal>(qtSaidas);
    MW_best = QVector<QVector<qreal> >(qtSaidas).toList();
    MW_crBest = QVector<Cromossomo>(qtSaidas);
    saidaComboBox->setCurrentIndex(0);
    actionParar->setText(QString("Continuar"));
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_PararContinuar()
{
    if (actionParar->text() == QString("Parar")) {
        emit signal_MW_Estado(2);
        actionParar->setEnabled(false);
    } else {
        m_state->Adj.iteracoesAnt = m_state->Adj.iteracoes;
        const qint32 qtSaidas = m_state->Adj.Dados.variaveis.qtSaidas;
        m_state->Adj.numeroCiclos = LEENC->text().toInt();
        m_state->Adj.jnrr = LEEM->text().toDouble();
        m_state->Adj.serr = LEESE->text().toDouble();
        m_state->Adj.isTipoExpo = intRealComboBox->currentIndex();
        m_state->Adj.isRacional = (polRacComboBox->currentIndex() == 0);
        LEEM->setEnabled(false);
        LEENC->setEnabled(false);
        LEESE->setEnabled(false);
        polRacComboBox->setEnabled(false);
        intRealComboBox->setEnabled(false);
        actionParar->setText(QString("Parar"));
        actionIni->setEnabled(false);
        MW_tempo = QDateTime::currentDateTime();
        m_state->Adj.Dados.tamPop = LEN->text().toInt();
        if ((m_state->Adj.Dados.iElitismo == 0) || (m_state->Adj.Dados.iElitismo != LEEL->text().toInt()))
            m_state->Adj.Dados.iElitismo = LEEL->text().toInt() ? LEEL->text().toInt() : 5;

        if ((!m_state->Adj.isCriado) || (m_state->Adj.Pop.at(0).size() < m_state->Adj.Dados.tamPop)) {
            m_state->lock_BufferSR.lockForWrite();
            if (m_state->Adj.Dados.isElitismo != 1) {
                for (qint32 k = 0; k < qtSaidas; k++)
                    m_state->Adj.Pop[k] = QVector<Cromossomo>(m_state->Adj.Dados.tamPop);
            } else {
                for (qint32 k = 0; k < qtSaidas; k++) {
                    std::sort(m_state->Adj.Pop[k].begin(), m_state->Adj.Pop[k].end(), CmpMaiorCrApt);
                    if (m_state->Adj.Pop.at(k).size() < m_state->Adj.Dados.tamPop)
                        m_state->Adj.Pop[k] += QVector<Cromossomo>(m_state->Adj.Dados.tamPop - m_state->Adj.Pop.at(k).size());
                }
            }
            m_state->lock_BufferSR.unlock();
        }
        if (actionIni->text() != QString("Finalizar")) actionIni->setText(QString("Finalizar"));
        emit signal_MW_Estado(3);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_CarDados()
{
    cr->slot_UL_Status(0);
    cr->exec();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_CarDadosExit()
{
    if (!m_state->Adj.Dados.variaveis.nome.isEmpty()) {
        actionIni->setEnabled(true);
        pbSalvarDadosTxT->setEnabled(true);
    } else {
        actionIni->setEnabled(false);
        pbSalvarDadosTxT->setEnabled(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_SalvarArquivo()
{
    const qreal step = (m_state->Adj.Dados.timeFinal - m_state->Adj.Dados.timeInicial) / m_state->Adj.Dados.variaveis.valores.numColunas();
    const QString fileName = QFileDialog::getSaveFileName(cr, tr("Salvar Dados"), QDir::currentPath(), tr("Arquivo de Dados (*.txt)"));
    qint32 i;
    qreal *valor, time = m_state->Adj.Dados.timeInicial;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream(&file);
        stream.setRealNumberNotation(QTextStream::ScientificNotation);
        stream.setRealNumberPrecision(9);
        stream << "TIME     ";
        for (i = 0; i < m_state->Adj.Dados.variaveis.nome.size(); i++) {
            if (i < m_state->Adj.Dados.variaveis.qtSaidas)
                stream << m_state->Adj.Dados.variaveis.nome.at(i) << "(" << m_state->Adj.Dados.variaveis.Vmaior.at(i) << "," << m_state->Adj.Dados.variaveis.Vmenor.at(i) << "," << m_state->Adj.decimacao.at(i) << ") ";
            else
                stream << m_state->Adj.Dados.variaveis.nome.at(i) << "(" << m_state->Adj.Dados.variaveis.Vmaior.at(i) << "," << m_state->Adj.Dados.variaveis.Vmenor.at(i) << ") ";
        }
        stream << "\r\n" << time << "  ";
        const qint32 count = m_state->Adj.Dados.variaveis.valores.numLinhas();
        for (valor = m_state->Adj.Dados.variaveis.valores.begin(), i = 0; valor < m_state->Adj.Dados.variaveis.valores.end(); valor++, i++) {
            if (i == count) { i = 0; time += step; stream << "\r\n" << time << "  "; }
            stream << *valor << " ";
        }
        stream << "\r\n ";
        file.close();
    }
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_SetStatus(const volatile qint64 &iteracoes, const QVector<qreal> *somaEr,
                                    const QList<QVector<qreal> > *resObtido,
                                    const QList<QVector<qreal> > *residuo,
                                    const QVector<Cromossomo> *crBest)
{
    m_state->lockLerDados.lockForWrite();
    MW_iteracoes = iteracoes;
    reCopy(MW_resObtido, *resObtido);
    reCopy(MW_residuo, *residuo);
    reCopy(MW_somaEr, *somaEr);
    std::copy(crBest->begin(), crBest->end(), MW_crBest.begin());
    emit signal_MW_StatusSetado();
    m_state->lockLerDados.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_EscreveEquacao()
{
    qint64 iteracoes;
    QVector<qreal> somaEr;
    QVector<Cromossomo> crBest;

    m_state->lockLerDados.lockForRead();
    iteracoes = MW_iteracoes;
    reCopy(somaEr, MW_somaEr);
    crBest.resize(MW_crBest.size());
    std::copy(MW_crBest.begin(), MW_crBest.end(), crBest.begin());
    m_state->lockLerDados.unlock();

    QString str;
    if (m_eqFormatter) {
        str = m_eqFormatter->format(crBest, iteracoes, somaEr);
    } else {
        str = QString("Cl:= %1\n").arg(iteracoes);
    }

    CheckBox1->setChecked(true);
    CheckBox2->setChecked(true);
    textEdit->setText(str);
    emit signal_MW_EquacaoEscrita();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_Desenha()
{
    QList<QVector<qreal> > resObtido;
    QVector<Cromossomo> crBest;
    QList<QVector<qreal> > residuos;
    qreal auxReal1, auxReal2;
    QVector<qreal> medida, time, residuo;
    qint32 j, i, numColuna;

    m_state->lockLerDados.lockForRead();
    reCopy(resObtido, MW_resObtido);
    reCopy(residuos, MW_residuo);
    crBest.resize(MW_crBest.size());
    std::copy(MW_crBest.begin(), MW_crBest.end(), crBest.begin());
    m_state->lockLerDados.unlock();

    numColuna = resObtido.at(MW_SaidaUsada).size();
    time.clear();
    if (!MW_changeStyle) {
        for (i = crBest.at(MW_SaidaUsada).maiorAtraso; i < numColuna; i++) {
            time.append(i);
            medida.append(m_state->Adj.Dados.variaveis.valores.at(MW_SaidaUsada, i * m_state->Adj.decimacao.at(MW_SaidaUsada)));
        }
        MW_crv_C->setSamples(time, resObtido.at(MW_SaidaUsada));
        MW_crv_R->setSamples(time, medida);
    } else {
        residuo += residuos[MW_SaidaUsada].mid(residuos[MW_SaidaUsada].size() / 2,
                                                residuos[MW_SaidaUsada].size() - (residuos[MW_SaidaUsada].size() / 2));
        std::sort(residuo.begin(), residuo.end(), std::less<qreal>());
        auxReal1 = residuo.last() > (-residuo.first()) ? residuo.last() / 25 : (-residuo.first()) / 25;
        auxReal2 = (residuo.last() > (-residuo.first()) ? (-residuo.last()) : residuo.first()) - (auxReal1 / 2);
        time.append(auxReal2); time.append(auxReal2);
        for (i = 0, auxReal2 += auxReal1; i < 50; i++, auxReal2 += auxReal1) { time.append(auxReal2); time.append(auxReal2); }
        medida = QVector<qreal>(time.size(), 0.0f);
        i = 0;
        auxReal2 = (residuo.last() > (-residuo.first()) ? (-residuo.last()) : residuo.first()) - (auxReal1 / 2);
        for (j = 0; j < residuo.size(); j++) {
            for (; auxReal2 <= residuo.at(j); auxReal2 += auxReal1, i++) ;
            if ((i * 2 + 2) < medida.size()) { medida[i * 2 + 1]++; medida[i * 2 + 2]++; }
        }
        MW_crv_C->setSamples(time, medida);
    }
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_Finalizar()
{
    saidaComboBox->setEnabled(true);
    slot_MW_EscreveEquacao();
    slot_MW_Desenha();
    actionParar->setEnabled(true);
    actionParar->setText(QString("Continuar"));
    actionIni->setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_changeCheckBox1(int st)
{
    if (!st) { MW_crv_R->setVisible(false); CheckBox2->setChecked(true); }
    else MW_crv_R->setVisible(true);
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_changeCheckBox2(int st)
{
    if (!st) { MW_crv_C->setVisible(false); CheckBox1->setChecked(true); }
    else MW_crv_C->setVisible(true);
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::ini_MW_interface()
{
    dialogConfig = new QDialog(this);
    (new Ui::DialogConfig())->setupUi(dialogConfig);
    mainToolbar = this->findChild<QToolBar *>("toolBar");
    actionExit = this->findChild<QAction *>("actionExit");
    actionIni = this->findChild<QAction *>("actionIni");
    actionParar = this->findChild<QAction *>("actionParar");
    actionCarDados = this->findChild<QAction *>("actionCarDados");
    actionConfig = this->findChild<QAction *>("actionConfig");
    actionZoom = this->findChild<QAction *>("actionZoom");
    pbSalvarDadosTxT = dialogConfig->findChild<QPushButton *>("pb_SArquivo");
    pbSalvarDados = dialogConfig->findChild<QPushButton *>("pb_DC_Salvar");
    pbOpenDados = dialogConfig->findChild<QPushButton *>("pb_DC_Carregar");
    pbConcDados = dialogConfig->findChild<QPushButton *>("pb_DC_Concatenar");
    actionZoom->setText("Zoom");
    actionZoom->setCheckable(true);
    connect(actionZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));

    QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);

    LN = new QLabel(" N:=", this);
    sizePolicy3.setHeightForWidth(LN->sizePolicy().hasHeightForWidth());
    LN->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LN);

    LEN = new QLineEdit("300", this);
    LEN->setObjectName(QString::fromUtf8("LEN"));
    LEN->setSizePolicy(sizePolicy3);
    LEN->setMaximumSize(QSize(40, 16777215));
    LEN->setMaxLength(50);
    LEN->setCursorPosition(2);
    LEN->setValidator(new QIntValidator(0, 10000, this));
    mainToolbar->addWidget(LEN);

    LEL = new QLabel(" E:=", this);
    LEL->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEL);

    LEEL = new QLineEdit("5", this);
    LEEL->setObjectName(QString::fromUtf8("LEEL"));
    LEEL->setSizePolicy(sizePolicy3);
    LEEL->setMaximumSize(QSize(20, 16777215));
    LEEL->setContextMenuPolicy(Qt::NoContextMenu);
    LEEL->setValidator(new QIntValidator(0, 10000, this));
    mainToolbar->addWidget(LEEL);

    LESE = new QLabel(" SERR:=", this);
    LESE->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LESE);

    LEESE = new QLineEdit("0.98", this);
    LEESE->setObjectName(QString::fromUtf8("LEESE"));
    LEESE->setSizePolicy(sizePolicy3);
    LEESE->setMaximumSize(QSize(40, 16777215));
    LEESE->setContextMenuPolicy(Qt::NoContextMenu);
    LEESE->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    mainToolbar->addWidget(LEESE);

    mainToolbar->addSeparator();
    polRacComboBox = new QComboBox;
    polRacComboBox->addItem("Racional");
    polRacComboBox->addItem("Polinomial");
    polRacComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(polRacComboBox);

    mainToolbar->addSeparator();
    LEIR = new QLabel(" Expoente do tipo:", this);
    LEIR->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEIR);
    intRealComboBox = new QComboBox;
    intRealComboBox->addItem("Real");
    intRealComboBox->addItem("Inteiro");
    intRealComboBox->addItem("Natural");
    intRealComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(intRealComboBox);

    mainToolbar->addSeparator();
    LEM = new QLabel(" RJn:= ", this);
    LEM->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEM);

    LEEM = new QLineEdit("0.001", this);
    LEEM->setObjectName(QString::fromUtf8("LEEM"));
    LEEM->setSizePolicy(sizePolicy3);
    LEEM->setMaximumSize(QSize(40, 16777215));
    LEEM->setContextMenuPolicy(Qt::NoContextMenu);
    LEEM->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    mainToolbar->addWidget(LEEM);

    LENC = new QLabel(" NCy:= ", this);
    LENC->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LENC);

    LEENC = new QLineEdit("100", this);
    LEENC->setObjectName(QString::fromUtf8("LEENC"));
    LEENC->setSizePolicy(sizePolicy3);
    LEENC->setMaximumSize(QSize(40, 16777215));
    LEENC->setContextMenuPolicy(Qt::NoContextMenu);
    LEENC->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    mainToolbar->addWidget(LEENC);

    mainToolbar->addSeparator();
    styleComboBox = new QComboBox;
    styleComboBox->addItem("Medido/Estimado");
    styleComboBox->addItem("Dist. Residuos");
    styleComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(styleComboBox);

    mainToolbar->addSeparator();
    CheckBox1 = new QCheckBox;
    CheckBox1->setText("Medido");
    CheckBox1->setChecked(true);
    mainToolbar->addWidget(CheckBox1);
    CheckBox2 = new QCheckBox;
    CheckBox2->setText("Estimado");
    CheckBox2->setChecked(true);
    mainToolbar->addWidget(CheckBox2);

    mainToolbar->addSeparator();
    mainToolbar->addWidget(new QLabel(" Saida Visivel:= ", this));
    saidaComboBox = new QComboBox;
    mainToolbar->addWidget(saidaComboBox);
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::enableZoomMode(bool on)
{
    MW_panner1->setEnabled(on);
    MW_zoomer1[0]->setEnabled(on);
    MW_zoomer1[0]->zoom(0);
    MW_zoomer1[1]->setEnabled(on);
    MW_zoomer1[1]->zoom(0);
    MW_picker1->setEnabled(!on);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::ini_MW_qwtPlot1()
{
    qwtPlot1->setAutoReplot(false);
    qwtPlot1->setCanvasBackground(QColor(Qt::white));

    MW_panner1 = new QwtPlotPanner(qwtPlot1->canvas());
    MW_panner1->setMouseButton(Qt::MidButton);

    MW_zoomer1[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, qwtPlot1->canvas());
    MW_zoomer1[0]->setRubberBand(QwtPicker::RectRubberBand);
    MW_zoomer1[0]->setRubberBandPen(QColor(Qt::green));
    MW_zoomer1[0]->setTrackerMode(QwtPicker::ActiveOnly);
    MW_zoomer1[0]->setTrackerPen(QColor(Qt::black));

    MW_zoomer1[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, qwtPlot1->canvas());

    MW_picker1 = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                    QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, qwtPlot1->canvas());
    MW_picker1->setRubberBandPen(QColor(Qt::darkBlue));
    MW_picker1->setRubberBand(QwtPicker::CrossRubberBand);
    MW_picker1->setTrackerPen(QColor(Qt::black));
    connect(MW_picker1, SIGNAL(moved(const QPoint &)), SLOT(moved1(const QPoint &)));

    QwtLegend *legend1 = new QwtLegend;
    legend1->setFrameStyle(QFrame::Box | QFrame::Sunken);
    qwtPlot1->insertLegend(legend1, QwtPlot::TopLegend);

    QwtPlotGrid *grid1 = new QwtPlotGrid;
    grid1->enableXMin(true);
    grid1->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid1->setMinorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid1->attach(qwtPlot1);

    qwtPlot1->enableAxis(QwtPlot::yLeft);
    qwtPlot1->setAxisTitle(QwtPlot::xBottom, "Amostras");
    qwtPlot1->setAxisTitle(QwtPlot::yLeft, "Saidas");
    qwtPlot1->setAxisMaxMajor(QwtPlot::xBottom, 6);
    qwtPlot1->setAxisMaxMinor(QwtPlot::xBottom, 10);

    MW_crv_R = new QwtPlotCurve("Medido");
    MW_crv_R->setRenderHint(QwtPlotItem::RenderAntialiased);
    MW_crv_R->setPen(QPen(Qt::darkBlue, 1));
    MW_crv_R->setYAxis(QwtPlot::yLeft);
    MW_crv_R->setStyle(QwtPlotCurve::Lines);
    MW_crv_R->attach(qwtPlot1);

    MW_crv_C = new QwtPlotCurve("Estimado");
    MW_crv_C->setRenderHint(QwtPlotItem::RenderAntialiased);
    MW_crv_C->setPen(QPen(Qt::red, 1));
    MW_crv_C->setYAxis(QwtPlot::yLeft);
    MW_crv_C->setStyle(QwtPlotCurve::Lines);
    MW_crv_C->attach(qwtPlot1);

    MW_mrk1 = new QwtPlotMarker();
    MW_mrk1->setValue(0.0, 0.0);
    MW_mrk1->setLineStyle(QwtPlotMarker::VLine);
    MW_mrk1->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    MW_mrk1->setLinePen(QPen(Qt::green, 0, Qt::DashLine));
    MW_mrk1->attach(qwtPlot1);
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::moved1(const QPoint &pos)
{
    qwtPlot1->invTransform(QwtPlot::xBottom, pos.x());
    qwtPlot1->invTransform(QwtPlot::yLeft, pos.y());
    qwtPlot1->invTransform(QwtPlot::yRight, pos.y());
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_openConfig()
{
    NameConfigSave.clear();
    NameConfigSave = QFileDialog::getOpenFileName(this, tr("Open Bookmark File"),
                                                   QDir::currentPath(), tr("XBEL Files (*.xbel *.xml)"));
    if (NameConfigSave.isEmpty()) return;
    QFile file(NameConfigSave);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Cannot read file %1:\n%2.").arg(NameConfigSave).arg(file.errorString()));
        return;
    }
    if (m_configPersist) {
        if (!m_configPersist->load(&file)) {
            QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                                 tr("Parse error in file %1:\n\n%2").arg(NameConfigSave).arg(m_configPersist->errorString()));
        } else {
            m_state->Adj.Dados.tamPop = m_state->Adj.qdadeCrSalvos;
            LEENC->setText(QString::number(m_state->Adj.numeroCiclos));
            LEEM->setText(QString::number(m_state->Adj.jnrr));
            LEESE->setText(QString::number(m_state->Adj.serr));
            intRealComboBox->setCurrentIndex(m_state->Adj.isTipoExpo);
            polRacComboBox->setCurrentIndex(m_state->Adj.isRacional ? 0 : 1);
            LEN->setText(QString::number(m_state->Adj.Dados.tamPop));
            if (m_state->Adj.isPararContinuarEnabled) {
                actionParar->setEnabled(true);
                if (m_state->Adj.isPararContinuarEnabled == 1) actionParar->setText("Parar");
                else actionParar->setText("Continuar");
            } else
                actionParar->setEnabled(false);
            if (m_state->Adj.isIniciaEnabled) { m_state->Adj.isCriado = true; Iniciar(); }
            statusBar()->showMessage(tr("File carregado"), 20000);
        }
    }
    file.close();
}

///////////////////////////////////////////////////////////////////////////////
void MainWindow::slot_MW_saveAsConfig()
{
    if (NameConfigSave.isEmpty())
        NameConfigSave = QFileDialog::getSaveFileName(this, tr("Salvar dados de configuracao"),
                                                       QDir::currentPath(), tr("XBEL Files (*.xbel *.xml)"));
    if (NameConfigSave.isEmpty()) return;
    QFile file(NameConfigSave);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Arquivo %1 nao pode ser escrito:\n%2.").arg(NameConfigSave).arg(file.errorString()));
        return;
    }
    m_state->Adj.numeroCiclos = LEENC->text().toInt();
    m_state->Adj.jnrr = LEEM->text().toDouble();
    m_state->Adj.serr = LEESE->text().toDouble();
    m_state->Adj.isTipoExpo = intRealComboBox->currentIndex();
    m_state->Adj.isRacional = (polRacComboBox->currentIndex() == 0);
    m_state->Adj.isIniciaEnabled = actionIni->isEnabled();
    m_state->Adj.salvarAutomati = true;
    m_state->Adj.isPararContinuarEnabled = !actionParar->isEnabled() ? 0 : actionParar->text() == QString("Parar") ? 1 : 2;
    m_state->Adj.qdadeCrSalvos = m_state->Adj.Dados.tamPop;
    m_state->Adj.salvDadosCarre = 1;
    m_state->Adj.nomeArqConfTxT = "";
    if (m_configPersist) {
        if (m_configPersist->save(&file))
            statusBar()->showMessage(tr("File salvo"), 20000);
    }
    file.close();
}
