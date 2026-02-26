#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QList>
#include <QDateTime>
#include <memory>
#include "MainWindowPresenter.h"
#include "DataLoaderDialog.h"
#include "../domain/Chromosome.h"

// Forward declarations
namespace Ui {
    class MainWindow;
    class DialogConfig;
}
class QToolBar;
class QAction;
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QDialog;
class QwtPlot;
class QwtPlotZoomer;
class QwtPlotPanner;
class QwtPlotPicker;
class QwtPlotCurve;
class QwtPlotMarker;

namespace Presentation {

/**
 * @brief Classe MainWindow - View completa no padrão MVP
 *
 * Replica fielmente a interface do ICalc original:
 * - Toolbar com labels, line-edits, combos, checkboxes
 * - QwtPlot com zoomer, panner, picker, curvas e legendas
 * - Dialog de configuração
 * - Delegate de lógica para MainWindowPresenter
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void setPresenter(std::shared_ptr<MainWindowPresenter> presenter);

signals:
    void algorithmStateRequested(quint16 state);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Toolbar actions
    void onActionIniciarTriggered();
    void onActionPararTriggered();
    void onActionCarDadosTriggered();
    void onActionZoomTriggered(bool on);
    void onActionConfigTriggered();
    void onActionExitTriggered();

    // Toolbar widgets
    void onCheckBox1Changed(int state);
    void onCheckBox2Changed(int state);
    void onStyleComboChanged(int index);
    void onSaidaComboChanged(int index);

    // Data loader dialog
    void onCarDadosDialogClosed();
    void onDataLoadRequested(const QString& fileName,
                             const QList<qint32>& selectedVariables,
                             qint32 outputCount,
                             double timeInitial, double timeFinal,
                             bool saveToFile);

    // Config dialog buttons
    void onSalvarDadosTxTClicked();
    void onSalvarConfigClicked();
    void onOpenConfigClicked();
    void onConcatenarClicked();

    // Presenter feedback
    void onStatusUpdated(qint64 iterations,
                        const QVector<qreal>& errors,
                        const QVector<Domain::Chromosome>& bestChromosomes);
    void onAlgorithmFinished();
    void onAlgorithmPaused();
    void onErrorOccurred(const QString& message);
    void onStatusMessageChanged(const QString& message);

    // Plot interaction
    void onPickerMoved(const QPoint& pos);

private:
    // Inicialização (equivale a ini_MW_interface + ini_MW_qwtPlot1)
    void initInterface();
    void initQwtPlot();
    void setupConnections();
    void updateUIState();
    void appendTextLog(const QString& message);
    QString formatChromosomePython(const Domain::Chromosome& cr,
                                   const Domain::Configuration& config,
                                   qint32 outputId) const;

private:
    // UI Designer
    std::unique_ptr<Ui::MainWindow> m_ui;
    std::shared_ptr<MainWindowPresenter> m_presenter;

    // Estado do algoritmo
    bool m_algorithmRunning;
    bool m_algorithmPaused;
    bool m_closeRequested;

    // Toolbar programática – referências a widgets do .ui localizados via findChild
    QToolBar* m_mainToolbar;
    QAction*  m_actionExit;
    QAction*  m_actionIni;
    QAction*  m_actionParar;
    QAction*  m_actionCarDados;
    QAction*  m_actionConfig;
    QAction*  m_actionZoom;

    // Widgets criados programaticamente na toolbar
    QLabel*    m_lblN;
    QLineEdit* m_editN;        // Tamanho da população (N)
    QLabel*    m_lblE;
    QLineEdit* m_editE;        // Elitismo (E)
    QLabel*    m_lblSERR;
    QLineEdit* m_editSERR;     // SERR
    QComboBox* m_comboPolRac;   // Racional / Polinomial
    QLabel*    m_lblExpTipo;
    QComboBox* m_comboIntReal;  // Real / Inteiro / Natural
    QLabel*    m_lblRJn;
    QLineEdit* m_editRJn;      // RJn
    QLabel*    m_lblNCy;
    QLineEdit* m_editNCy;      // NCy (número de ciclos)
    QComboBox* m_comboStyle;    // Medido/Estimado vs Dist. Resíduos
    QCheckBox* m_checkMedido;
    QCheckBox* m_checkEstimado;
    QComboBox* m_comboSaida;    // Saída Visível

    // Dialog de configuração
    QDialog* m_dialogConfig;
    QPushButton* m_pbSalvarDadosTxT;
    QPushButton* m_pbSalvarDados;
    QPushButton* m_pbOpenDados;
    QPushButton* m_pbConcDados;

    // Dialog de carregamento de dados (réplica do ICarregar)
    DataLoaderDialog* m_dataLoaderDialog;

    // Qwt plot objects
    QwtPlotZoomer* m_zoomer[2];
    QwtPlotPanner* m_panner;
    QwtPlotPicker* m_picker;
    QwtPlotCurve*  m_curveMedido;
    QwtPlotCurve*  m_curveEstimado;
    QwtPlotMarker* m_marker;

    // Dados para o gráfico
    quint32 m_currentOutput;
    quint32 m_currentStyle;
};

} // namespace Presentation

#endif // MAINWINDOW_H
