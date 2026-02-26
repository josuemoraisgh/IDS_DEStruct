#include "MainWindow.h"
#include "../domain/Configuration.h"
#include "../domain/Chromosome.h"
#include "../services/DifferentialEvolutionService.h"
#include "ui_imainwindow.h"
#include "ui_dialogConfig.h"
#include "PlotManager.h"

#include <QMessageBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QDateTime>
#include <QCloseEvent>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QSizePolicy>
#include <QFont>
#include <QScreen>
#include <QGuiApplication>
#include <QDialog>

#include "../utils/MathMatrix.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_legend.h>
#include <qwt_picker_machine.h>

// ============================================================================
// Zoomer helper – réplica fiel do original
// ============================================================================
class Zoomer : public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget* canvas)
        : QwtPlotZoomer(xAxis, yAxis, static_cast<QwtPlotCanvas*>(canvas))
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);
        setMousePattern(QwtEventPattern::MouseSelect2,
                        Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,
                        Qt::RightButton);
    }
};

namespace Presentation {

// ============================================================================
// Construtor / Destrutor
// ============================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_algorithmRunning(false)
    , m_closeRequested(false)
    , m_algorithmPaused(false)
    , m_mainToolbar(nullptr)
    , m_actionExit(nullptr), m_actionIni(nullptr), m_actionParar(nullptr)
    , m_actionCarDados(nullptr), m_actionConfig(nullptr), m_actionZoom(nullptr)
    , m_lblN(nullptr), m_editN(nullptr), m_lblE(nullptr), m_editE(nullptr)
    , m_lblSERR(nullptr), m_editSERR(nullptr)
    , m_comboPolRac(nullptr), m_lblExpTipo(nullptr), m_comboIntReal(nullptr)
    , m_lblRJn(nullptr), m_editRJn(nullptr)
    , m_lblNCy(nullptr), m_editNCy(nullptr)
    , m_comboStyle(nullptr), m_checkMedido(nullptr), m_checkEstimado(nullptr)
    , m_comboSaida(nullptr)
    , m_dialogConfig(nullptr)
    , m_pbSalvarDadosTxT(nullptr), m_pbSalvarDados(nullptr)
    , m_pbOpenDados(nullptr), m_pbConcDados(nullptr)
    , m_dataLoaderDialog(nullptr)
    , m_panner(nullptr), m_picker(nullptr)
    , m_curveMedido(nullptr), m_curveEstimado(nullptr), m_marker(nullptr)
    , m_currentOutput(0), m_currentStyle(0)
{
    m_zoomer[0] = nullptr;
    m_zoomer[1] = nullptr;

    // Monta o esqueleto vindo do .ui
    m_ui->setupUi(this);

    // ---- Inicialização programática (réplica de ini_MW_interface) -----------
    initInterface();

    // ---- Inicialização do gráfico (réplica de ini_MW_qwtPlot1) -------------
    initQwtPlot();

    // ---- Conexões ----------------------------------------------------------
    setupConnections();

    // Estado inicial (réplica do original)
    m_actionParar->setEnabled(false);
    m_actionIni->setEnabled(false);
    m_pbSalvarDadosTxT->setEnabled(false);
    m_comboSaida->setEnabled(false);

    onActionZoomTriggered(false);     // enableZoomMode(false)

    statusBar()->showMessage("Pronto", 2000);
    setWindowTitle("LASEC - FEELT - UFU  |  IDS_DEStruct (Refatorado)");

    // Centraliza na tela
    QRect r = geometry();
    r.moveCenter(QGuiApplication::primaryScreen()->availableGeometry().center());
    setGeometry(r);
}

MainWindow::~MainWindow() = default;

// ============================================================================
// setPresenter
// ============================================================================
void MainWindow::setPresenter(std::shared_ptr<MainWindowPresenter> presenter)
{
    m_presenter = presenter;
    if (!m_presenter) return;

    connect(m_presenter.get(), &MainWindowPresenter::statusUpdated,
            this, &MainWindow::onStatusUpdated);
    connect(m_presenter.get(), &MainWindowPresenter::algorithmFinished,
            this, &MainWindow::onAlgorithmFinished);
    connect(m_presenter.get(), &MainWindowPresenter::algorithmPaused,
            this, &MainWindow::onAlgorithmPaused);
    connect(m_presenter.get(), &MainWindowPresenter::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_presenter.get(), &MainWindowPresenter::statusMessageChanged,
            this, &MainWindow::onStatusMessageChanged);
}

// ============================================================================
// initInterface – réplica fiel de ICalc::ini_MW_interface()
// ============================================================================
void MainWindow::initInterface()
{
    // ----- Dialog de Carregamento de Dados (réplica do ICarregar) -----------
    m_dataLoaderDialog = new DataLoaderDialog(this);

    // ----- Dialog de Configuração -------------------------------------------
    m_dialogConfig = new QDialog(this);
    (new Ui::DialogConfig())->setupUi(m_dialogConfig);

    // ----- Localiza widgets do .ui ------------------------------------------
    m_mainToolbar  = findChild<QToolBar*>("toolBar");
    m_actionExit   = findChild<QAction*>("actionExit");
    m_actionIni    = findChild<QAction*>("actionIni");
    m_actionParar  = findChild<QAction*>("actionParar");
    m_actionCarDados = findChild<QAction*>("actionCarDados");
    m_actionConfig = findChild<QAction*>("actionConfig");
    m_actionZoom   = findChild<QAction*>("actionZoom");

    // Botões dentro do dialogConfig
    m_pbSalvarDadosTxT = m_dialogConfig->findChild<QPushButton*>("pb_SArquivo");
    m_pbSalvarDados    = m_dialogConfig->findChild<QPushButton*>("pb_DC_Salvar");
    m_pbOpenDados      = m_dialogConfig->findChild<QPushButton*>("pb_DC_Carregar");
    m_pbConcDados      = m_dialogConfig->findChild<QPushButton*>("pb_DC_Concatenar");

    // ----- Zoom toggle ------------------------------------------------------
    m_actionZoom->setText("Zoom");
    m_actionZoom->setCheckable(true);

    // ----- Widgets adicionados PROGRAMATICAMENTE na toolbar ------------------
    QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(0);

    // N (tamanho da população)
    m_lblN = new QLabel(" N:=", this);
    m_lblN->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblN);

