#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ui_imainwindow.h"
#include "ui_dialogConfig.h"
#include "xtipodados.h"

#include <QMainWindow>
#include <QVector>
#include <QDateTime>

class QwtPlotZoomer;
class QwtPlotPanner;
class QwtPlotPicker;
class QwtPlotCurve;
class QwtPlotMarker;
class QLabel;
class QAction;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QToolBar;
class QDialog;

class SharedState;
class ThreadWorker;
class IEquationFormatter;
class IConfigPersistence;
class ICarregar;

///////////////////////////////////////////////////////////////////////////////
/// DummyBase — mesmo padrão do original para herança de UI
///////////////////////////////////////////////////////////////////////////////
class DummyBase : public QMainWindow, protected Ui::MainWindow
{
public:
    DummyBase(QWidget *parent = nullptr) : QMainWindow(parent) {}
};

///////////////////////////////////////////////////////////////////////////////
/// MainWindow — Janela principal refatorada
///
/// SRP: Gerencia a UI principal e coordena sinais com as threads.
///      Delega formatação de equação para IEquationFormatter.
///      Delega persistência de config para IConfigPersistence.
///////////////////////////////////////////////////////////////////////////////
class MainWindow : public DummyBase
{
    Q_OBJECT

public:
    MainWindow(SharedState *state, QWidget *parent = nullptr);
    ~MainWindow();

    void setEquationFormatter(IEquationFormatter *formatter);
    void setConfigPersistence(IConfigPersistence *persistence);

signals:
    void signal_MW_Estado(const quint16 &std) const;
    void signal_MW_EquacaoEscrita() const;
    void signal_MW_StatusSetado();

public slots:
    void slot_MW_Finalizado();
    void slot_MW_Parado();
    void slot_MW_closed();

private slots:
    void slot_MW_changeCheckBox1(int st);
    void slot_MW_changeCheckBox2(int st);
    void slot_MW_CarDados();
    void slot_MW_FName(const QString &fileName);
    void slot_MW_IniciarFinalizar();
    void slot_MW_PararContinuar();
    void slot_MW_SetStatus(const volatile qint64 &iteracoes,
                           const QVector<qreal> *somaEr,
                           const QList<QVector<qreal> > *resObtido,
                           const QList<QVector<qreal> > *residuo,
                           const QVector<Cromossomo> *crBest);
    void slot_MW_Desenha();
    void slot_MW_EscreveEquacao();
    void slot_MW_Finalizar();
    void slot_MW_CarDadosExit();
    void slot_MW_SalvarArquivo();
    void slot_MW_saveAsConfig();
    void slot_MW_openConfig();
    void slot_MW_changeStyle(const int &changeStyle);
    void slot_MW_changeVerSaida(const int &idSaida);

private slots:
    void moved1(const QPoint &);
    void enableZoomMode(bool);

private:
    void closeEvent(QCloseEvent *event) override;
    void ini_MW_interface();
    void ini_MW_qwtPlot1();
    void Iniciar();

    SharedState          *m_state;
    IEquationFormatter   *m_eqFormatter;
    IConfigPersistence   *m_configPersist;

    QVector<ThreadWorker *> m_workers;

    QToolBar *mainToolbar;
    QAction *actionExit, *actionIni, *actionParar, *actionCarDados, *actionConfig, *actionZoom;

    QwtPlotZoomer *MW_zoomer1[2];
    QPushButton *pbSalvarDadosTxT, *pbSalvarDados, *pbOpenDados, *pbConcDados;

    QCheckBox *CheckBox1, *CheckBox2;

    QLabel *LN, *LEL, *LESE, *LEM, *LENC, *LEIR, *LDI, *LPI;
    QLineEdit *LEN, *LEEL, *LEESE, *LEEM, *LEENC, *LEDI, *LEPI;
    QComboBox *styleComboBox, *saidaComboBox, *polRacComboBox, *intRealComboBox;
    QString NameConfigSave;
    QDialog *dialogConfig;

    quint32 MW_SaidaUsada, MW_changeStyle;

    QDateTime MW_tempo;
    QVector<qreal> MW_bestFeito;
    QList<QVector<qreal> > MW_resObtido, MW_residuo, MW_best;
    QVector<qreal> MW_somaEr;
    QVector<Cromossomo> MW_crBest;

    ICarregar *cr;

    QwtPlotPanner *MW_panner1;
    QwtPlotMarker *MW_mrk1;
    QwtPlotPicker *MW_picker1;
    QwtPlotCurve *MW_crv_R, *MW_crv_E, *MW_crv_C;

    bool volatile isThClose;
    QStringList MW_strList;
    qint64 volatile MW_iteracoes;
};

#endif // MAIN_WINDOW_H
