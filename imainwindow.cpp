#include "imainwindow.h"
#include "icarregar.h"
#include "xbelreader.h"
#include "xmlreaderwriter.h"
#include "xtipodados.h"
#include "destruct.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QApplication>
#include <QScreen>
#include <QApplication>
#include <QFileDialog>
#include <QCoreApplication>
#include <QDateTime>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <QMetaType>
//#include <QtConcurrentMap>
//#include <QtConcurrentRun>
#include <limits>
#include <algorithm>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

void crCopy(Cromossomo &cr1,const Cromossomo &cr2);

using namespace std;
Q_DECLARE_METATYPE(QVector<qreal> )
Q_DECLARE_METATYPE(QList<QVector<qreal> > )
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline void reCopy(QVector<qreal > &cr1,const QVector<qreal > &cr2)
{
    qint32 id;
    const qint32 count1=cr1.size(),count2=cr2.size();
    if(count1<count2) cr1 += QVector<qreal >(count2-count1);
    else if(count1>count2) cr1.remove(count2,count1-count2);
    for(id=0;id<count2;id++) cr1[id]=cr2.at(id);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline void reCopy(QList<QVector<qreal > > &cr1,const QList<QVector<qreal > > &cr2)
{
    qint32 id;
    const qint32 count1=cr1.size(),count2=cr2.size();
    if(count1<count2) cr1 += QVector<QVector<qreal > >(count2-count1).toList();
    else for(id=count2;id<count1;id++) cr1.removeAt(id);
    for(id=0;id<count2;id++) reCopy(cr1[id],cr2.at(id));
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline bool CmpMaiorCrApt(const Cromossomo &cr1, const Cromossomo &cr2)
{
    return(cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer(int xAxis, int yAxis, QWidget *canvas):
        QwtPlotZoomer(xAxis, yAxis, (QwtPlotCanvas *) canvas)
    {
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);

        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size

        setMousePattern(QwtEventPattern::MouseSelect2,
            Qt::RightButton, Qt::ControlModifier);
        setMousePattern(QwtEventPattern::MouseSelect3,
            Qt::RightButton);
    }
};
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Fun��o  ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//qreal expoToReal1(const Termo *var)
//{
//    qreal result = ((qreal) var->tTermo1.fracao)/MASKFRA;//Pega parte fracionaria
//    result = var->tTermo1.inteira<0?((qreal) var->tTermo1.inteira)-result:((qreal) var->tTermo1.inteira)+result;
//    return(result);
//}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Classe  ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
ICalc::ICalc( QWidget* parent): DummyBase(parent)
{
  //setupWorkaround(this);
  setupUi(this);
  //////////////////////////////////////////////////////////////////////////////
  cr = new ICarregar(this);
  ags = new QVector<DEStruct *>();
  xmlRW = new XmlReaderWriter(&DEStruct::DES_Adj);
  MW_SaidaUsada = 0;
  MW_changeStyle = 0;
  //////////////////////////////////////////////////////////////////////////////
  //Inicializando as threads
  //MW_tp          = QTime::currentTime();
  //MW_iteracoes   = 0;
  ///////////////////////////////////////////////////////////////////////////
  ini_MW_interface();
  //ini_MW_qwtPlot();
  ini_MW_qwtPlot1();
  //////////////////////////////////////////////////////////////////////////////
  //Conex�es 
  connect(CheckBox1, SIGNAL(stateChanged(int)),this, SLOT(slot_MW_changeCheckBox1(int)));
  connect(CheckBox2, SIGNAL(stateChanged(int)),this, SLOT(slot_MW_changeCheckBox2(int)));
  connect(styleComboBox, SIGNAL(activated (int)),this, SLOT(slot_MW_changeStyle(int)));
  connect(saidaComboBox, SIGNAL(activated (int)),this, SLOT(slot_MW_changeVerSaida(int)));
  connect( actionIni, SIGNAL( triggered() ), this, SLOT( slot_MW_IniciarFinalizar() ) );
  connect( actionParar, SIGNAL( triggered() ), this, SLOT( slot_MW_PararContinuar() ) );
  connect( actionCarDados, SIGNAL( triggered() ), this, SLOT( slot_MW_CarDados() ) );
  connect( cr, SIGNAL( finished ( int )), this, SLOT( slot_MW_CarDadosExit() ) );  
  connect( cr,  SIGNAL( signal_UL_SalvarArquivo() ), this, SLOT(slot_MW_SalvarArquivo()),Qt::DirectConnection);
  connect( cr,  SIGNAL( signal_UL_FName(const QString &)), this, SLOT( slot_MW_FName(const QString &)),Qt::DirectConnection);
  connect( actionConfig, SIGNAL(triggered()), dialogConfig, SLOT(open()));
  connect( pbSalvarDadosTxT,  SIGNAL( clicked() ), this, SLOT(slot_MW_SalvarArquivo()),Qt::DirectConnection);
  connect( pbSalvarDados,  SIGNAL( clicked() ), this, SLOT(slot_MW_saveAsConfig()),Qt::DirectConnection);
  connect( pbOpenDados,  SIGNAL( clicked() ), this, SLOT(slot_MW_openConfig()),Qt::DirectConnection);
  connect( pbConcDados,  SIGNAL( clicked() ), this, SLOT(slot_MW_SalvarArquivo()),Qt::DirectConnection);
  connect( actionExit, SIGNAL(triggered()), this, SLOT(close())); 

  for(qint32 i=0;i < QThread::idealThreadCount();i++)
  {      
    ags->append(new DEStruct());
    connect( ags->at(i), SIGNAL(signal_DES_SetStatus(const volatile qint64 &,const QVector<qreal> *,const QList<QVector<qreal> > *,const QList<QVector<qreal> > *,const QVector<Cromossomo> *)), this, SLOT( slot_MW_SetStatus(const volatile qint64 &,const QVector<qreal> *,const QList<QVector<qreal> > *,const QList<QVector<qreal> > *,const QVector<Cromossomo> *)),Qt::DirectConnection);
    connect( ags->at(i), SIGNAL(signal_DES_closed()), this, SLOT( slot_MW_closed()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Parado()), cr, SLOT( slot_UL_Parado()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Parado()), this, SLOT( slot_MW_Parado()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Finalizado()), cr, SLOT( slot_UL_Finalizado()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Finalizado()), this, SLOT( slot_MW_Finalizado()),Qt::QueuedConnection);
    connect( this, SIGNAL(signal_MW_EquacaoEscrita()), ags->at(i), SLOT( slot_DES_EquacaoEscrita()),Qt::QueuedConnection);
    connect( this, SIGNAL(signal_MW_StatusSetado()), ags->at(i), SLOT( slot_DES_StatusSetado()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Desenha()), this, SLOT(slot_MW_Desenha()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_EscreveEquacao()), this, SLOT(slot_MW_EscreveEquacao()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Finalizar()), this, SLOT( slot_MW_Finalizar()),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Status(const quint16 &)), cr, SLOT( slot_UL_Status(const quint16 &)),Qt::QueuedConnection);
    connect( ags->at(i), SIGNAL(signal_DES_Tam()), cr, SLOT(slot_UL_Tam()),Qt::QueuedConnection);
  }
  isThClose = false;
  connect( this, SIGNAL(signal_MW_Estado(const quint16 &)), ags->at(0), SLOT(slot_DES_Estado(const quint16 &)),Qt::QueuedConnection);
  connect( cr,  SIGNAL(signal_UL_Estado(const quint16 &)), ags->at(0), SLOT(slot_DES_Estado(const quint16 &)),Qt::QueuedConnection);
  //////////////////////////////////////////////////////////////////////////////
  //Desabilitando o Botao Parar
  actionParar->setEnabled(false);
  actionIni->setEnabled(false);
  pbSalvarDadosTxT->setEnabled(false);
  saidaComboBox->setEnabled(false);
  //Centraliza a tela;
  enableZoomMode(false);
  //showInfo();
  QRect r = geometry();
  r.moveCenter(QGuiApplication::primaryScreen()->availableGeometry().center());
  setGeometry(r);
}
////////////////////////////////////////////////////////////////////////////
ICalc::~ICalc()
{

}
////////////////////////////////////////////////////////////////////////////
//Fecha a tela apenas se as threads ja foram fechadas, sen�o manda fecha-las e aguarda
void ICalc::closeEvent(QCloseEvent *event)
{
    bool isOk;
    if(isThClose) event->accept();
    else
    {
        isOk = DEStruct::DES_modo_Oper_TH() == 1;       
        if(isOk) emit signal_MW_Estado(0);//Finaliza as threads
        else QMessageBox::warning(this, tr("Imposs�vel fechar"),tr("Finalize as tarefas para poder fecha-lo!"));
        event->ignore();
    }
}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Slots //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_closed()
{
    isThClose = true;
    QMainWindow::close();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_changeStyle(const int &changeStyle)
{
    MW_changeStyle = changeStyle;
    if(!MW_changeStyle)
    {
        CheckBox1->setDisabled(false);
        CheckBox2->setDisabled(false);
        qwtPlot1->setAxisTitle(QwtPlot::yLeft, tr("Saidas"));
        qwtPlot1->setAxisTitle(QwtPlot::xBottom, tr("Amostras"));
        MW_crv_R->attach(qwtPlot1);
        MW_crv_C->setTitle(tr("Estimado"));
    }
    else
    {
        qwtPlot1->setAxisTitle(QwtPlot::yLeft, tr("Quantidade de Residuos"));
        qwtPlot1->setAxisTitle(QwtPlot::xBottom, tr("Valores dos Residuos"));
        MW_crv_C->setTitle(tr("Distribui��o dos Residuos"));
        MW_crv_R->detach();
        CheckBox1->setDisabled(true);
        CheckBox2->setDisabled(true);
    }
    slot_MW_Desenha();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_changeVerSaida(const int &idSaida)
{
   MW_SaidaUsada=idSaida;
   slot_MW_Desenha();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_FName(const QString &fileName)
{
    DEStruct::DES_fileName = fileName;
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_Parado()
{
    if(this->isActiveWindow())
    {
        actionIni->setEnabled(true);
        LEEM->setEnabled(true);
        LEENC->setEnabled(true);
        LEESE->setEnabled(true);
        polRacComboBox->setEnabled(true);
        intRealComboBox->setEnabled(true);
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_Finalizado()
{
    if(this->isActiveWindow())
    {
        actionIni->setEnabled(true);
        LEEM->setEnabled(true);
        LEENC->setEnabled(true);
        LEESE->setEnabled(true);
        polRacComboBox->setEnabled(true);
        intRealComboBox->setEnabled(true);
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_IniciarFinalizar()
{
    //qint32 k=0;
    if(actionIni->text()==QString("Finalizar"))
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        actionIni->setEnabled(false);
        actionIni->setText(QString("Iniciar"));
        emit signal_MW_Estado(1);//Finaliza as threads
        actionParar->setEnabled(false);
        actionCarDados->setEnabled(true);
        LEN->setEnabled(true);
        LEEL->setEnabled(true);
        LEEM->setEnabled(true);
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //DEStruct::DES_Adj.isCriado=false;
    }
    else
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //MW_xbestmedia.clear();
        //MW_bestFeito.clear();
        //MW_best.clear();
        //MW_media.clear();
        MW_crBest.clear();
        //MW_isSupRes .clear();
        DEStruct::DES_Adj.isCriado=false;
        MW_iteracoes = 0;
        ////////////////////////////////////////////////////////////////////////////
        for(qint32 k=0;k<TAMPIPELINE;k++)
        {
            DEStruct::DES_Adj.vetPop[k].clear();
            DEStruct::DES_crMut[k].clear();
            DEStruct::DES_somaSSE[k].clear();
            DEStruct::DES_index[k]=0;
            DEStruct::DES_idParadaJust[k]=false;
            ////////////////////////////////////////////////////////////////////////////
            //Todas as threads inicializam algumas variaveis.
            //DEStruct::DES_vcalc[k].clear();
            //DEStruct::DES_residuos[k].clear();
            //DEStruct::DES_BufferSR[k].clear();
        }
        ////////////////////////////////////////////////////////////////////////////
        //const qint32 qtSaidas = DEStruct::DES_Adj.Dados.variaveis.qtSaidas;
        //const qint32 tamPop   = LEN->text().toInt();
        ////////////////////////////////////////////////////////////////////////////
        DEStruct::DES_Adj.iteracoes = 0;
        DEStruct::DES_Adj.segundos = QDateTime::fromString("M1d1y0000:00:00","'M'M'd'd'y'yyhh:mm:ss");
        ////////////////////////////////////////////////////////////////////////////
        DEStruct::DES_Adj.Dados.isElitismo = 2;//Come�a do zero
        if(DEStruct::DES_Adj.Dados.qtdadeVarAnte==DEStruct::DES_Adj.Dados.variaveis.valores.numLinhas())
        {
            if(!(QMessageBox::question(this, tr("Limpar pontos"),tr("Deseja manter os pontos do Elitismo?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes))
                DEStruct::DES_Adj.Dados.isElitismo = 1;//Conserva os eleitos anteriormente
        }
        else DEStruct::DES_Adj.Dados.qtdadeVarAnte=DEStruct::DES_Adj.Dados.variaveis.valores.numLinhas();
        ////////////////////////////////////////////////////////////////////////////
        Iniciar();
        slot_MW_PararContinuar();
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::Iniciar()
{
    qint32 i=0,j=0,k=0;
    QString str;
    ////////////////////////////////////////////////////////////////////////////
    const qint32 qtSaidas = DEStruct::DES_Adj.Dados.variaveis.qtSaidas;
    const qint32 tamDados = DEStruct::DES_Adj.Dados.variaveis.valores.numColunas();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Ajusta o tempo
    actionCarDados->setEnabled(false);
    actionParar->setEnabled(true);
    actionIni->setEnabled(false);
    LEN->setEnabled(false);
    LEEL->setEnabled(false);
    textEdit->clear();
    //MW_xbestmedia.clear();
    saidaComboBox->clear();
    ////////////////////////////////////////////////////////////////////////////
    //Escreve os valores de m�ximo e m�nimo das variaveis.
    str.append(QString("Cl:= ")+QString::number(MW_iteracoes)+QString("; Tempo: ")+DEStruct::DES_Adj.segundos.toString("hh:mm:ss.zzz")+QString("ms\n"));
    for(i=0;i<DEStruct::DES_Adj.Dados.variaveis.nome.size();i++)
    {
        if(i<DEStruct::DES_Adj.Dados.variaveis.qtSaidas) str.append(DEStruct::DES_Adj.Dados.variaveis.nome.at(i)+QString(": Max = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i))+QString(", Min = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i))+QString(tr(", Decimação = "))+QString::number(DEStruct::DES_Adj.decimacao.at(i))+QString("; "));
        else str.append(DEStruct::DES_Adj.Dados.variaveis.nome.at(i)+QString(": Max = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i))+QString(", Min = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i))+QString("; "));
    }
    textEdit->append(str);
    ////////////////////////////////////////////////////////////////////////////
    for(k=0;k<TAMPIPELINE;k++)
    {
        DEStruct::DES_crMut[k] += QVector<Cromossomo >(qtSaidas);
        DEStruct::DES_BufferSR[k] = (QVector<QVector<Cromossomo> >(qtSaidas)).toList();
        DEStruct::DES_somaSSE[k].fill(0.0f,qtSaidas);
        DEStruct::DES_index[k]=0;
        DEStruct::DES_idParadaJust[k]=false;
        DEStruct::DES_vcalc[k] = QVector<QVector<qreal> >(qtSaidas).toList();
        DEStruct::DES_residuos[k] = QVector<QVector<qreal> >(qtSaidas).toList();
        DEStruct::DES_Adj.isSR[k] = QVector<QList<bool> >(qtSaidas).toList();
        DEStruct::DES_idChange[k] = QVector<qint32>(qtSaidas);
        DEStruct::DES_Adj.vetElitismo[k] = QVector<QVector<qint32 > >(qtSaidas).toList();
        if(DEStruct::DES_Adj.Pop.isEmpty())
            DEStruct::DES_Adj.Pop = QVector<QVector<Cromossomo > >(qtSaidas).toList();
        for(j=0;j<qtSaidas;j++)
        {
            DEStruct::DES_BufferSR[k][j] = QVector<Cromossomo >(10);
            DEStruct::DES_vcalc[k][j] = QVector<qreal>(tamDados/DEStruct::DES_Adj.decimacao.at(j));
            DEStruct::DES_residuos[k][j] = QVector<qreal>(tamDados/DEStruct::DES_Adj.decimacao.at(j));
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    //Verifica se cada saida esta com a quantiadade correta de cromossomos.    
    for(i=0;i<qtSaidas;i++)
        saidaComboBox->addItem(DEStruct::DES_Adj.Dados.variaveis.nome.at(i));
    MW_bestFeito = QVector<qreal>(qtSaidas);
    MW_best      = QVector<QVector<qreal> >(qtSaidas).toList();
    MW_crBest    = QVector<Cromossomo>(qtSaidas);
    //MW_isSupRes = QVector<QList<bool> >(qtSaidas).toList();   
    ////////////////////////////////////////////////////////////////////////////
    saidaComboBox->setCurrentIndex(0);
    actionParar->setText(QString("Continuar"));
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_PararContinuar()
{
    if(actionParar->text()==QString("Parar"))
    {
        emit signal_MW_Estado(2);//Parar as threads
        //actionParar->setText(QString("Continuar"));
        actionParar->setEnabled(false);
    }
    else
    {
        DEStruct::DES_Adj.iteracoesAnt = DEStruct::DES_Adj.iteracoes;
        const qint32 qtSaidas = DEStruct::DES_Adj.Dados.variaveis.qtSaidas;
        DEStruct::DES_Adj.numeroCiclos = LEENC->text().toInt();
        DEStruct::DES_Adj.jnrr = LEEM->text().toDouble();
        DEStruct::DES_Adj.serr = LEESE->text().toDouble();
        DEStruct::DES_Adj.isTipoExpo = intRealComboBox->currentIndex();
        DEStruct::DES_Adj.isRacional = (polRacComboBox->currentIndex()==0);
        LEEM->setEnabled(false);
        LEENC->setEnabled(false);
        LEESE->setEnabled(false);
        polRacComboBox->setEnabled(false);
        intRealComboBox->setEnabled(false);
        actionParar->setText(QString("Parar"));
        actionIni->setEnabled(false);    
        MW_tempo = QDateTime::currentDateTime();
        DEStruct::DES_Adj.Dados.tamPop = LEN->text().toInt();
        ////////////////////////////////////////////////////////////////////////////
        if((DEStruct::DES_Adj.Dados.iElitismo==0)||(DEStruct::DES_Adj.Dados.iElitismo!=LEEL->text().toInt()))
            DEStruct::DES_Adj.Dados.iElitismo = LEEL->text().toInt() ? LEEL->text().toInt() : 5;//iElitismo
        ////////////////////////////////////////////////////////////////////////////
        //Verifica se cada saida esta com a quantiadade correta de cromossomos.
        if((!DEStruct::DES_Adj.isCriado)||(DEStruct::DES_Adj.Pop.at(0).size()<DEStruct::DES_Adj.Dados.tamPop))
        {
            DEStruct::lock_DES_BufferSR.lockForWrite();
            if(DEStruct::DES_Adj.Dados.isElitismo != 1)
            {
                for(qint32 k=0;k<qtSaidas;k++)
                    DEStruct::DES_Adj.Pop[k] = QVector<Cromossomo >(DEStruct::DES_Adj.Dados.tamPop);
            }
            else
            {
                for(qint32 k=0;k<qtSaidas;k++)
                {
                    std::sort(DEStruct::DES_Adj.Pop[k].begin(),DEStruct::DES_Adj.Pop[k].end(),CmpMaiorCrApt);
                    if(DEStruct::DES_Adj.Pop.at(k).size()<DEStruct::DES_Adj.Dados.tamPop) DEStruct::DES_Adj.Pop[k] += QVector<Cromossomo >(DEStruct::DES_Adj.Dados.tamPop-DEStruct::DES_Adj.Pop.at(k).size());
                }
            }
            DEStruct::lock_DES_BufferSR.unlock();
        }
        ////////////////////////////////////////////////////////////////////////////
        if(actionIni->text()!=QString("Finalizar")) actionIni->setText(QString("Finalizar"));
        emit signal_MW_Estado(3);//Continuar as threads
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_CarDados()
{
    cr->slot_UL_Status(0);
    cr->exec();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_CarDadosExit()
{
    if(!DEStruct::DES_Adj.Dados.variaveis.nome.isEmpty())
    {
        actionIni->setEnabled(true);
        pbSalvarDadosTxT->setEnabled(true);
    }
    else
    {
        actionIni->setEnabled(false);
        pbSalvarDadosTxT->setEnabled(false);
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_SalvarArquivo()
{
    //////////////////////////////////////////////////////////////////////////////////
    //JStrSet jst;
    //////////////////////////////////////////////////////////////////////////////////
    const qreal step = (DEStruct::DES_Adj.Dados.timeFinal - DEStruct::DES_Adj.Dados.timeInicial)/DEStruct::DES_Adj.Dados.variaveis.valores.numColunas();
    const QString fileName = QFileDialog::getSaveFileName(cr,tr("Salvar Dados"), QDir::currentPath() , tr("Arquivo de Dados (*.txt)"));
    qint32 i;
    //JMathVar<qreal> matAux;
    qreal *valor,time=DEStruct::DES_Adj.Dados.timeInicial;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream stream(&file);
        stream.setRealNumberNotation(QTextStream::ScientificNotation);
        stream.setRealNumberPrecision(9);
        stream << "TIME     ";
        for(i=0;i<DEStruct::DES_Adj.Dados.variaveis.nome.size();i++)
        {
            if(i<DEStruct::DES_Adj.Dados.variaveis.qtSaidas) stream << DEStruct::DES_Adj.Dados.variaveis.nome.at(i) << "(" << DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i) << "," << DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i) << "," << DEStruct::DES_Adj.decimacao.at(i) <<  ") ";
            else stream << DEStruct::DES_Adj.Dados.variaveis.nome.at(i) << "(" << DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i) << "," << DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i) << ") ";
            //matAux.replace(DEStruct::DES_Adj.Dados.variaveis.valores,jst.set("(%1,:)=(%1,0:%2:%3)").argInt(i).argInt(DEStruct::DES_Adj.decimacao.at(0)).argInt(DEStruct::DES_Adj.Dados.variaveis.valores.numColunas()));
        }
        stream << "\r\n" << time << "  ";
        const qint32 count = DEStruct::DES_Adj.Dados.variaveis.valores.numLinhas();
        for(valor=DEStruct::DES_Adj.Dados.variaveis.valores.begin(),i=0;valor<DEStruct::DES_Adj.Dados.variaveis.valores.end();valor++,i++)
        {
            if(i==count)
            {
                i=0;
                time  += step;
                stream << "\r\n" << time << "  ";
            }
            stream << *valor << " ";
        }
        stream << "\r\n ";
        file.close();
    }
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_SetStatus(const volatile qint64 &iteracoes,const QVector<qreal> *somaEr,const QList<QVector<qreal> > *resObtido,const QList<QVector<qreal> > *residuo,const QVector<Cromossomo> *crBest)
{    
    DEStruct::LerDados.lockForWrite();
    MW_iteracoes = iteracoes;
    reCopy(MW_resObtido,*resObtido);
    reCopy(MW_residuo,*residuo);
    reCopy(MW_somaEr,*somaEr);
    std::copy(crBest->begin(),crBest->end(),MW_crBest.begin());
    emit signal_MW_StatusSetado();
    DEStruct::LerDados.unlock();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_EscreveEquacao()
{
    qint64 iteracoes;
    bool isFeito=false;
    QVector<qreal> somaEr;
    QVector<Cromossomo> crBest;
    QString str,strErr,strNum,strDen,strErrNum,strErrDen,strRegress;
    qint32 countRegress=0,i=0,index=0,idVariavel=0,idAtraso=0,numColuna=0,idSaida=0;
    qreal idCoefic=0.,aux=0.,varAux=0.,jn=0.,jnM=0.,rsme=0.,idExpo=0.;

    DEStruct::LerDados.lockForRead();
    iteracoes = MW_iteracoes;
    reCopy(somaEr,MW_somaEr);
    crBest.resize(MW_crBest.size());
    std::copy(MW_crBest.begin(),MW_crBest.end(),crBest.begin());
    DEStruct::LerDados.unlock();

    str.append(QString("Cl:= ")+QString::number(iteracoes)+QString("; Tempo: ")+QString::number(MW_tempo.secsTo(QDateTime::currentDateTime()))+"s \n");
    //+dt.toString("hh:mm:ss.zzz")+QString("ms\n"));
    for(i=0;i<DEStruct::DES_Adj.Dados.variaveis.nome.size();i++)
    {
        if(i<DEStruct::DES_Adj.Dados.variaveis.qtSaidas) str.append(DEStruct::DES_Adj.Dados.variaveis.nome.at(i)+QString(": Max = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i))+QString(", Min = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i))+QString(tr(", Decima��o = "))+QString::number(DEStruct::DES_Adj.decimacao.at(i))+QString("; "));
        else str.append(DEStruct::DES_Adj.Dados.variaveis.nome.at(i)+QString(": Max = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(i))+QString(", Min = ")+QString::number(DEStruct::DES_Adj.Dados.variaveis.Vmenor.at(i))+QString("; "));
    }
    for(idSaida=0;idSaida<DEStruct::DES_Adj.Dados.variaveis.qtSaidas;idSaida++)
    {
        numColuna  = DEStruct::DES_Adj.Dados.variaveis.valores.numColunas()-crBest.at(idSaida).maiorAtraso;
        strNum = "";strDen = "";strErrNum = "";strErrDen = "";strRegress = "";
        for(countRegress=0;countRegress<crBest.at(idSaida).regress.size();countRegress++) //Varre todos os termos para aquele cromossomo
        {
            idCoefic = crBest.at(idSaida).vlrsCoefic.at(countRegress);
            if(crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.reg)
            {
                for(i=0;i<crBest.at(idSaida).regress.at(countRegress).size();i++)
                {
                    idVariavel = crBest.at(idSaida).regress.at(countRegress).at(i).vTermo.tTermo1.var;  //Obtem a variavel
                    if(!DEStruct::DES_Adj.isTipoExpo) idExpo = crBest.at(idSaida).regress.at(countRegress).at(i).expoente; //Obtem o expoente deste termo (Grau da N�o-Linearidade)
                    else
                    {
                        idExpo = (qint32) crBest.at(idSaida).regress.at(countRegress).at(i).expoente; //Obtem o expoente deste termo (Grau da N�o-Linearidade)
                        idExpo +=(crBest.at(idSaida).regress.at(countRegress).at(i).expoente-idExpo)>=0.5?1:(crBest.at(idSaida).regress.at(countRegress).at(i).expoente-idExpo)<=-0.5?-1:0;
                        if(DEStruct::DES_Adj.isTipoExpo==2) idExpo = fabs(idExpo);
                    }                     
                    if(idExpo)
                    {
                        idAtraso   = crBest.at(idSaida).regress.at(countRegress).at(i).vTermo.tTermo1.atraso;
                        ///////////////////////////////////////////////////////////////////////////////////
                        //Monta a string vers�o c++
                        /*strRegress += (idCoefic)&&(idExpo)?
                                                 (idVariavel>=1?(idExpo>1?QString("pow(" + DEStruct::DES_Adj.Dados.variaveis.nome.at(idVariavel-1) + "[%1],%2)*").arg(idAtraso).arg(idExpo):
                                                                (idExpo==1?QString(DEStruct::DES_Adj.Dados.variaveis.nome.at(idVariavel-1) + "[%1]*").arg(idAtraso):""))
                                                               :(idExpo>1?QString("pow(E[%1],%2)*").arg(idAtraso).arg(idExpo):
                                                                (idExpo==1?QString("E[%1]*").arg(idAtraso):""))
                                                  )
                                                 :"";*/
                        ///////////////////////////////////////////////////////////////////////////////////
                        //Monta a string vers�o Scilab/MatLab
                        strRegress += (idCoefic)&&(idExpo)?
                                    (idVariavel>=1?((idExpo!=1.)&&(idExpo!=0.)?QString("(" + DEStruct::DES_Adj.Dados.variaveis.nome.at(idVariavel-1) + "(k-%1)^%2)*").arg(idAtraso).arg(idExpo):
                                                   ((idExpo==1.)?QString(DEStruct::DES_Adj.Dados.variaveis.nome.at(idVariavel-1) + "(k-%1)*").arg(idAtraso):""))
                                                  :((idExpo!=1.)&&(idExpo!=0.)?QString("(E(k-%1)^%2)*").arg(idAtraso).arg(idExpo):
                                                   (idExpo==1.?QString("E(k-%1)*").arg(idAtraso):""))
                                                  )
                                                 :"";

                    }
                }
                strRegress.remove(strRegress.size()-1,1);
                if(idCoefic)
                {
                    if(crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.nd)
                    {
                        strNum += (QString(idCoefic>0.0f?" +%1*":" %1*").arg(idCoefic))+strRegress;
                        strErrNum += " " + QString::number(crBest.at(idSaida).err.at(countRegress)) + "; ";
                    }
                    else
                    {
                        strDen += (QString(idCoefic>0.0f?" +%1*":" %1*").arg(idCoefic))+strRegress;
                        strErrDen += " " + QString::number(crBest.at(idSaida).err.at(countRegress)) + "; ";
                    }
                }
                strRegress = "";
            }
            else
            {
                strRegress = "";
                if(idCoefic)
                {
                    if(crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.nd)
                    {
                        strNum = QString("%1").arg(idCoefic) + strNum;
                        strErrNum = QString::number(crBest.at(idSaida).err.at(countRegress))+ "; " + strErrNum;
                    }
                    else
                    {
                        isFeito = true;
                        strDen = QString("%1").arg(idCoefic+1) + strDen;
                        strErrDen = QString::number(crBest.at(idSaida).err.at(countRegress))+ "; " + strErrDen;
                    }
                }
            }
        }
        /////////////////////////////////////////////////////////////////
        const qint32 tamErro = crBest.at(idSaida).vlrsCoefic.size()-crBest.at(idSaida).regress.size();
        for(countRegress=0;countRegress<tamErro;countRegress++)
        {
            idCoefic = crBest.at(idSaida).vlrsCoefic.at(crBest.at(idSaida).regress.size()+countRegress);
            strErr += (QString(idCoefic>0.0f?" +%1*E(k-%2)":" %1*E(k-%2)").arg(idCoefic).arg(countRegress+1));
        }
        /////////////////////////////////////////////////////////////////
        if(strNum == "") strNum = "1";
        if(strDen == "") strDen = "1";
        else if(!isFeito)
        {
            strDen = QString("+1") + strDen;
            strErrDen = QString("-1;") + strErrDen;
        }
        /////////////////////////////////////////////////////////////////
        for(index=crBest.at(idSaida).maiorAtraso+2;index<numColuna+crBest.at(idSaida).maiorAtraso;index++)
        {
            aux = DEStruct::DES_Adj.Dados.variaveis.valores.at(idSaida,index)-DEStruct::DES_Adj.Dados.variaveis.valores.at(idSaida,index-2);
            varAux += aux*aux;
        }
        jn  = crBest.at(idSaida).erro;//DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(idSaida);
        jnM = somaEr.at(idSaida)/(DEStruct::DES_Adj.Dados.tamPop);//*DEStruct::DES_Adj.Dados.variaveis.Vmaior.at(idSaida));
        rsme = (sqrt(jn))/(sqrt(varAux/(numColuna-2)));
        str.append(QString("\nRMSE(2):= %1; Jn(Menor):= %2; Jn(Md):= %3").arg(rsme).arg(jn).arg(jnM));
        if(strNum.size()) str.append(QString("\n%1(k) = ((%1_Num+%1_Err)/(%1_Den));\n%1_Num = "+strNum+";\n%1_Err = "+strErr+";\n%1_Den = "+strDen+";\nERR_Num:=("+strErrNum+");\nERR_Den:=("+strErrDen+");").arg(DEStruct::DES_Adj.Dados.variaveis.nome.at(idSaida)));
    }
    str.append(QString("\n"));
    CheckBox1->setChecked(true);
    CheckBox2->setChecked(true);
    textEdit->setText(str);
    emit signal_MW_EquacaoEscrita();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_Desenha()
{
    QList<QVector<qreal> > resObtido;
    QVector<Cromossomo> crBest;
    QList<QVector<qreal> > residuos;
    qreal auxReal1,auxReal2;
    QVector<qreal> medida,time,residuo;
    qint32 j,i,numColuna;

    DEStruct::LerDados.lockForRead();
    reCopy(resObtido,MW_resObtido);
    reCopy(residuos,MW_residuo);
    crBest.resize(MW_crBest.size());
    std::copy(MW_crBest.begin(),MW_crBest.end(),crBest.begin());
    DEStruct::LerDados.unlock();

    numColuna=resObtido.at(MW_SaidaUsada).size();
    time.clear();
    if(!MW_changeStyle)
    {
        for(i=crBest.at(MW_SaidaUsada).maiorAtraso;i<numColuna;i++)
        {
            time.append(i);
            medida.append(DEStruct::DES_Adj.Dados.variaveis.valores.at(MW_SaidaUsada,i*DEStruct::DES_Adj.decimacao.at(MW_SaidaUsada)));
        }
        MW_crv_C->setSamples(time,resObtido.at(MW_SaidaUsada));
        MW_crv_R->setSamples(time,medida);
    }
    else
    {
        residuo+=residuos[MW_SaidaUsada].mid(residuos[MW_SaidaUsada].size()/2,residuos[MW_SaidaUsada].size()-(residuos[MW_SaidaUsada].size()/2));
        std::sort(residuo.begin(),residuo.end(),std::less<qreal>());
        auxReal1 = residuo.last()>(-residuo.first())?residuo.last()/25:(-residuo.first())/25;
        auxReal2 = (residuo.last()>(-residuo.first())?(-residuo.last()):residuo.first())-(auxReal1/2);
        time.append(auxReal2);
        time.append(auxReal2);
        for(i=0,auxReal2+=auxReal1;i<50;i++,auxReal2+=auxReal1) {time.append(auxReal2);time.append(auxReal2);}
        medida = QVector<qreal>(time.size(),0.0f);
        i=0;
        auxReal2=(residuo.last()>(-residuo.first())?(-residuo.last()):residuo.first())-(auxReal1/2);
        for(j=0;j<residuo.size();j++)
        {
            for(;auxReal2<=residuo.at(j);auxReal2+=auxReal1,i++);
            if((i*2+2)<medida.size())
            {
                medida[i*2+1]++;
                medida[i*2+2]++;
            }
        }
        MW_crv_C->setSamples(time,medida);
    }
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_Finalizar()
{
    saidaComboBox->setEnabled(true);
    slot_MW_EscreveEquacao();
    slot_MW_Desenha();
    actionParar->setEnabled(true);
    actionParar->setText(QString("Continuar"));
    actionIni->setEnabled(true);

}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_changeCheckBox1(int st)
{
    if(!st) {MW_crv_R->setVisible(false);CheckBox2->setChecked(true);}
    else MW_crv_R->setVisible(true);
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_changeCheckBox2(int st)
{
    if(!st) {MW_crv_C->setVisible(false);CheckBox1->setChecked(true);}
    else MW_crv_C->setVisible(true);
    qwtPlot1->setAutoReplot(true);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->replot();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::ini_MW_interface()
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
    //////////////////////////////////////////////////////////////////////////////
    actionZoom->setText("Zoom");
    //actionZoom->setIcon(QIcon(zoom_xpm));
    actionZoom->setCheckable(true);
    //actionZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    connect(actionZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));
    //////////////////////////////////////////////////////////////////////////////
    LN = new QLabel(" N:=",this);
    QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy3.setHeightForWidth(LN->sizePolicy().hasHeightForWidth());
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    QFont font;
    font.setFamily(QString::fromUtf8("Arial"));
    LN->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LN);
    //////////////////////////////////////////////////////////////////////////////
    LEN = new QLineEdit("300",this);
    LEN->setObjectName(QString::fromUtf8("LEN"));
    LEN->setSizePolicy(sizePolicy3);
    LEN->setMaximumSize(QSize(40, 16777215));
    LEN->setMaxLength(50);
    LEN->setCursorPosition(2);
    LEN->setValidator(new QIntValidator (0,10000,this));
    mainToolbar->addWidget(LEN);
    //////////////////////////////////////////////////////////////////////////////
    LEL = new QLabel(" E:=",this);
    LEL->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEL);
    //////////////////////////////////////////////////////////////////////////////
    LEEL = new QLineEdit("5",this);
    LEEL->setObjectName(QString::fromUtf8("LEEL"));
    LEEL->setSizePolicy(sizePolicy3);
    LEEL->setMaximumSize(QSize(20, 16777215));
    LEEL->setContextMenuPolicy(Qt::NoContextMenu);
    LEEL->setValidator(new QIntValidator (0,10000,this));
    mainToolbar->addWidget(LEEL);
    //////////////////////////////////////////////////////////////////////////////
    LESE = new QLabel(" SERR:=",this);
    LESE->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LESE);
    //////////////////////////////////////////////////////////////////////////////
    LEESE = new QLineEdit("0.98",this);
    LEESE->setObjectName(QString::fromUtf8("LEEL"));
    LEESE->setSizePolicy(sizePolicy3);
    LEESE->setMaximumSize(QSize(40, 16777215));
    LEESE->setContextMenuPolicy(Qt::NoContextMenu);
    LEESE->setValidator(new QDoubleValidator (0.9,0.999,3,this));
    mainToolbar->addWidget(LEESE);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    polRacComboBox = new QComboBox;
    polRacComboBox->addItem("Racional");
    polRacComboBox->addItem("Polinomial");
    polRacComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(polRacComboBox);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    LEIR = new QLabel(" Expoente do tipo:",this);
    LEIR->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEIR);
    intRealComboBox = new QComboBox;
    intRealComboBox->addItem("Real");
    intRealComboBox->addItem("Inteiro");
    intRealComboBox->addItem("Natural");
    intRealComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(intRealComboBox);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    LEM = new QLabel(" RJn:= ",this);
    LEM->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LEM);
    //////////////////////////////////////////////////////////////////////////////
    LEEM = new QLineEdit("0.001",this);
    LEEM->setObjectName(QString::fromUtf8("LEEM"));
    LEEM->setSizePolicy(sizePolicy3);
    LEEM->setMaximumSize(QSize(40, 16777215));
    LEEM->setContextMenuPolicy(Qt::NoContextMenu);
    LEEM->setValidator(new QDoubleValidator (0.0,1000000.0,6,this));
    mainToolbar->addWidget(LEEM);
    //////////////////////////////////////////////////////////////////////////////
    LENC = new QLabel(" NCy:= ",this);
    LENC->setSizePolicy(sizePolicy3);
    mainToolbar->addWidget(LENC);
    //////////////////////////////////////////////////////////////////////////////
    LEENC = new QLineEdit("100",this);
    LEENC->setObjectName(QString::fromUtf8("LEENC"));
    LEENC->setSizePolicy(sizePolicy3);
    LEENC->setMaximumSize(QSize(40, 16777215));
    LEENC->setContextMenuPolicy(Qt::NoContextMenu);
    LEENC->setValidator(new QIntValidator (1,1000000,this));
    mainToolbar->addWidget(LEENC);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    styleComboBox = new QComboBox;
    styleComboBox->addItem("Medido/Estimado");
    styleComboBox->addItem("Dist. Residuos");
    styleComboBox->setCurrentIndex(0);
    mainToolbar->addWidget(styleComboBox);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    CheckBox1 = new QCheckBox;
    CheckBox1->setText("Medido");
    CheckBox1->setChecked(true);
    mainToolbar->addWidget(CheckBox1);
    CheckBox2 = new QCheckBox;
    CheckBox2->setText("Estimado");
    CheckBox2->setChecked(true);
    mainToolbar->addWidget(CheckBox2);
    //////////////////////////////////////////////////////////////////////////////
    mainToolbar->addSeparator();
    mainToolbar->addWidget(new QLabel(" Sa�da Vis�vel:= ",this));
    saidaComboBox = new QComboBox;
    mainToolbar->addWidget(saidaComboBox);
    //////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////
void ICalc::enableZoomMode(bool on)
{
    //MW_panner->setEnabled(on);
    MW_panner1->setEnabled(on);

    //MW_zoomer[0]->setEnabled(on);
    //MW_zoomer[0]->zoom(0);
    MW_zoomer1[0]->setEnabled(on);
    MW_zoomer1[0]->zoom(0);

    //MW_zoomer[1]->setEnabled(on);
    //MW_zoomer[1]->zoom(0);
    MW_zoomer1[1]->setEnabled(on);
    MW_zoomer1[1]->zoom(0);

    //MW_picker->setEnabled(!on);
    MW_picker1->setEnabled(!on);

    //qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    //qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    qwtPlot1->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot1->setAxisAutoScale(QwtPlot::xBottom);
    //showInfo();
}
////////////////////////////////////////////////////////////////////////////
void ICalc::ini_MW_qwtPlot1()
{
    //qwtPlot
    qwtPlot1->setAutoReplot(false);
    qwtPlot1->setCanvasBackground(QColor(Qt::white));

    MW_panner1 = new QwtPlotPanner(qwtPlot1->canvas());
    MW_panner1->setMouseButton(Qt::MidButton);

    MW_zoomer1[0] = new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft,
        qwtPlot1->canvas());
    MW_zoomer1[0]->setRubberBand(QwtPicker::RectRubberBand);
    MW_zoomer1[0]->setRubberBandPen(QColor(Qt::green));
    MW_zoomer1[0]->setTrackerMode(QwtPicker::ActiveOnly);
    MW_zoomer1[0]->setTrackerPen(QColor(Qt::black));

    MW_zoomer1[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight,
         qwtPlot1->canvas());

    MW_picker1 = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
        qwtPlot1->canvas());

    MW_picker1->setRubberBandPen(QColor(Qt::darkBlue));
    MW_picker1->setRubberBand(QwtPicker::CrossRubberBand);
    MW_picker1->setTrackerPen(QColor(Qt::black));
    connect(MW_picker1, SIGNAL(moved(const QPoint &)),SLOT(moved1(const QPoint &)));
    //connect(MW_picker1, SIGNAL(selected(const QPolygon &)),SLOT(selected(const QPolygon &)));

    // legend
    QwtLegend *legend1 = new QwtLegend;
    legend1->setFrameStyle(QFrame::Box|QFrame::Sunken);
    qwtPlot1->insertLegend(legend1, QwtPlot::TopLegend);
    // grid
    QwtPlotGrid *grid1 = new QwtPlotGrid;
    grid1->enableXMin(true);
    grid1->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid1->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid1->attach(qwtPlot1);
    // Axes2
    qwtPlot1->enableAxis(QwtPlot::yLeft);
    qwtPlot1->setAxisTitle(QwtPlot::xBottom, "Amostras");
    qwtPlot1->setAxisTitle(QwtPlot::yLeft, "Saidas");
    qwtPlot1->setAxisMaxMajor(QwtPlot::xBottom, 6);
    qwtPlot1->setAxisMaxMinor(QwtPlot::xBottom, 10);
    // Real QWP2
    MW_crv_R = new QwtPlotCurve("Medido");
    MW_crv_R->setRenderHint(QwtPlotItem::RenderAntialiased);
    MW_crv_R->setPen(QPen(Qt::darkBlue,1));
    MW_crv_R->setYAxis(QwtPlot::yLeft);
    MW_crv_R->setStyle(QwtPlotCurve::Lines);
    MW_crv_R->attach(qwtPlot1);
    // Calculado QWP2
    MW_crv_C = new QwtPlotCurve("Estimado");
    MW_crv_C->setRenderHint(QwtPlotItem::RenderAntialiased);
    MW_crv_C->setPen(QPen(Qt::red,1));
    MW_crv_C->setYAxis(QwtPlot::yLeft);
    MW_crv_C->setStyle(QwtPlotCurve::Lines);
    MW_crv_C->attach(qwtPlot1);
    // marker
    MW_mrk1 = new QwtPlotMarker();
    MW_mrk1->setValue(0.0, 0.0);
    MW_mrk1->setLineStyle(QwtPlotMarker::VLine);
    MW_mrk1->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    MW_mrk1->setLinePen(QPen(Qt::green, 0, Qt::DashLine));
    MW_mrk1->attach(qwtPlot1);
}
////////////////////////////////////////////////////////////////////////////
void ICalc::moved1(const QPoint &pos)
{
    qwtPlot1->invTransform(QwtPlot::xBottom, pos.x());
    qwtPlot1->invTransform(QwtPlot::yLeft, pos.y());
    qwtPlot1->invTransform(QwtPlot::yRight, pos.y());
}
////////////////////////////////////////////////////////////////////////////
//Carrega um configura��o
void ICalc::slot_MW_openConfig()
{
    NameConfigSave.clear();
    NameConfigSave = QFileDialog::getOpenFileName(this, tr("Open Bookmark File"),QDir::currentPath(),tr("XBEL Files (*.xbel *.xml)"));
    if (NameConfigSave.isEmpty()) return;
    QFile file(NameConfigSave);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(NameConfigSave)
                             .arg(file.errorString()));
        return;
    }
    if (!xmlRW->readFile(&file))
    {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Parse error in file %1:\n\n%2")
                             .arg(NameConfigSave)
                             .arg(xmlRW->errorString()));
    }
    else
    {
        DEStruct::DES_Adj.Dados.tamPop = DEStruct::DES_Adj.qdadeCrSalvos;
        LEENC->setText(QString::number(DEStruct::DES_Adj.numeroCiclos));
        LEEM->setText(QString::number(DEStruct::DES_Adj.jnrr));
        LEESE->setText(QString::number(DEStruct::DES_Adj.serr));
        intRealComboBox->setCurrentIndex(DEStruct::DES_Adj.isTipoExpo);
        polRacComboBox->setCurrentIndex(DEStruct::DES_Adj.isRacional?0:1);
        LEN->setText(QString::number(DEStruct::DES_Adj.Dados.tamPop));
        if(DEStruct::DES_Adj.isPararContinuarEnabled)
        {
            actionParar->setEnabled(true);
            if(DEStruct::DES_Adj.isPararContinuarEnabled==1) actionParar->setText("Parar");
            else actionParar->setText("Continuar");
        }
        else actionParar->setEnabled(false);
        if(DEStruct::DES_Adj.isIniciaEnabled) {DEStruct::DES_Adj.isCriado=true;Iniciar();}
        statusBar()->showMessage(tr("File carregado"), 20000);
        file.close();
    }

}
////////////////////////////////////////////////////////////////////////////
void ICalc::slot_MW_saveAsConfig()
{
    if(NameConfigSave.isEmpty()) NameConfigSave=QFileDialog::getSaveFileName(this, tr("Salvar dados de configura��o"),QDir::currentPath(),tr("XBEL Files (*.xbel *.xml)"));
    if(NameConfigSave.isEmpty()) return;
    QFile file(NameConfigSave);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("QXmlStream Bookmarks"),
                             tr("Arquivo %1 n�o pode ser escrito:\n%2.")
                             .arg(NameConfigSave)
                             .arg(file.errorString()));
        return;
    }
    DEStruct::DES_Adj.numeroCiclos = LEENC->text().toInt();
    DEStruct::DES_Adj.jnrr = LEEM->text().toDouble();
    DEStruct::DES_Adj.serr = LEESE->text().toDouble();
    DEStruct::DES_Adj.isTipoExpo = intRealComboBox->currentIndex();
    DEStruct::DES_Adj.isRacional = (polRacComboBox->currentIndex()==0);
    DEStruct::DES_Adj.isIniciaEnabled=actionIni->isEnabled();
    DEStruct::DES_Adj.salvarAutomati=true;
    DEStruct::DES_Adj.isPararContinuarEnabled = !actionParar->isEnabled()?0:actionParar->text()==QString("Parar")?1:2;
    DEStruct::DES_Adj.qdadeCrSalvos=DEStruct::DES_Adj.Dados.tamPop;
    DEStruct::DES_Adj.salvDadosCarre=1;
    DEStruct::DES_Adj.nomeArqConfTxT="";
    if (xmlRW->writeFile(&file))
        statusBar()->showMessage(tr("File salvo"), 20000);
    file.close();
}
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Fim ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