    m_editN = new QLineEdit("300", this);
    m_editN->setObjectName("LEN");
    m_editN->setSizePolicy(sp);
    m_editN->setMaximumSize(QSize(40, 16777215));
    m_editN->setMaxLength(50);
    m_editN->setValidator(new QIntValidator(0, 10000, this));
    m_mainToolbar->addWidget(m_editN);

    // E (elitismo)
    m_lblE = new QLabel(" E:=", this);
    m_lblE->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblE);

    m_editE = new QLineEdit("5", this);
    m_editE->setObjectName("LEEL");
    m_editE->setSizePolicy(sp);
    m_editE->setMaximumSize(QSize(20, 16777215));
    m_editE->setContextMenuPolicy(Qt::NoContextMenu);
    m_editE->setValidator(new QIntValidator(0, 10000, this));
    m_mainToolbar->addWidget(m_editE);

    // SERR
    m_lblSERR = new QLabel(" SERR:=", this);
    m_lblSERR->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblSERR);

    m_editSERR = new QLineEdit("0.98", this);
    m_editSERR->setObjectName("LEESE");
    m_editSERR->setSizePolicy(sp);
    m_editSERR->setMaximumSize(QSize(40, 16777215));
    m_editSERR->setContextMenuPolicy(Qt::NoContextMenu);
    m_editSERR->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    m_mainToolbar->addWidget(m_editSERR);

    // Racional / Polinomial
    m_mainToolbar->addSeparator();
    m_comboPolRac = new QComboBox(this);
    m_comboPolRac->addItem("Racional");
    m_comboPolRac->addItem("Polinomial");
    m_comboPolRac->setCurrentIndex(0);
    m_mainToolbar->addWidget(m_comboPolRac);

    // Expoente do tipo
    m_mainToolbar->addSeparator();
    m_lblExpTipo = new QLabel(" Expoente do tipo:", this);
    m_lblExpTipo->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblExpTipo);

    m_comboIntReal = new QComboBox(this);
    m_comboIntReal->addItem("Real");
    m_comboIntReal->addItem("Inteiro");
    m_comboIntReal->addItem("Natural");
    m_comboIntReal->setCurrentIndex(0);
    m_mainToolbar->addWidget(m_comboIntReal);

    // RJn
    m_mainToolbar->addSeparator();
    m_lblRJn = new QLabel(" RJn:= ", this);
    m_lblRJn->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblRJn);

    m_editRJn = new QLineEdit("0.001", this);
    m_editRJn->setObjectName("LEEM");
    m_editRJn->setSizePolicy(sp);
    m_editRJn->setMaximumSize(QSize(40, 16777215));
    m_editRJn->setContextMenuPolicy(Qt::NoContextMenu);
    m_editRJn->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    m_mainToolbar->addWidget(m_editRJn);

    // NCy (número de ciclos)
    m_lblNCy = new QLabel(" NCy:= ", this);
    m_lblNCy->setSizePolicy(sp);
    m_mainToolbar->addWidget(m_lblNCy);

    m_editNCy = new QLineEdit("100", this);
    m_editNCy->setObjectName("LEENC");
    m_editNCy->setSizePolicy(sp);
    m_editNCy->setMaximumSize(QSize(40, 16777215));
    m_editNCy->setContextMenuPolicy(Qt::NoContextMenu);
    m_editNCy->setValidator(new QDoubleValidator(0.9, 0.999, 3, this));
    m_mainToolbar->addWidget(m_editNCy);

    // Medido/Estimado vs Dist. Resíduos
    m_mainToolbar->addSeparator();
    m_comboStyle = new QComboBox(this);
    m_comboStyle->addItem(QString::fromUtf8("Medido/Estimado"));
    m_comboStyle->addItem(QString::fromUtf8("Dist. Resíduos"));
    m_comboStyle->setCurrentIndex(0);
    m_mainToolbar->addWidget(m_comboStyle);

    // Checkboxes Medido / Estimado
    m_mainToolbar->addSeparator();
    m_checkMedido = new QCheckBox("Medido", this);
    m_checkMedido->setChecked(true);
    m_mainToolbar->addWidget(m_checkMedido);

    m_checkEstimado = new QCheckBox("Estimado", this);
    m_checkEstimado->setChecked(true);
    m_mainToolbar->addWidget(m_checkEstimado);

    // Saída Visível
    m_mainToolbar->addSeparator();
    m_mainToolbar->addWidget(new QLabel(QString::fromUtf8(" Saída Visível:= "), this));
    m_comboSaida = new QComboBox(this);
    m_mainToolbar->addWidget(m_comboSaida);
}

