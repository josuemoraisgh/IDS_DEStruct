#ifndef ICARREGAR_H
#define ICARREGAR_H

#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDialog>
#include <QStatusBar>
#include <QStringListModel>
#include <QtCore>

#include <QtCore/QObject>

#include "ui_icarregar.h"
#include "ui_dialogMaxMin.h"
#include "ui_dialogDecimacao.h"

class DEStruct;

////////////////////////////////////////////////////////////////////////////
class DICarregar : public QDialog, protected Ui::DialogCar
{
  public:
    DICarregar(QWidget* parent = 0)
      : QDialog(parent)
    {
    }
};
////////////////////////////////////////////////////////////////////////////
class ICarregar : public DICarregar
{
    Q_OBJECT

    public:
        ICarregar( QWidget* parent = 0);
        ~ICarregar();
    signals:
        void signal_UL_Normalizar();
        void signal_UL_Carregar(const QList<quint32> *UL_cVariaveis,const bool &isCarregar);
        void signal_UL_Estado(const quint16 &std);
        void signal_UL_SalvarArquivo();
        void signal_UL_FName(const QString &fileName);

    public slots:
        void slot_UL_Finalizado();
        void slot_UL_Parado();
        void slot_UL_Status(const quint16 &std);

    private slots:       
        void slot_UL_Carregar();
        void slot_UL_Indicar(const QString &fileName);
        void slot_UL_Concatenar();
        void slot_UL_Finalizar();
        void slot_UL_Caminho();
        void slot_UL_ModelClicked();
        void slot_UL_Tam();
        void slot_UL_ChangeCombo(int);
        //void slot_UL_ChangeComboDeci(int);
        void slot_UL_ChangeFim(int);
        //void slot_UL_ChangeFimDeci(int);

    private:
        qint32 UL_IndexVar;
        void ativar(const bool &atv);
        QList<qint32> UL_cVariaveis;
        QString UL_fileName;
        QStringList UL_CabecalhoList;
        QStringListModel *UL_slm;
        QStatusBar *LVStBar1;

        QGroupBox *dmmgb;
        QDialog   *dialogMaxMin;
                  //*dialogDecimacao;
        QComboBox *dmmComboBox,
                  *dmmComboBox1;
        QLineEdit *dmmLineEditMax;
        QLineEdit *dmmLineEditMin;
        QLineEdit *dmmLineEditDECI;

        QLabel *dmmTal10;
        QLabel *dmmTal20;
};
#endif // ICARREGAR_H
