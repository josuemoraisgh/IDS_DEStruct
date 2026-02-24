#ifndef IMAINWINDOW_H
#define IMAINWINDOW_H

#include "designerworkaround.h"
#include "ui_imainwindow.h"
#include "ui_dialogConfig.h"

#include <qmainwindow.h>
#include <QMouseEvent>
#include <QVector>

#include <xmatriz.h>
#include <qwt_plot.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

class QwtPlotZoomer;
class QLabel;
class QTimer;
class QAction;
class QCoreApplication;
class QDateTime;
class DEStruct;
class ICarregar;
class XmlReaderWriter;
class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtLinearScaleEngine;
#include "xtipodados.h"
class DummyBase : public QMainWindow, protected Ui::MainWindow//, protected DesignerWorkaround
{
  public:
    DummyBase(QWidget* parent = 0)
      : QMainWindow(parent)
    {
    }
};

class ICalc : public DummyBase
{
        Q_OBJECT
    public:
        ICalc( QWidget* parent = 0);
        ~ICalc();
        QVector<DEStruct *> *ags;

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
        void slot_MW_SetStatus(const volatile qint64 &iteracoes,const QVector<qreal> *somaEr,const QList<QVector<qreal> > *resObtido,const QList<QVector<qreal> > *residuo,const QVector<Cromossomo> *crBest);
        void slot_MW_Desenha();
        void slot_MW_EscreveEquacao();
        void slot_MW_Finalizar();
        void slot_MW_CarDadosExit();
        void slot_MW_SalvarArquivo();
        void slot_MW_saveAsConfig();
        void slot_MW_openConfig();
        void slot_MW_changeStyle(const int &changeStyle);
        void slot_MW_changeVerSaida(const int &idSaida);

    private Q_SLOTS:
        void moved1(const QPoint &);
        void enableZoomMode(bool);
    private:
        //void showInfo(QString text = QString::null);
        virtual void closeEvent(QCloseEvent * event);
        QToolBar *mainToolbar;
        QAction *actionExit,*actionIni,*actionParar,*actionCarDados,*actionConfig,*actionZoom;

        QwtPlotZoomer *MW_zoomer[2],*MW_zoomer1[2];
        QPushButton* pbSalvarDadosTxT,* pbSalvarDados,* pbOpenDados,* pbConcDados;

        QCheckBox *CheckBox1,*CheckBox2;

        QLabel *LN,*LEL,*LESE,*LEM,*LENC,*LEIR,*LDI,*LPI;
        QLineEdit *LEN,*LEEL,*LEESE,*LEEM,*LEENC,*LEDI,*LEPI;
        QComboBox *styleComboBox,
                  *saidaComboBox,
                  *polRacComboBox,
                  *intRealComboBox;
        QString NameConfigSave;
        QDialog *dialogConfig;

        //QDateTime MW_segundos;
        //quint64 MW_iteracoes;
        quint32 MW_SaidaUsada,
                MW_changeStyle;

        QDateTime MW_tempo;
        QVector<qreal> MW_bestFeito;
        QList<QVector<qreal> > MW_resObtido,MW_residuo,MW_best;
        QVector<qreal> MW_somaEr/*,MW_xbestmedia*/;
        QVector<Cromossomo> MW_crBest;
        ICarregar *cr;
        XmlReaderWriter *xmlRW;
        //QXmlStreamReader xmlReader;
        //QXmlStreamWriter xmlWriter;
        QwtPlotPanner *MW_panner,*MW_panner1;
        QwtPlotMarker *MW_mrk,*MW_mrk1;
        QwtPlotPicker *MW_picker,*MW_picker1;
        QwtPlotCurve /*MW_crv_Best,*MW_crv_Media,*/*MW_crv_R,*MW_crv_E,*MW_crv_C;
        bool connectA (const QObject* sender, const char * slot);
        bool connectAG (const QObject* sender, const char * slot);
        bool volatile isThClose;
        QStringList MW_strList;
        qint64 volatile MW_iteracoes;
        void ini_MW_qwtPlot1();
        void ini_MW_interface();
        void Iniciar();
};

#endif // IMAINWINDOW_H