// ============================================================================
// initQwtPlot – réplica fiel de ICalc::ini_MW_qwtPlot1()
// ============================================================================
void MainWindow::initQwtPlot()
{
    QwtPlot* plot = m_ui->qwtPlot1;
    if (!plot) return;

    plot->setAutoReplot(false);
    plot->setCanvasBackground(QColor(Qt::white));

    // Panner (botão do meio)
    m_panner = new QwtPlotPanner(plot->canvas());
    m_panner->setMouseButton(Qt::MidButton);

    // Zoomers
    m_zoomer[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, plot->canvas());
    m_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
    m_zoomer[0]->setRubberBandPen(QColor(Qt::green));
    m_zoomer[0]->setTrackerMode(QwtPicker::ActiveOnly);
    m_zoomer[0]->setTrackerPen(QColor(Qt::black));

    m_zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, plot->canvas());

    // Picker (crosshair)
    m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        plot->canvas());
    m_picker->setRubberBandPen(QColor(Qt::darkBlue));
    m_picker->setRubberBand(QwtPicker::CrossRubberBand);
    m_picker->setTrackerPen(QColor(Qt::black));

    // Legend
    QwtLegend* legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    plot->insertLegend(legend, QwtPlot::TopLegend);

    // Grid
    QwtPlotGrid* grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(plot);

    // Eixos
    plot->enableAxis(QwtPlot::yLeft);
    plot->setAxisTitle(QwtPlot::xBottom, "Amostras");
    plot->setAxisTitle(QwtPlot::yLeft, "Saidas");
    plot->setAxisMaxMajor(QwtPlot::xBottom, 6);
    plot->setAxisMaxMinor(QwtPlot::xBottom, 10);

    // Curva "Medido"
    m_curveMedido = new QwtPlotCurve("Medido");
    m_curveMedido->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curveMedido->setPen(QPen(Qt::darkBlue, 1));
    m_curveMedido->setYAxis(QwtPlot::yLeft);
    m_curveMedido->setStyle(QwtPlotCurve::Lines);
    m_curveMedido->attach(plot);

    // Curva "Estimado"
    m_curveEstimado = new QwtPlotCurve("Estimado");
    m_curveEstimado->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curveEstimado->setPen(QPen(Qt::red, 1));
    m_curveEstimado->setYAxis(QwtPlot::yLeft);
    m_curveEstimado->setStyle(QwtPlotCurve::Lines);
    m_curveEstimado->attach(plot);

    // Marker
    m_marker = new QwtPlotMarker();
    m_marker->setValue(0.0, 0.0);
    m_marker->setLineStyle(QwtPlotMarker::VLine);
    m_marker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_marker->setLinePen(QPen(Qt::green, 0, Qt::DashLine));
    m_marker->attach(plot);
}

// ============================================================================
// setupConnections
// ============================================================================
void MainWindow::setupConnections()
{
    // ---- Actions da toolbar ------------------------------------------------
    connect(m_actionIni,      &QAction::triggered,
            this, &MainWindow::onActionIniciarTriggered);
    connect(m_actionParar,    &QAction::triggered,
            this, &MainWindow::onActionPararTriggered);
    connect(m_actionCarDados, &QAction::triggered,
            this, &MainWindow::onActionCarDadosTriggered);
    connect(m_actionZoom,     &QAction::toggled,
            this, &MainWindow::onActionZoomTriggered);
    connect(m_actionConfig,   &QAction::triggered,
            this, [this]() { if (m_dialogConfig) m_dialogConfig->open(); });
    connect(m_actionExit,     &QAction::triggered,
            this, &MainWindow::onActionExitTriggered);

    // ---- Widgets programáticos ---------------------------------------------
    connect(m_checkMedido,   &QCheckBox::stateChanged,
            this, &MainWindow::onCheckBox1Changed);
    connect(m_checkEstimado, &QCheckBox::stateChanged,
            this, &MainWindow::onCheckBox2Changed);
    connect(m_comboStyle,    QOverload<int>::of(&QComboBox::activated),
            this, &MainWindow::onStyleComboChanged);
    connect(m_comboSaida,    QOverload<int>::of(&QComboBox::activated),
            this, &MainWindow::onSaidaComboChanged);

    // ---- Botões do dialog de configuração ----------------------------------
    if (m_pbSalvarDadosTxT)
        connect(m_pbSalvarDadosTxT, &QPushButton::clicked,
                this, &MainWindow::onSalvarDadosTxTClicked);
    if (m_pbSalvarDados)
        connect(m_pbSalvarDados, &QPushButton::clicked,
                this, &MainWindow::onSalvarConfigClicked);
    if (m_pbOpenDados)
        connect(m_pbOpenDados, &QPushButton::clicked,
                this, &MainWindow::onOpenConfigClicked);
    if (m_pbConcDados)
        connect(m_pbConcDados, &QPushButton::clicked,
                this, &MainWindow::onConcatenarClicked);

    // ---- DataLoaderDialog ---------------------------------------------------
    connect(m_dataLoaderDialog, &DataLoaderDialog::dataLoadRequested,
            this, &MainWindow::onDataLoadRequested);

    // ---- Picker ------------------------------------------------------------
    connect(m_picker, SIGNAL(moved(const QPoint&)),
            this, SLOT(onPickerMoved(const QPoint&)));

    // ---- Mensagem inicial --------------------------------------------------
    appendTextLog("-> Aplicação iniciada");
    appendTextLog(QString("-> Versão Refatorada | %1")
                  .arg(QDateTime::currentDateTime().toString()));
}

