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
#include <QtScript/QScriptClass>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptString>

#include "ui_icarregar.h"
#include "agstruct.h"

#ifndef CARREGAR
#define CARREGAR
struct Carregar{
    QString str;
    QVector<QVector<qreal> > valores; //É um vetor de pontos de operação
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
            for(quint16 j=0;j < UL_variaveis.valores.at(i).size();j++)
            {
                UL_variaveis.valores[i][j] =
                        (UL_variaveis.valores.at(i).at(j)-UL_variaveis.Vmenor.at(j))/
                        (UL_variaveis.Vmaior.at(j)-UL_variaveis.Vmenor.at(j));
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        static void UL_filtraVariaveis (QVector<QVector<qreal> > &valores,const quint32 &i)
        {
            if(i <= UL_variaveis.valores.size())
            {
                valores.append(UL_variaveis.valores.at(i-1));
                if(i == UL_variaveis.valores.size()) valores.append(UL_variaveis.valores.at(i));
            }
        }
        ////////////////////////////////////////////////////////////////////////////
        static bool UL_compVariaveis(const quint32 &i)
        {
            if(i==0) return(false);
            else
            {
                quint16 isIgual=0,
                        j=0;
                const QVector<qreal> ponto1 = UL_variaveis.valores.at(i-1);
                const QVector<qreal> ponto2 = UL_variaveis.valores.at(i);
                for(j=0;j < ponto1.size();j++)
                    if((ponto1.at(j) + 0.05) > ponto2.at(j)) isIgual++;
                return(isIgual < ponto1.size());
            }
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
