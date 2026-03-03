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
#include "xtipodados.h"

#ifndef CARREGAR
#define CARREGAR
struct Carregar{
    QString str;
    QVector<QVector<qreal> > valores; //� um vetor de pontos de opera��o
};
#endif //Fim CARREGAR

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
        static PSim UL_variaveis;
        static quint32 UL_count,UL_item,UL_pos,UL_linha;
        static QList<quint16> UL_cVariaveis;
        ICarregar( QWidget* parent = 0);
        ~ICarregar();
        ////////////////////////////////////////////////////////////////////////////
        static PSim value_UL_variaveis()
        {
           return UL_variaveis;
        }
        ////////////////////////////////////////////////////////////////////////////
        static quint32 UL_ipontoVariaveis(const QVector<qreal> &)
        {
            return(UL_count++);
        }
        ////////////////////////////////////////////////////////////////////////////
        static void UL_caregaFilterVariaveis(Carregar &vl,const QChar &ch)
        {
            quint32 j;
            if(UL_item > UL_pos)
            {
                for(j=0;j < UL_cVariaveis.size();j++)
                    if(UL_cVariaveis.at(j) == UL_pos) return;
                if(j < UL_cVariaveis.size())
                {
                    vl.valores.last().replace(j,vl.str.toDouble());
                    if((UL_variaveis.Vmaior.at(j) < vl.valores.last().at(j))||(UL_variaveis.Vmaior.at(j) == UL_variaveis.Vmenor.at(j))) UL_variaveis.Vmaior.replace(j,vl.valores.last().at(j));
                    if((UL_variaveis.Vmenor.at(j) > vl.valores.last().at(j))||(UL_variaveis.Vmaior.at(j) == UL_variaveis.Vmenor.at(j))) UL_variaveis.Vmenor.replace(j,vl.valores.last().at(j));
                }
                UL_pos++;
                UL_item = UL_pos;
                vl.str = QString(ch);
            }
            else vl.str += QString(ch);
            if(vl.valores.empty()||(UL_linha > vl.valores.size()))
            {
                QVector<qreal> valores(UL_cVariaveis.size(),0);
                vl.valores.append(valores);
                UL_linha = (vl.valores.size());
                UL_pos=0;
                UL_item = 0;
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        static bool UL_caregaVariaveis(const QChar &ch)
        {
            if(ch.category() == QChar::Separator_Line) {UL_linha++;UL_item++;return false;}
            else if(ch.isSpace()) {UL_item++;return false;}
            else return true;
        }
        ////////////////////////////////////////////////////////////////////////////
        static void UL_normalizaVariaveis(const quint32 &i)
        {
            if (i >= (quint32)UL_variaveis.valores.numColunas())
                return;

            const qint32 numRows = UL_variaveis.valores.numLinhas();
            for (qint32 j = 0; j < numRows; j++) {
                if (j >= UL_variaveis.Vmaior.size() || j >= UL_variaveis.Vmenor.size())
                    continue;
                const qreal den = UL_variaveis.Vmaior.at(j) - UL_variaveis.Vmenor.at(j);
                if (qFuzzyIsNull(den))
                    continue;
                UL_variaveis.valores(j, i) =
                        (UL_variaveis.valores.at(j, i) - UL_variaveis.Vmenor.at(j)) / den;
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        static void UL_filtraVariaveis (QVector<QVector<qreal> > &valores,const quint32 &i)
        {
            const qint32 numCols = UL_variaveis.valores.numColunas();
            const qint32 numRows = UL_variaveis.valores.numLinhas();
            if (i == 0 || i > (quint32)numCols)
                return;

            QVector<qreal> ponto(numRows);
            for (qint32 j = 0; j < numRows; j++)
                ponto[j] = UL_variaveis.valores.at(j, i - 1);
            valores.append(ponto);

            if (i == (quint32)numCols && numCols > 0) {
                QVector<qreal> ultimo(numRows);
                for (qint32 j = 0; j < numRows; j++)
                    ultimo[j] = UL_variaveis.valores.at(j, numCols - 1);
                valores.append(ultimo);
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        static bool UL_compVariaveis(const quint32 &i)
        {
            const qint32 numCols = UL_variaveis.valores.numColunas();
            const qint32 numRows = UL_variaveis.valores.numLinhas();
            if (i == 0 || i >= (quint32)numCols)
                return false;

            quint16 isIgual = 0;
            for (qint32 j = 0; j < numRows; j++) {
                const qreal ponto1 = UL_variaveis.valores.at(j, i - 1);
                const qreal ponto2 = UL_variaveis.valores.at(j, i);
                if ((ponto1 + 0.05) > ponto2)
                    isIgual++;
            }
            return (isIgual < numRows);
        }
        ////////////////////////////////////////////////////////////////////////////
    private slots:
        void slot_UL_Carregar();
        void slot_UL_Caminho();
        void slot_UL_Indicar(QString elementos);

    private:
        QString UL_fileName;
        QStringList UL_CabecalhoList;
        QStringListModel *UL_slm;
        QStatusBar *LVStBar1;

};
#endif // ICARREGAR_H