// ============================================================================
// updateUIState
// ============================================================================
void MainWindow::updateUIState()
{
    m_actionIni->setEnabled(!m_algorithmRunning);
    m_actionParar->setEnabled(m_algorithmRunning);
    m_actionCarDados->setEnabled(!m_algorithmRunning);
    m_actionConfig->setEnabled(!m_algorithmRunning);
}

// ============================================================================
// appendTextLog
// ============================================================================
void MainWindow::appendTextLog(const QString& message)
{
    if (m_ui && m_ui->textEdit) {
        QDateTime now = QDateTime::currentDateTime();
        QString ts = now.toString("hh:mm:ss.zzz");
        m_ui->textEdit->append(QString("[%1] %2").arg(ts, message));
    }
}

// ============================================================================
// initInterface widgets – toolbar slots
// ============================================================================
void MainWindow::onCheckBox1Changed(int state)
{
    // Mostrar/ocultar curva "Medido"
    if (m_curveMedido) {
        if (state == Qt::Checked)
            m_curveMedido->attach(m_ui->qwtPlot1);
        else
            m_curveMedido->detach();
        m_ui->qwtPlot1->replot();
    }
}

void MainWindow::onCheckBox2Changed(int state)
{
    // Mostrar/ocultar curva "Estimado"
    if (m_curveEstimado) {
        if (state == Qt::Checked)
            m_curveEstimado->attach(m_ui->qwtPlot1);
        else
            m_curveEstimado->detach();
        m_ui->qwtPlot1->replot();
    }
}

void MainWindow::onStyleComboChanged(int index)
{
    m_currentStyle = index;
    QwtPlot* plot = m_ui->qwtPlot1;
    if (!plot) return;

    if (index == 0) {
        // Medido / Estimado
        m_checkMedido->setDisabled(false);
        m_checkEstimado->setDisabled(false);
        plot->setAxisTitle(QwtPlot::yLeft, "Saidas");
        plot->setAxisTitle(QwtPlot::xBottom, "Amostras");
        m_curveMedido->attach(plot);
        m_curveEstimado->setTitle("Estimado");
    } else {
        // Distribuição de Resíduos
        plot->setAxisTitle(QwtPlot::yLeft, "Quantidade de Resíduos");
        plot->setAxisTitle(QwtPlot::xBottom, "Valores dos Resíduos");
        m_curveEstimado->setTitle(QString::fromUtf8("Distribuição dos Resíduos"));
        m_curveMedido->detach();
        m_checkMedido->setDisabled(true);
        m_checkEstimado->setDisabled(true);
    }
    plot->replot();
}

void MainWindow::onSaidaComboChanged(int index)
{
    m_currentOutput = index;
    // Redesenhar com dados da saída selecionada (delegado ao presenter)
    if (m_presenter)
        m_presenter->loadData(QString::number(index));
}

// ============================================================================
// Config dialog button slots (delegam para presenter)
// ============================================================================
void MainWindow::onSalvarDadosTxTClicked()
{
    if (m_presenter)
        m_presenter->saveDataFile("");
}

void MainWindow::onSalvarConfigClicked()
{
    if (m_presenter)
        m_presenter->saveConfiguration("");
}

void MainWindow::onOpenConfigClicked()
{
    if (m_presenter)
        m_presenter->loadConfiguration("");
}

void MainWindow::onConcatenarClicked()
{
    if (m_presenter)
        m_presenter->saveDataFile("");
}

// ============================================================================
// Zoom / picker slots
// ============================================================================
void MainWindow::onActionZoomTriggered(bool on)
{
    if (m_panner)    m_panner->setEnabled(on);
    if (m_zoomer[0]) { m_zoomer[0]->setEnabled(on); m_zoomer[0]->zoom(0); }
    if (m_zoomer[1]) { m_zoomer[1]->setEnabled(on); m_zoomer[1]->zoom(0); }
    if (m_picker)    m_picker->setEnabled(!on);

    QwtPlot* plot = m_ui->qwtPlot1;
    if (plot) {
        plot->setAxisAutoScale(QwtPlot::yLeft);
        plot->setAxisAutoScale(QwtPlot::xBottom);
    }
}

void MainWindow::onPickerMoved(const QPoint& pos)
{
    QwtPlot* plot = m_ui->qwtPlot1;
    if (!plot) return;
    plot->invTransform(QwtPlot::xBottom, pos.x());
    plot->invTransform(QwtPlot::yLeft,   pos.y());
    plot->invTransform(QwtPlot::yRight,  pos.y());
}

// ============================================================================
// Actions da toolbar (lógica delega ao presenter)
// ============================================================================
void MainWindow::onActionIniciarTriggered()
{
    if (!m_presenter) return;

    // Ler valores da toolbar e configurar o presenter (réplica do original)
    auto& config = m_presenter->getConfiguration();
    config.getAlgorithmData().populationSize = m_editN->text().toInt();
    config.getAlgorithmData().elitismIndex   = m_editE->text().toInt();
    config.setSSE(m_editSERR->text().toDouble());
    config.setJNRR(m_editRJn->text().toDouble());
    config.setCycleCount(m_editNCy->text().toInt());
    config.setRational(m_comboPolRac->currentIndex() == 0);
    config.setExponentType(m_comboIntReal->currentIndex());
    config.setCreated(true);

    // ── Resume from JNRR pause (like original: "Continuar") ──────────
    if (m_algorithmPaused) {
        appendTextLog(QString("-> Continuando algoritmo DE... RJn=%1, NCy=%2, SERR=%3")
            .arg(m_editRJn->text()).arg(m_editNCy->text()).arg(m_editSERR->text()));

        // Update runtime params in the running service (like original updates
        // iteracoesAnt and reads new RJn/NCy/SERR/PolRac/IntReal)
        auto* deService = dynamic_cast<Services::DifferentialEvolutionService*>(
            m_presenter->algorithmService());
        if (deService) {
            deService->updateRuntimeParams(
                config.getJNRR(),
                config.getCycleCount(),
                config.getSSE(),
                config.isRational(),
                config.getExponentType());
        }

        m_algorithmPaused = false;
        m_actionParar->setEnabled(true);
        m_actionIni->setEnabled(false);
        m_editN->setEnabled(false);
        m_editE->setEnabled(false);
        m_editRJn->setEnabled(false);
        m_editNCy->setEnabled(false);
        m_editSERR->setEnabled(false);
        m_comboPolRac->setEnabled(false);
        m_comboIntReal->setEnabled(false);

        m_presenter->resumeAlgorithm();
        return;
    }

    // ── Fresh start ──────────────────────────────────────────────────
    appendTextLog(QString("-> Iniciando algoritmo DE... N=%1, E=%2, SERR=%3, RJn=%4, NCy=%5")
        .arg(m_editN->text()).arg(m_editE->text())
        .arg(m_editSERR->text()).arg(m_editRJn->text()).arg(m_editNCy->text()));

    m_algorithmRunning = true;
    m_algorithmPaused = false;
    m_actionCarDados->setEnabled(false);
    m_actionParar->setEnabled(true);
    m_actionIni->setEnabled(false);
    m_editN->setEnabled(false);
    m_editE->setEnabled(false);

    m_presenter->startAlgorithm();
}

void MainWindow::onActionPararTriggered()
{
    if (!m_presenter) return;

    appendTextLog("-> Parando algoritmo...");
    m_presenter->stopAlgorithm();
    // Não reseta m_algorithmRunning aqui —
    // será resetado quando onAlgorithmFinished() for chamado via signal
    statusBar()->showMessage(QString::fromUtf8("Parando algoritmo..."));
}

void MainWindow::onActionCarDadosTriggered()
{
    appendTextLog("-> Abrindo tela de carregamento de dados...");
    m_dataLoaderDialog->resetStatus();
    m_dataLoaderDialog->exec();
    onCarDadosDialogClosed();
}

void MainWindow::onCarDadosDialogClosed()
{
    // Réplica de slot_MW_CarDadosExit: habilitar botões se dados foram carregados
    // Por enquanto, habilita o botão Iniciar após carregar
    m_actionIni->setEnabled(true);
    m_pbSalvarDadosTxT->setEnabled(true);
    appendTextLog("-> Tela de carregamento fechada.");
}

void MainWindow::onDataLoadRequested(const QString& fileName,
                                      const QList<qint32>& selectedVariables,
                                      qint32 outputCount,
                                      double timeInitial, double timeFinal,
                                      bool /*saveToFile*/)
{
    if (!m_presenter) return;
    appendTextLog(QString("-> Carregando dados de: %1").arg(fileName));

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        appendTextLog("-> ERRO: Não foi possível abrir o arquivo.");
        return;
    }

    QTextStream stream(&file);

    // --- 1) Ler cabeçalho (primeira linha) ---
    QString headerLine = stream.readLine();
    if (headerLine.isNull()) { file.close(); return; }
    QStringList allHeaders = headerLine.split(QRegularExpression("(\\s+)"), Qt::SkipEmptyParts);

    // Nomes das variáveis selecionadas
    QList<QString> varNames;
    for (int i = 0; i < selectedVariables.size(); ++i) {
        qint32 idx = selectedVariables.at(i);
        if (idx >= 0 && idx < allHeaders.size())
            varNames.append(allHeaders.at(idx));
        else
            varNames.append(QString("Var%1").arg(idx));
    }

    const qint32 numVars = selectedVariables.size(); // linhas da matriz

    // --- 2) Ler todas as linhas de dados ---
    QList<QList<qreal>> rawData; // rawData[coluna][variável]
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;

        // Primeiro campo pode ser tempo
        bool isNumber = false;
        parts.at(0).toDouble(&isNumber);
        if (!isNumber) continue;

        // Filtrar por intervalo de tempo
        double t = parts.at(0).toDouble();
        if (t < timeInitial || t > timeFinal) continue;

        QList<qreal> row;
        for (int i = 0; i < selectedVariables.size(); ++i) {
            qint32 colIdx = selectedVariables.at(i);
            if (colIdx < parts.size())
                row.append(parts.at(colIdx).toDouble());
            else
                row.append(0.0);
        }
        rawData.append(row);
    }
    file.close();

    const qint32 numSamples = rawData.size();
    if (numSamples == 0) {
        appendTextLog("-> ERRO: Nenhuma amostra válida encontrada no arquivo.");
        return;
    }

    // --- 3) Preencher MathMatrix (linhas=variáveis, colunas=amostras) ---
    Utils::MathMatrix<qreal> dataMatrix(numVars, numSamples);
    QList<qreal> maxVals, minVals;
    for (qint32 v = 0; v < numVars; ++v) {
        maxVals.append(-std::numeric_limits<qreal>::max());
        minVals.append( std::numeric_limits<qreal>::max());
    }

    for (qint32 s = 0; s < numSamples; ++s) {
        for (qint32 v = 0; v < numVars; ++v) {
            qreal val = rawData.at(s).at(v);
            dataMatrix.at(v, s) = val;
            if (val > maxVals[v]) maxVals[v] = val;
            if (val < minVals[v]) minVals[v] = val;
        }
    }

    // --- 4) Preencher Configuration/SimulationData ---
    auto& config = m_presenter->getConfiguration();
    auto& simData = config.getAlgorithmData().variables;
    simData.clear();
    simData.setOutputCount(outputCount);
    simData.setNames(varNames);
    simData.setMaxValues(maxVals);
    simData.setMinValues(minVals);
    simData.setValues(dataMatrix);

    config.getAlgorithmData().initialTime = timeInitial;
    config.getAlgorithmData().finalTime   = timeFinal;
    config.setCreated(true);

    appendTextLog(QString("-> Dados carregados: %1 variáveis, %2 amostras, %3 saídas")
        .arg(numVars).arg(numSamples).arg(outputCount));
    appendTextLog(QString("-> Tempo: %1 a %2").arg(timeInitial).arg(timeFinal));

    // Atualizar combo de saída com nomes das variáveis de saída
    m_comboSaida->clear();
    for (qint32 i = 0; i < outputCount && i < varNames.size(); ++i)
        m_comboSaida->addItem(varNames.at(i));

    // Habilitar botão Iniciar
    m_actionIni->setEnabled(true);
    if (m_pbSalvarDadosTxT) m_pbSalvarDadosTxT->setEnabled(true);
}

void MainWindow::onActionConfigTriggered()
{
    if (m_dialogConfig)
        m_dialogConfig->open();
}

void MainWindow::onActionExitTriggered()
{
    close();
}

// ============================================================================
// Presenter feedback slots
// ============================================================================
void MainWindow::onStatusUpdated(qint64 iterations,
                                 const QVector<qreal>& errors,
                                 const QVector<Domain::Chromosome>& /*bestChromosomes*/)
{
    if (!errors.isEmpty()) {
        QString msg = QString("Iteração: %1 | Melhor BIC: %2 | MSE: %3")
            .arg(iterations)
            .arg(errors[0], 0, 'g', 6)
            .arg(!errors.isEmpty() ? errors[0] : 0.0, 0, 'g', 6);
        statusBar()->showMessage(msg);
    }
    if (iterations % 100 == 0) {
        appendTextLog(QString("-> Iteração %1 | Melhor BIC: %2")
            .arg(iterations)
            .arg(!errors.isEmpty() ? QString::number(errors[0], 'g', 6) : "N/A"));
    }
}

void MainWindow::onAlgorithmFinished()
{
    m_algorithmRunning = false;
    m_algorithmPaused = false;
    m_closeRequested = false;
    m_actionIni->setEnabled(true);
    m_actionParar->setEnabled(false);
    m_actionCarDados->setEnabled(true);
    m_editN->setEnabled(true);
    m_editE->setEnabled(true);
    m_editRJn->setEnabled(true);
    m_editNCy->setEnabled(true);
    m_editSERR->setEnabled(true);
    m_comboPolRac->setEnabled(true);
    m_comboIntReal->setEnabled(true);
    updateUIState();

    appendTextLog("-> Algoritmo finalizado com sucesso!");
    statusBar()->showMessage("Algoritmo finalizado", 3000);

    // ── Gerar saída formatada em Python para cada saída ──────────────────
    if (m_presenter) {
        const auto& config = m_presenter->getConfiguration();
        const qint32 qtSaidas = config.getAlgorithmData().variables.getOutputCount();
        for (qint32 s = 0; s < qtSaidas; ++s) {
            Domain::Chromosome best = m_presenter->getBestChromosome(s);
            if (best.getRegressions().isEmpty()) continue;
            QString pyCode = formatChromosomePython(best, config, s);
            // Mostra no log separado visualmente
            appendTextLog("════════════════════════════════════════════════");
            appendTextLog(QString("  RESULTADO SAÍDA %1  —  Código Python").arg(s));
            appendTextLog("════════════════════════════════════════════════");
            appendTextLog(pyCode);
            appendTextLog("════════════════════════════════════════════════");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// formatChromosomePython — gera código Python pronto para colar e executar
////////////////////////////////////////////////////////////////////////////////
QString MainWindow::formatChromosomePython(const Domain::Chromosome& cr,
                                           const Domain::Configuration& config,
                                           qint32 outputId) const
{
    using Domain::CompositeTerm;
    const auto& regs  = cr.getRegressions();
    const auto& coefs = cr.getCoefficients();
    const auto& errs  = cr.getErrors();
    const auto& vars  = config.getAlgorithmData().variables;
    const auto& names = vars.getNames();
    const quint8 expoType = config.getExponentType();

    QString outName = (outputId < names.size()) ? names.at(outputId) : QString("y%1").arg(outputId);

    // ── Separar termos do numerador, denominador e erro ──────────────────
    QStringList numTerms, denTerms, errTerms;
    QStringList numERR, denERR;
    QString constStr;
    bool hasConst = false;

    for (qint32 r = 0; r < regs.size(); ++r) {
        if (r >= coefs.size()) break;
        qreal c = coefs[r];
        if (c == 0.0) continue;

        const auto& rg = regs[r];
        bool isNum = rg.at(0).isNumerator();
        quint32 regId = rg.at(0).getRegression();

        // ── Montar string do regressor (produto de termos) ──────────────
        QString regStr;
        if (regId == 0) {
            // Constante
            hasConst = true;
            if (isNum) {
                constStr = QString::number(c, 'g', 15);
                numERR.append(QString::number(r < errs.size() ? errs[r] : -1));
            } else {
                // Denominador constante → coef + 1
                denTerms.prepend(QString::number(c + 1.0, 'g', 15));
                denERR.prepend(QString::number(r < errs.size() ? errs[r] : -1));
            }
            continue;
        }

        for (qint32 t = 0; t < rg.size(); ++t) {
            const CompositeTerm& term = rg[t];
            quint32 varId  = term.getVariable();
            quint32 delay  = term.getDelay();
            qreal   expo   = term.getExponent();

            // Arredondamento de expoente por tipo (fiel ao original)
            if (expoType == 1) { // Inteiro
                qint32 iexp = (qint32)expo;
                expo = iexp + ((expo - iexp) >= 0.5 ? 1 : ((expo - iexp) <= -0.5 ? -1 : 0));
            } else if (expoType == 2) { // Natural (abs inteiro)
                qint32 iexp = (qint32)expo;
                expo = qAbs(iexp + ((expo - iexp) >= 0.5 ? 1 : ((expo - iexp) <= -0.5 ? -1 : 0)));
            }

            if (expo == 0.0) continue;

            QString varName;
            if (varId >= 1 && (varId - 1) < (quint32)names.size())
                varName = names.at(varId - 1);
            else if (varId == 0)
                varName = "E";
            else
                varName = QString("x%1").arg(varId);

            // Python: data['varName'][k-delay]  ou  E[k-delay]
            QString baseAccess;
            if (varId == 0)
                baseAccess = QString("E[k-%1]").arg(delay);
            else
                baseAccess = QString("data['%1'][k-%2]").arg(varName).arg(delay);

            // Formatar de acordo com o tipo de base
            Domain::BasisType bt = term.getBasisType();
            QString termStr;
            switch (bt) {
            case Domain::BasisAbsPow:
                if (expo == 1.0)
                    termStr = QString("np.abs(%1)").arg(baseAccess);
                else
                    termStr = QString("(np.abs(%1)**%2)").arg(baseAccess).arg(expo, 0, 'g', 15);
                break;
            case Domain::BasisLogPow:
                if (expo == 1.0)
                    termStr = QString("np.log(1+np.abs(%1))").arg(baseAccess);
                else
                    termStr = QString("(np.log(1+np.abs(%1))**%2)").arg(baseAccess).arg(expo, 0, 'g', 15);
                break;
            case Domain::BasisExp:
                // expo armazena alpha
                termStr = QString("np.exp(%1*%2)").arg(expo, 0, 'g', 15).arg(baseAccess);
                break;
            case Domain::BasisTanhPow:
                if (expo == 1.0)
                    termStr = QString("np.tanh(%1)").arg(baseAccess);
                else
                    termStr = QString("(np.tanh(%1)**%2)").arg(baseAccess).arg(expo, 0, 'g', 15);
                break;
            default: // BasisPow — original
                termStr = baseAccess;
                if (expo != 1.0)
                    termStr = QString("(%1**%2)").arg(termStr).arg(expo, 0, 'g', 15);
                break;
            }

            if (!regStr.isEmpty()) regStr += " * ";
            regStr += termStr;
        }

        if (regStr.isEmpty()) continue;

        QString fullTerm = QString("%1 * %2").arg(c, 0, 'g', 15).arg(regStr);

        if (isNum) {
            numTerms.append(fullTerm);
            numERR.append(QString::number(r < errs.size() ? errs[r] : -1, 'g', 6));
        } else {
            denTerms.append(fullTerm);
            denERR.append(QString::number(r < errs.size() ? errs[r] : -1, 'g', 6));
        }
    }

    // ── Termos de erro (coeficientes além dos regressores) ───────────────
    const qint32 tamErro = coefs.size() - regs.size();
    for (qint32 e = 0; e < tamErro; ++e) {
        qreal c = coefs[regs.size() + e];
        errTerms.append(QString("%1 * E[k-%2]").arg(c, 0, 'g', 15).arg(e + 1));
    }

    // ── Montar string da equação ─────────────────────────────────────────
    QString numStr;
    if (!constStr.isEmpty()) numStr = constStr;
    for (const auto& s : numTerms) {
        if (!numStr.isEmpty()) numStr += " + ";
        numStr += s;
    }
    if (numStr.isEmpty()) numStr = "0";

    QString errStr;
    for (const auto& s : errTerms) {
        if (!errStr.isEmpty()) errStr += " + ";
        errStr += s;
    }

    QString denStr;
    bool hasDenConst = false;
    for (const auto& s : denTerms) {
        bool isConst = false;
        // Tenta detectar se é só número (constante do denominador)
        s.toDouble(&isConst);
        if (isConst && !hasDenConst) {
            denStr = s;
            hasDenConst = true;
        } else {
            if (!denStr.isEmpty()) denStr += " + ";
            else denStr = "1 + ";
            denStr += s;
        }
    }
    if (denStr.isEmpty()) denStr = "1";
    else if (!hasDenConst && !denStr.startsWith("1 + ")) denStr = "1 + " + denStr;

    // ── Métricas ─────────────────────────────────────────────────────────
    qreal bic = cr.getFitness();
    qreal mse = cr.getError();
    qint32 maxDelay = cr.getMaxDelay();
    qint64 iters = config.getIterations();

    // ── Gerar código Python ──────────────────────────────────────────────
    QString py;
    py += "import numpy as np\n";
    py += "\n";
    py += QString("# ===== Modelo Identificado: %1 =====\n").arg(outName);
    py += QString("# BIC  = %1\n").arg(bic, 0, 'g', 10);
    py += QString("# MSE  = %1\n").arg(mse, 0, 'g', 10);
    py += QString("# Iter = %1\n").arg(iters);
    py += QString("# MaxDelay = %1\n").arg(maxDelay);

    // Info das variáveis
    for (qint32 i = 0; i < names.size(); ++i) {
        py += QString("# %1: Max=%2, Min=%3")
            .arg(names[i])
            .arg(i < vars.getMaxValues().size() ? vars.getMaxValues()[i] : 0, 0, 'g', 6)
            .arg(i < vars.getMinValues().size() ? vars.getMinValues()[i] : 0, 0, 'g', 6);
        if (i < config.getDecimation().size())
            py += QString(", Dec=%1").arg(config.getDecimation()[i]);
        py += "\n";
    }

    py += "#\n";
    py += QString("# %1_Num = %2\n").arg(outName, numStr);
    if (!errStr.isEmpty())
        py += QString("# %1_Err = %2\n").arg(outName, errStr);
    py += QString("# %1_Den = %2\n").arg(outName, denStr);
    py += QString("# ERR_Num = [%1]\n").arg(numERR.join(", "));
    py += QString("# ERR_Den = [%1]\n").arg(denERR.join(", "));
    py += "#\n";
    py += QString("# Equação: %1(k) = (%1_Num + %1_Err) / (%1_Den)\n").arg(outName);
    py += "\n";

    // ── Função Python ────────────────────────────────────────────────────
    py += QString("def model_%1(data, N):\n").arg(outName);
    py += "    \"\"\"\n";
    py += QString("    Modelo identificado para saída '%1'.\n").arg(outName);
    py += "    \n";
    py += "    Parâmetros:\n";
    py += QString("        data : dict com chaves = nomes das variáveis,\n");
    py += QString("               cada valor é um np.array de tamanho N.\n");
    py += QString("               Ex: data['%1'] = np.array([...])  # saída medida\n").arg(outName);
    for (qint32 i = 0; i < names.size(); ++i) {
        if (i == outputId) continue;
        py += QString("               data['%1'] = np.array([...])  # entrada\n").arg(names[i]);
    }
    py += "        N    : número de amostras\n";
    py += "    \n";
    py += "    Retorna:\n";
    py += "        y_est : np.array com valores estimados\n";
    py += "        E     : np.array com resíduos (erro)\n";
    py += "    \"\"\"\n";
    py += "    y_est = np.zeros(N)\n";
    py += "    E = np.zeros(N)\n";
    py += "\n";
    py += QString("    for k in range(%1, N):\n").arg(maxDelay);
    py += "        # Numerador\n";
    py += QString("        num = %1\n").arg(numStr);
    if (!errStr.isEmpty()) {
        py += "        # Termos de erro\n";
        py += QString("        err = %1\n").arg(errStr);
    } else {
        py += "        err = 0.0\n";
    }
    py += "        # Denominador\n";
    py += QString("        den = %1\n").arg(denStr);
    py += "        # Saída estimada\n";
    py += "        y_est[k] = (num + err) / den\n";
    py += QString("        E[k] = data['%1'][k] - y_est[k]\n").arg(outName);
    py += "\n";
    py += "    return y_est, E\n";

    return py;
}

void MainWindow::onAlgorithmPaused()
{
    m_algorithmPaused = true;
    m_actionIni->setEnabled(true);
    m_editRJn->setEnabled(true);
    m_editNCy->setEnabled(true);
    m_editSERR->setEnabled(true);
    m_comboPolRac->setEnabled(true);
    m_comboIntReal->setEnabled(true);

    appendTextLog("-> Algoritmo pausado");
    statusBar()->showMessage("Algoritmo pausado");
}

void MainWindow::onErrorOccurred(const QString& message)
{
    m_algorithmRunning = false;
    m_algorithmPaused = false;
    updateUIState();

    appendTextLog(QString("-> ERRO: %1").arg(message));
    statusBar()->showMessage("ERRO: " + message, 5000);
    QMessageBox::critical(this, "Erro",
        QString("Erro durante execução:\n%1").arg(message));
}

void MainWindow::onStatusMessageChanged(const QString& message)
{
    appendTextLog(QString("-> %1").arg(message));
}

// ============================================================================
// closeEvent
// ============================================================================
void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_algorithmRunning && m_presenter) {
        if (!m_closeRequested) {
            // Primeira tentativa: pede para parar e avisa o usuário
            m_closeRequested = true;
            m_presenter->stopAlgorithm();
            statusBar()->showMessage(QString::fromUtf8("Finalizando algoritmo... feche novamente para forçar."));
            event->ignore();
            return;
        }
        // Segunda tentativa: força o fechamento
        qWarning() << "[MainWindow] Forçando fechamento — worker thread pode ainda estar ativa";
        m_algorithmRunning = false;
    }
    event->accept();
}

} // namespace Presentation
