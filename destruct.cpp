#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSemaphore>
#include <QDateTime>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMutexLocker>
#include <math.h>
#include <qmath.h>
#include <QLinkedList>
#include <limits>
#include <QtAlgorithms>
#include "destruct.h"
#include "xtipodados.h"
//#include <boost\bind\bind.hpp>
#define TAMMAXCARACTER 900000 //Tamanho maximo caracteres que o programa le de uma vez
#define MAX(a, b) ((a) > (b) ? (a) : (b))
////////////////////////////////////////////////////////////////////////////
//DES_size  = Quantidade de Thread.
//DES_count = Indicador de qual das thread que esta sendo usada.
//DES_index = Indicador de qual item da popula��o esta sendo trabalhado
////////////////////////////////////////////////////////////////////////////
QSemaphore     DEStruct::DES_justThread[TAMPIPELINE],
               DEStruct::DES_waitThread;
QReadWriteLock DEStruct::lock_DES_index[TAMPIPELINE],
               DEStruct::lock_DES_Elitismo[TAMPIPELINE],
               DEStruct::lock_DES_BufferSR,
               DEStruct::lock_DES_modeOper_TH,
               DEStruct::LerDados;
               //DEStruct::lock_DES_Pior[TAMPIPELINE],
               //DEStruct::lock_DES_Adj.vetPop[TAMPIPELINE],
QWaitCondition DEStruct::justSync,
               DEStruct::waitSync;
QMutex         DEStruct::mutex;       
////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////PIPELINE - Saidas - Cromossomo
QList<QVector<Cromossomo > >         DEStruct::DES_crMut = QVector<QVector<Cromossomo > >(TAMPIPELINE).toList();
QList<QVector<qreal > >              DEStruct::DES_somaSSE = QVector<QVector<qreal> >(TAMPIPELINE).toList();
                                     //DEStruct::DES_erroMelhor = QVector<QVector<qreal> >(TAMPIPELINE).toList(),
                                     //DEStruct::DES_BIC = QVector<QVector<qreal> >(TAMPIPELINE).toList();
QList<QVector<qint32 > >             DEStruct::DES_idChange = QVector<QVector<qint32 > >(TAMPIPELINE).toList();
QList<QList<QVector<qreal > > >      DEStruct::DES_vcalc = QVector<QList<QVector<qreal> > >(TAMPIPELINE).toList(),
                                     DEStruct::DES_residuos = QVector<QList<QVector<qreal> > >(TAMPIPELINE).toList();
QList<QList<QVector<Cromossomo > > > DEStruct::DES_BufferSR = QVector<QList<QVector<Cromossomo > > >(TAMPIPELINE).toList();
//QList<QList<QVector<Cromossomo > > > DEStruct::DES_forMut = QVector<QList<QVector<Cromossomo > > >(TAMPIPELINE).toList();
//QList<QList<Cromossomo *> >          DEStruct::DES_Pior = QVector<QList<Cromossomo * > >(TAMPIPELINE).toList();
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
Config          DEStruct::DES_Adj;
QList<qint32>   DEStruct::DES_cVariaveis;
QString         DEStruct::DES_fileName;
QAtomicInt DEStruct::DES_index[TAMPIPELINE];
bool            DEStruct::DES_isCarregar,
                DEStruct::DES_idParadaJust[TAMPIPELINE] = {false,false,false};
QAtomicInteger<qint64> DEStruct::tamArquivo(0);
QAtomicInt DEStruct::DES_TH_size(0);
QAtomicInt DEStruct::DES_countSR(0);
QList<qreal>    DEStruct::DES_mediaY,
                DEStruct::DES_mediaY2;
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Fun��es ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
qreal sign(const qreal &x)
{
    if(x>0) return 1;
    else if(x==0) return 0;
    else return -1; //if(x<0)
}
////////////////////////////////////////////////////////////////////////////
const compTermo XInv(compTermo var1)
{
    compTermo var;
    var.vTermo.tTermo0 = var1.vTermo.tTermo0;
    var.expoente = (-1)*var1.expoente;
    return(var);
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
const compTermo XSum(const compTermo &var1, const compTermo &var2)
{
    compTermo var;
    if((var1.vTermo.tTermo0&&var2.vTermo.tTermo0)?(var1.vTermo.tTermo2.idVar != var2.vTermo.tTermo2.idVar):false)
        qDebug() <<"Error:XSum Termos var1 != var2";
    else
    {
        var.vTermo.tTermo0 = var1.vTermo.tTermo0;
        var.expoente = var1.expoente+var2.expoente;
    }
    return(var);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const compTermo XMult(const quint32 &var1,const compTermo &var2)
{
    compTermo var;
    var.vTermo.tTermo0 = var2.vTermo.tTermo0;
    var.expoente = var1*var2.expoente;
    return(var);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//inline const qreal expoToReal(const compTermo *var)
//{
//    qreal result = ((qreal) var->vTermo.tTermo1.fracao)/MASKFRA;//Pega parte fracionaria
//    result = var->vTermo.tTermo1.inteira<0?((qreal) var->vTermo.tTermo1.inteira)-result:((qreal) var->vTermo.tTermo1.inteira)+result;
//    return(result);
//}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//inline void realToExpo(compTermo *var,const qreal exp)
//{
//    var->vTermo.tTermo1.inteira = ((qint32) exp < MASKINT ? exp : MASKINT);
//    var->vTermo.tTermo1.fracao = ((quint32) ((exp - ((qreal) var->vTermo.tTermo1.inteira))*MASKFRA));
//}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline bool CmpMaiorTerm(const compTermo &vlr1, const compTermo &vlr2)
{
    return (vlr1.vTermo.tTermo0>vlr2.vTermo.tTermo0);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline bool CmpMaiorApt(const qint32 &countCr1, const qint32 &countCr2,const qint32 &idSaida)
{
    QReadLocker locker(&DEStruct::lock_DES_BufferSR);
    const Cromossomo cr1 = DEStruct::DES_Adj.Pop.at(idSaida).at(countCr1);
    const Cromossomo cr2 = DEStruct::DES_Adj.Pop.at(idSaida).at(countCr2);
    return(cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*void crCopy(Cromossomo &cr1,const Cromossomo &cr2)
{
    cr1.termos.resize(cr2.termos.size());
    qCopy(cr2.termos.begin(),cr2.termos.end(),cr1.termos.begin());

    cr1.aptidao = cr2.aptidao;
    cr1.erro    = cr2.erro;

    cr1.idSaida = cr2.idSaida;
    cr1.maiorAtraso = cr2.maiorAtraso;

    cr1.coeficDen   = cr2.coeficDen;
    cr1.coeficNum   = cr2.coeficNum;
}*/
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void qSortDuplo(compTermo *start,compTermo *end,qint32 *posStart,qint32 *posEnd,bool (*compare)(const compTermo &,const compTermo &))
{
top:
    int span = int(end - start);
    if (span < 2) return;
    --end;
    --posEnd;

    compTermo *low = start, *high = end - 1;
    qint32 *posLow = posStart, *posHigh = posEnd-1;
    compTermo *pivot = start + span / 2;
    qint32 *posPivot = posStart + span / 2;
    //Se end menor que start
    if(compare(*end,*start)) {qSwap(*end, *start);qSwap(*posEnd, *posStart);}
    if(span == 2) return;
    if(compare(*pivot,*start)) {qSwap(*pivot, *start);qSwap(*posPivot, *posStart);}
    if(compare(*end,*pivot)) {qSwap(*end, *pivot);qSwap(*posEnd, *posPivot);}
    if(span == 3) return;

    qSwap(*pivot,*end);
    qSwap(*posPivot,*posEnd);

    while (low < high)
    {
        while((low < high) && (compare(*low,*end))) {++low;++posLow;}
        while((high > low) && (compare(*end,*high))) {--high;--posHigh;}
        if (low < high){qSwap(*low, *high);++low;--high;qSwap(*posLow, *posHigh);++posLow;--posHigh;}
        else break;
    }

    if (compare(*low,*end)){++low;++posLow;}

    qSwap(*end,*low);
    qSwap(*posEnd,*posLow);

    qSortDuplo(start, low, posStart, posLow,compare);

    start = low + 1;
    posStart = posLow + 1;

    ++end;
    ++posEnd;

    goto top;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////Calcula o sistema linear////////////////////////////
//C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..
template<typename T>
XVetor<T> SistemaLinear(bool &isOK,const JMathVar<T> &mat1,const XVetor<T> &vet1)
{
    qint32 n=vet1.size(),
           i,j,k,l;
    JMathVar<T> matAux(mat1);
    XVetor<T> result(n);
    matAux.append('C',vet1);//Matriz expandida (n,n+1)
    double termo,m;
    try
    {
        //Implementando M�todo de Gauss
        for (k=0;k<n-1;k++)
        {
            for (i=k+1; i<n;i++)
            {
                //Multiplicadores
                m=-1*(matAux.item(i,k)/matAux.item(k,k));
                //Matriz A Transformada em Gauss
                for (j=0;j<n+1;j++) matAux(i,j)=(matAux.item(k,j)*m)+matAux.item(i,j);
            }
        }
        /*Resolvendo o sistema*/
        for (i=0;i<n;i++)
        {
            termo=0;
            l=n-i;
            for (j=l;j<n;j++) termo=termo+(result.at(j)*matAux.item(n-i-1,j));
            result[n-i-1]=(matAux.item(n-1-i,n)-termo)/matAux.item(n-i-1,n-i-1);
        }
        isOK=true;
    }
    catch(char * str) {isOK=false;}
    return result;
}
////////////////////////////////////////////////////////////////////////////////
///////////////Calcula a matriz ao quadrado Transposta//////////////////////////
//A'*A : sendo mat1=A'
template<typename T>
JMathVar<T> AoQuadTrans(const JMathVar<T> &mat1)
{
    JMathVar<T> result(mat1.numLinhas(),mat1.numLinhas());
    for (qint32 linha = 0; linha < mat1.numLinhas(); linha++)
    {
        for (qint32 coluna = 0; coluna < mat1.numLinhas(); coluna++)
        {
            T sum(0.0f);
            for (int aux = 0; aux < mat1.numColunas(); aux++)
                sum += mat1.at(linha,aux) * mat1.at(coluna,aux);
            result(linha,coluna) = (sum!=sum)||(sum>1e9f) ? 1e9f: sum;
        }
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////
///////////////Calcula a matriz ao quadrado Transposta//////////////////////////
//A'*A : sendo mat1=A
template<typename T>
JMathVar<T> AoQuad(const JMathVar<T> &mat1)
{
    JMathVar<T> result(mat1.numColunas(),mat1.numColunas());
    for (qint32 linha = 0; linha < mat1.numColunas(); linha++)
    {
        for (qint32 coluna = 0; coluna < mat1.numColunas(); coluna++)
        {
            T sum(0.0f);
            for (int aux = 0; aux < mat1.numLinhas(); aux++)
                sum += mat1.item(aux,linha) * mat1.item(aux,coluna);
            result(linha,coluna) = (sum!=sum)||(sum>1e9f) ? 1e9f: sum;
        }
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////
//////////Calcula a multiplica��o de uma matriz Transpostas por um vetor////////
//<M12, M21> = <M12, M11> * <M22,1>
template<typename T>
XVetor<T> MultMatTransVet(const JMathVar<T> &mat1,const XVetor<T> &vet1)
{
    XVetor<T> result(mat1.numColunas());
    if(mat1.numLinhas()==vet1.size())
    {
        for (qint32 coluna = 0; coluna < mat1.numColunas(); coluna++)
        {
            T sum(0.0f);
            for (qint32 aux = 0; aux < mat1.numLinhas(); aux++)
                sum += mat1.item(aux,coluna) * vet1.at(aux);
            result[coluna] = (sum!=sum) ? 1e9f: sum;
        }
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////
///////////////Calcula a multiplica��o de uma matriz por um vetor///////////////
//<M12, M21> = <M12, M11> * <M22,1>
template<typename T>
XVetor<T> MultMatVet(const JMathVar<T> &mat1,const XVetor<T> &vet1)
{
    XVetor<T> result(mat1.numLinhas());
    if(mat1.numColunas()==vet1.size())
    {
        for (qint32 linha = 0; linha < mat1.numLinhas(); linha++)
        {
            T sum(0.0f);
            for (qint32 aux = 0; aux < mat1.numColunas(); aux++)
                sum += mat1.item(linha,aux) * vet1.at(aux);
            result[linha] = (sum!=sum) ? 1e9f: sum;
        }
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Classe /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DEStruct::DEStruct() : QThread()
{
    {
        QMutexLocker locker(&mutex);
        DES_TH_id = DES_TH_size.fetchAndAddOrdered(0);
        if(DES_TH_id)//Cria uma quantidade de semafaros = qtdeThread-1
        {
            for(qint32 count=0;count<TAMPIPELINE;count++) DES_justThread[count].release();
            DES_waitThread.release();
        }
        else
        {
            DES_Adj.modeOper_TH = 1;    
            DES_Adj.vetPop = QVector<QList<qint32 > >(TAMPIPELINE).toList();
            DES_Adj.vetElitismo = QVector<QList<QVector<qint32 > > >(TAMPIPELINE).toList();
            DES_Adj.isSR = QVector<QList<QList<bool> > >(TAMPIPELINE).toList();
            DES_Adj.isCriado=false;
        }
        DES_TH_size.fetchAndAddOrdered(1);
    }
    //DES_LM = new SRLevMarq();
    DES_Adj.Dados.variaveis.nome.clear();
    DES_Adj.Dados.variaveis.valores.clear();
    DES_Adj.Dados.variaveis.Vmaior.clear();
    DES_Adj.Dados.variaveis.Vmenor.clear();
    DES_RG.seed(QTime::currentTime().msec());
    DES_isEquacaoEscrita.storeRelaxed(1);
    DES_isStatusSetado.storeRelaxed(1);
    //DES_vlrRegressores = new XMatriz<qreal>();
    start();//Inicia thread
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DEStruct::~DEStruct()
{
    {
        QMutexLocker locker(&mutex);
        DES_Adj.modeOper_TH = 0;
        DES_TH_size.fetchAndAddOrdered(-1);
    }
    quit();
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::qSortPop(qint32 *start, qint32 *end,const qint32 &idSaida) const
{
top:
    int span = int(end - start);
    if (span < 2) return;
    --end;

    qint32 *low = start, *high = end - 1;
    qint32 *pivot = start + span / 2;

    if (CmpMaiorApt(*end, *start, idSaida)) qSwap(*end, *start);
    if (span == 2) return;
    if (CmpMaiorApt(*pivot, *start, idSaida)) qSwap(*pivot, *start);
    if (CmpMaiorApt(*end, *pivot, idSaida))qSwap(*end, *pivot);
    if(span == 3) return;

    qSwap(*pivot,*end);

    while (low < high)
    {
        while(low < high && CmpMaiorApt(*low, *end, idSaida))++low;
        while (high > low && CmpMaiorApt(*end, *high, idSaida))--high;
        if (low < high){qSwap(*low, *high);++low;--high;}
        else break;
    }

    if (CmpMaiorApt(*low, *end, idSaida))++low;

    qSwap(*end,*low);

    qSortPop(start, low, idSaida);

    start = low + 1;
    ++end;
    goto top;
}
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_Carregar()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 nlinha = DES_cVariaveis.size()/*variaveis*/;
    const qint32 qtSaidas = DES_Adj.Dados.variaveis.qtSaidas;
    bool isOk=false,isOkIni = false,isNumber = false,isNormalizado=false;
    qint32 indexIni=0,index=0,i=0,j=0,ncoluna=0,idSaida=0;
    QList<QString> nome;
    QString str;
    QList<qreal> posPonto,
                 Vmaior,
                 Vmenor,
                 mediaY = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList(),
                 mediaY2 = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList();
    qreal talr  =0,
          talr2 =0,
          talMin=0,
          *valor=NULL;
    QVector<qreal> Max1,Min1;
    ////////////////////////////////////////////////////////////////////////////
    QStringList strList,lineList;
    QByteArray line,lineMeio/*,lineAntes*/,lineDepois;
    QFile file(DES_fileName);
    qint64 posicaoIni,posicaoFinal;
    //Se for concatenar cada thread ja armazena seus valores de maximo e minimo anteriores
    ////////////////////////////////////////////////////////////////////////////
    if(!DES_isCarregar)
    {
        Max1 = DES_Adj.Dados.variaveis.Vmaior.toVector();
        Min1 = DES_Adj.Dados.variaveis.Vmenor.toVector();
    }
    ////////////////////////////////////////////////////////////////////////////
    //Roda apenas uma thread para inicializar o tamanho do vetor dos dados e do arquivo.
    {
        QMutexLocker locker(&mutex);
        if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
        else
        {
            DES_justThread[0].release(DES_TH_size.fetchAndAddOrdered(0)-1);
            DES_index[0].storeRelaxed(0);
            if(DES_isCarregar)
            {
                DES_Adj.Dados.variaveis.nome.clear();
                DES_Adj.Dados.variaveis.valores.clear();
            }
            else indexIni = DES_Adj.Dados.variaveis.valores.numColunas();
            DES_Adj.Dados.variaveis.Vmaior.clear();
            DES_Adj.Dados.variaveis.Vmenor.clear();
            DES_mediaY = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList();
            DES_mediaY2 = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList();
            if (file.open(QFile::ReadOnly))
            {
                tamArquivo.storeRelaxed(file.size());
                file.close();
            }
            else qDebug() << "Func:DES_Carregar - N�o abriu arquivo para ler tamanho";
            emit signal_DES_Status(0);
            //Le a variavel sem o QReadWriteLock pois apenas uma thread esta rodando.
            if(DES_Adj.modeOper_TH==2) waitSync.wait(&mutex);
            justSync.wakeAll();
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    //Verifica se � para fechar o programa.
    {
        QReadLocker locker(&lock_DES_modeOper_TH);
        isOk = DES_Adj.modeOper_TH<=1;
    }
    if(isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    //Monta a divisao para cada thread
    const qint64 tamArquivoValue = tamArquivo.loadAcquire();
    const qint32 DES_TH_sizeValue = DES_TH_size.loadAcquire();
    const qint32 tamCadaTh = ((DES_TH_id+1)==DES_TH_sizeValue)?tamArquivoValue - (DES_TH_id*(tamArquivoValue/DES_TH_sizeValue)):(tamArquivoValue/DES_TH_sizeValue);//Tamanho para cada thread sendo a ultima diferente
    const qint32 tamCadaRepet = (tamCadaTh>TAMMAXCARACTER)?TAMMAXCARACTER:tamCadaTh;
    const qint32 numRepet = (tamCadaTh/tamCadaRepet) + (tamCadaTh%tamCadaRepet?1:0);
    ////////////////////////////////////////////////////////////////////////////
    //Le do arquivo os dados referentes a esta thread.
    posicaoFinal = DES_TH_id*(tamArquivo/DES_TH_size);
    //lock_DES_index[0].lockForWrite();
    if(file.open(QFile::ReadOnly))
    {
        for(index=0;index<numRepet;index++)
        {
            ////////////////////////////////////////////////////////////////////////////
            posicaoIni = posicaoFinal;
            posicaoFinal = (posicaoIni+tamCadaRepet);
            posicaoFinal =  posicaoFinal>tamArquivo?tamArquivo:posicaoFinal>((DES_TH_id+1)*(tamArquivo/DES_TH_size))?((DES_TH_id+1)*(tamArquivo/DES_TH_size)):posicaoFinal;
            ////////////////////////////////////////////////////////////////////////////
            lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
            if(isOk) return;
            ////////////////////////////////////////////////////////////////////////////
            lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH==2;lock_DES_modeOper_TH.unlock();
            mutex.lock();
            if(isOk) waitSync.wait(&mutex);
            mutex.unlock();
            ////////////////////////////////////////////////////////////////////////////
            //Posiciona no inicio da leitura dos dados para esta itera��o nesta thread.
            file.seek(posicaoIni?posicaoIni-1:0);
            //L� os dados referentes a esta itera��o.
            lineMeio = file.read(posicaoFinal-posicaoIni+(posicaoIni?1:0));
            //Busca o fim da linha se n�o for a ultima itera��o da ultima thread.
            lineDepois.clear();
            if(lineMeio.size()?(lineMeio.right(1)!=QByteArray(1, '\n'))&&(lineMeio.right(1)!= QByteArray(1,'\0')):true)
            {
                line = file.read(1);
                while((line != QByteArray("\n"))&&(line != QByteArray("\0")))
                {
                    lineDepois.append(line);
                    line = file.read(1);
                }
            }
            //Busca o inicio da linha se n�o for a primeira itera��o da primeira thread.
            //lineAntes.clear();
            if(DES_TH_id||index)
            {

                line = lineMeio.left(1);
                while((line != QByteArray("\n"))&&(line != QByteArray("\0")))
                {
                    lineMeio.remove(0,1);
                    line = lineMeio.left(1);
                }
                /*
                file.seek(posicaoIni);
                if(file.peek(1)==QChar(10)) i=-2;
                else i=-1;
                file.seek(posicaoIni+i);
                line = file.peek(1);
                while( line != QChar(10)&&(posicaoIni+i)>=0)
                {
                    lineAntes.prepend(line);
                    i--;
                    file.seek(posicaoIni+i);
                    line = file.peek(1);
                }*/
            }
            ////////////////////////////////////////////////////////////////////////////
            line = lineMeio.right(40);
            lineList = QString(/*lineAntes+*/lineMeio+lineDepois).replace(QRegExp("\\r"),"").split(QRegExp("\\n"),QString::SkipEmptyParts);
            ////////////////////////////////////////////////////////////////////////////
            for(i=0/*(DES_TH_id||index)?1:0*/;i<lineList.size();i++)
            {
                if(lineList.at(i).size())
                {
                    strList = lineList.at(i).split(QString(QChar(32)),QString::SkipEmptyParts);
                    if(strList.size()?strList.first().isEmpty():false)
                        strList.removeFirst();
                    if(strList.size()>=DES_cVariaveis.size())
                    {
                        for(j=0,isNumber=true;(isNumber)&&(j<strList.size());j++) strList.at(j).toDouble(&isNumber);
                        if(j==strList.size())
                        {
                            for(j=0;j < nlinha;j++)
                            {
                                posPonto.append((QString().setNum(strList.at(DES_cVariaveis.at(j)).toDouble(),'E',9)).toDouble());                               
                                if(j<qtSaidas)
                                {
                                    mediaY[j] += posPonto.last();
                                    mediaY2[j] += posPonto.last()*posPonto.last();
                                }
                                //Se ja possui m�ximos e m�nimos n�o necessita fazer o incremento
                                if(!DES_Adj.Dados.variaveis.Vmaior.size())
                                {
                                    if(!isOkIni)
                                    {
                                        Vmaior.append(posPonto.last());
                                        Vmenor.append(posPonto.last());
                                        isOkIni = (Vmaior.size() >= nlinha);
                                    }
                                    else
                                    {
                                        if(Vmaior.at(j) < posPonto.last()) Vmaior.replace(j,posPonto.last());
                                        if(Vmenor.at(j) > posPonto.last()) Vmenor.replace(j,posPonto.last());
                                    }
                                }
                                else if(!isNormalizado) isNormalizado=true;
                            }
                        }
                        else
                        {
                            for(j=0;j < nlinha;j++)
                            {
                                str = strList.at(DES_cVariaveis.at(j));
                                QRegExp rx("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*),([-+]?\\d*)\\)"); //Float com ponto opcional e o exp opcional
                                int pos = rx.indexIn(str);
                                if(pos>-1)
                                {
                                    DES_Adj.Dados.variaveis.Vmaior.append(rx.cap(1).toDouble());
                                    DES_Adj.Dados.variaveis.Vmenor.append(rx.cap(2).toDouble());
                                    DES_Adj.decimacao.append(rx.cap(3).toInt());
                                    str.replace(rx,"");
                                }
                                else
                                {
                                    rx = QRegExp("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\)"); //Float com ponto opcional
                                    pos = rx.indexIn(str);
                                    if(pos>-1)
                                    {
                                        DES_Adj.Dados.variaveis.Vmaior.append(rx.cap(1).toDouble());
                                        DES_Adj.Dados.variaveis.Vmenor.append(rx.cap(2).toDouble());
                                        str.replace(rx,"");
                                    }
                                }
                                str.replace("(","");
                                str.replace(")","");
                                nome.append(str);
                            }
                        }
                    }
                }
            }
        }
        //Fecha o arquivo e liberando-o.
        file.close();
    }
    else qDebug() << "Func:DES_Carregar - N�o abriu arquivo para ler dados";
    //lock_DES_index[0].unlock();
    lineList.clear();
    ////////////////////////////////////////////////////////////////////////////
    lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
    if(isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread roda para inicializar o index
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else
    {
        DES_justThread[0].release(DES_TH_size-1);
        DES_index[0]=0;
        emit signal_DES_Status(1);
        if(DES_Adj.modeOper_TH==2) waitSync.wait(&mutex);
        justSync.wakeAll();
    }
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread roda por vez em ordem concatenando os dados
    mutex.lock();
    do
    {
        if(DES_index[0]!=DES_TH_id) justSync.wait(&mutex);
        else
        {
            ncoluna = posPonto.size()/nlinha;
            DES_Adj.Dados.variaveis.valores.append(posPonto,nlinha,ncoluna);
            for(idSaida=0;idSaida<qtSaidas;idSaida++)
            {
                DES_mediaY[idSaida] += mediaY.at(idSaida);
                DES_mediaY2[idSaida] += mediaY2.at(idSaida);
            }
            posPonto.clear();
            DES_index[0]++;
            if(!Vmaior.empty()&&!isNormalizado)
            {
                if(DES_Adj.Dados.variaveis.Vmaior.isEmpty()) {DES_Adj.Dados.variaveis.Vmaior.append(Vmaior);DES_Adj.Dados.variaveis.Vmenor.append(Vmenor);}
                else for(j=0;j < nlinha;j++)
                {
                    if(DES_Adj.Dados.variaveis.Vmaior.at(j) < Vmaior.at(j)) DES_Adj.Dados.variaveis.Vmaior.replace(j,Vmaior.at(j));
                    if(DES_Adj.Dados.variaveis.Vmenor.at(j) > Vmenor.at(j)) DES_Adj.Dados.variaveis.Vmenor.replace(j,Vmenor.at(j));
                }
            }
            if(!nome.empty()&&(DES_isCarregar||DES_Adj.Dados.variaveis.nome.isEmpty())) DES_Adj.Dados.variaveis.nome.append(nome);
            if(DES_Adj.modeOper_TH==2) waitSync.wait(&mutex);
            justSync.wakeAll();
        }
    }
    while(DES_index[0]<DES_TH_size);
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
    if(isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread qualquer roda para inicializar o index
    /*
    justMutex.lock();
    if(justThread[0].tryAcquire()) sync.wait(&justMutex);
    else
    {
        justThread[0].release(DES_TH_size-1);
        DES_index[0]=0;
        emit signal_DES_Status(2);
        if(DES_modeOper_TH==2) waitSync.wait(&justMutex);
        sync.wakeAll();
    }
    justMutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    lock_DES_modeOper_TH.lockForRead();isOk=DES_modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
    if(isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    //As threads rodam para desnormalizar os dados antigos se for concatenar.
    forever
    {
        lock_DES_index[0].lockForWrite();index = DES_index[0]++;lock_DES_index[0].unlock();
        if((index < ncoluna)&&(index<indexIni))
            for(j=0;j < nlinha;j++)
                //DES_Adj.Dados.variaveis.valores(j,index) = QString::number((DES_Adj.Dados.variaveis.valores.item(j,index)*(Max1.at(j)-Min1.at(j))+Min1.at(j)),'f', 2).toDouble();
                DES_Adj.Dados.variaveis.valores(j,index) = (DES_Adj.Dados.variaveis.valores.item(j,index)*(Max1.at(j)-Min1.at(j)))+Min1.at(j);
        else break;
    }
    ////////////////////////////////////////////////////////////////////////////
    lock_DES_modeOper_TH.lockForRead();isOk=DES_modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
    if(isOk) return;*/
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread qualquer roda para abrir a tela de Normaliza��o.
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else
    {
        DES_justThread[0].release(DES_TH_size-1);
        DES_index[0]=0;
        DES_index[0]=0;
        //////////////////////////////////////////////////////////////        
        if(!isNormalizado)
        {
            //////////////////////////////////////////////////////////////
            //Calcula a Decima��o
            j=DES_Adj.Dados.variaveis.valores.numLinhas();
            for(idSaida=0;idSaida<qtSaidas;idSaida++)
            {
                DES_Adj.decimacao.append(1);
                DES_mediaY[idSaida]/=DES_Adj.Dados.variaveis.valores.numColunas();
                DES_mediaY2[idSaida]/=DES_Adj.Dados.variaveis.valores.numColunas();
                for(i=1;i<DES_Adj.Dados.variaveis.valores.numColunas();i++)
                {
                    talr+=0;
                    talr2+=0;
                    for(valor = DES_Adj.Dados.variaveis.valores.begin()+(i*j)+idSaida;valor<DES_Adj.Dados.variaveis.valores.end();valor+=j)
                    {
                        talr+=((*valor)-DES_mediaY.at(idSaida))*(*(valor-(i*j))-DES_mediaY.at(idSaida));
                        talr2+=(((*valor)*(*valor))-DES_mediaY2.at(idSaida))*(((*(valor-(i*j)))*(*(valor-(i*j))))-DES_mediaY2.at(idSaida));
                    }
                    talr/=DES_Adj.Dados.variaveis.valores.numColunas();
                    talr2/=DES_Adj.Dados.variaveis.valores.numColunas();
                    if((i==1)||(talr<talMin)||(talr2<talMin)) {if(talr<talr2) talMin=talr; else talMin=talr2;}
                    else {DES_Adj.talDecim.append(i);break;}
                }
            }
            emit signal_DES_Status(3);
        }
        else emit signal_DES_Status(4);
        emit signal_DES_Tam();
        lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH==2;lock_DES_modeOper_TH.unlock();
        if(isOk) waitSync.wait(&mutex);
        lock_DES_modeOper_TH.lockForWrite();
        DES_Adj.modeOper_TH = 1;//Finaliza a tarefa colocando a thread em espera
        lock_DES_modeOper_TH.unlock();
        justSync.wakeAll();
    }
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::slot_DES_Normalizar()
{/*
    ////////////////////////////////////////////////////////////////////////////
    const qint32 nlinha =DES_Adj.Dados.variaveis.valores.numLinhas();//Variaveis
    qint32 index=0,j=0,ncoluna=DES_Adj.Dados.variaveis.valores.numColunas();//Atrasos
    ////////////////////////////////////////////////////////////////////////////
    //Normalizando os dados (0 � 1).
    forever
    {
        lock_DES_index[0].lockForWrite();index = DES_index[0]++;lock_DES_index[0].unlock();
        if(index < ncoluna)
        {
            for(j=0;j < nlinha;j++)
            {
                DES_Adj.Dados.variaveis.valores(j,index) = (QString("%1").arg(DES_Adj.Dados.variaveis.valores(j,index)>=DES_Adj.Dados.variaveis.Vmenor.at(j)?DES_Adj.Dados.variaveis.valores(j,index)<=DES_Adj.Dados.variaveis.Vmaior.at(j)?(DES_Adj.Dados.variaveis.valores.at(j,index)-DES_Adj.Dados.variaveis.Vmenor.at(j))/(DES_Adj.Dados.variaveis.Vmaior.at(j)-DES_Adj.Dados.variaveis.Vmenor.at(j)):1:0)).toDouble();
            }
        }
        else break;
    }*/
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread roda para fechar a tela
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else
    {
        DES_justThread[0].release(DES_TH_size-1);
        DES_index[0]=0;
        emit signal_DES_Status(4);
        justSync.wakeAll();
    }
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    DES_Adj.modeOper_TH = 1;//Finaliza a tarefa
    ////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::slot_DES_Estado(const quint16 &std)
{
    lock_DES_modeOper_TH.lockForWrite();
    DES_Adj.modeOper_TH = std;
    lock_DES_modeOper_TH.unlock();
    waitSync.wakeAll();
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::run()
{
    bool isOk=false;
    while(DES_Adj.modeOper_TH)
    {
        switch(DES_Adj.modeOper_TH)
        {
            case 0://Termina a Thread para fechar o programa
                break;
            case 1://Tarefa finalizou e a Thread espera o proximo comando.
                mutex.lock();
                if(DES_waitThread.tryAcquire()) waitSync.wait(&mutex);
                else{DES_waitThread.release(DES_TH_size-1);signal_DES_Finalizado();waitSync.wait(&mutex);}
                mutex.unlock();
                break;
            case 2://Thread para onde estiver e espera para continuar ou finalizar.
                mutex.lock();
                if(DES_waitThread.tryAcquire()) waitSync.wait(&mutex);
                else{DES_waitThread.release(DES_TH_size-1);signal_DES_Parado();waitSync.wait(&mutex);}
                mutex.unlock();
                break;
            case 3://Executando o DE em Multi-Thread...
                DES_AlgDiffEvol();
                break;
            case 4://Carregando e condicionando os dados em Multi-Thread...
                DES_Carregar();
                break;
            case 5://Normalizando os dados em Multi-Thread...
                slot_DES_Normalizar();
                break;
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    //Apenas uma thread roda para fechar a tela principal
    mutex.lock();
    waitSync.wakeAll();
    if(!DES_waitThread.tryAcquire()) isOk=true;
    mutex.unlock();
    if(isOk) emit signal_DES_closed();
    ////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::slot_DES_EquacaoEscrita()
{
    DES_isEquacaoEscrita.storeRelaxed(1);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::slot_DES_StatusSetado()
{
    DES_isStatusSetado.storeRelaxed(1);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_AlgDiffEvol()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 qtSaidas = DES_Adj.Dados.variaveis.qtSaidas;
    const qint32 tamPop = DES_Adj.Dados.tamPop;
    //QList<QVector<Cromossomo > > buffer;   
    Cromossomo cr0, cr1, cr2;// cr3;
    qint32 tokenPop;
    JMathVar<qreal> m1(10,10,5.0),m2(10,10,5.0);
    QVector<Cromossomo > crBest(qtSaidas);
    ////////////////////////////////////////////////////////////////////////////
    bool isOk=false,isPrint=true;
    qint32 count0=0,count1=0,count2=0,cr0Point,cr1Point,cr2Point,idSaida=0,idPipeLine = 0;//count3=0;
    ////////////////////////////////////////////////////////////////////////////
    for(idPipeLine=0;idPipeLine<TAMPIPELINE;idPipeLine++) DES_idParada_Th[idPipeLine] = !DES_idParadaJust[count0];
    ////////////////////////////////////////////////////////////////////////////    
    //Apenas um thread qualquer inicializa as variaveis e os ponteiros deste m�todo.
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else
    {
        for(count2=0;count2<m1.size();count2++) {m1[count2]=count2;m2[count2]=count2;}
        DES_justThread[0].release(DES_TH_size-1);
        DES_Adj.tp = QTime::currentTime();
        for(idPipeLine=0;idPipeLine<TAMPIPELINE;idPipeLine++)
        {
            if(idPipeLine) DES_Adj.vetPop[idPipeLine].clear();
            for(idSaida=0;idSaida<qtSaidas;idSaida++)
            {
                DES_idChange[idPipeLine][idSaida]=tamPop+1;
                DES_Adj.vetElitismo[idPipeLine][idSaida].append(1);
                DES_Adj.vetElitismo[idPipeLine][idSaida].append(0);
                for(count0=2;count0<tamPop;count0++)
                    DES_Adj.vetElitismo[idPipeLine][idSaida].append(count0);
                DES_crMut[idPipeLine][idSaida] = DES_criaCromossomo(idSaida);//� o melhor cromossomo inicial
                DES_Adj.melhorAptidaoAnt.clear();
                DES_Adj.melhorAptidaoAnt.append(DES_crMut.at(idPipeLine).at(idSaida).aptidao);
            }
        }
        for(count0=0;count0<tamPop;count0++)
        {
            if(DES_Adj.Dados.isElitismo == 1?count0>=DES_Adj.Dados.iElitismo:true) DES_Adj.vetPop[0].append(count0);
            else DES_Adj.vetPop[TAMPIPELINE-1].append(count0);
        }
        justSync.wakeAll();
    }
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    //Calcula a aptid�o ou cria os cromossomos iniciais
    ////////////////////////////////////////////////////////////////////////////
    idPipeLine = 0;
    isOk=false;
    lock_DES_index[0].lockForWrite();
    if(DES_Adj.vetPop.at(0).size()&&!DES_index[0])
    {
        isOk=true;
        tokenPop = DES_Adj.vetPop[0].takeFirst();
    }
    lock_DES_index[0].unlock();
    ////////////////////////////////////////////////////////////////////////////
    while(isOk)
    {
        for(idSaida=0;idSaida<qtSaidas;idSaida++)
        {
            if(!DES_Adj.isCriado) DES_Adj.Pop[idSaida][tokenPop] = DES_criaCromossomo(idSaida);
            else DES_calAptidao(DES_Adj.Pop[idSaida][tokenPop]);
        }
        ////////////////////////////////////////////////////////////////////////////
        //Insere o cromossomo no final do vetor produtor.
        lock_DES_index[TAMPIPELINE-1].lockForWrite();
        DES_Adj.vetPop[TAMPIPELINE-1].append(tokenPop);
        lock_DES_index[TAMPIPELINE-1].unlock();
        ////////////////////////////////////////////////////////////////////////////
        //Checa se tem que continuar ou parar
        isOk=false;
        lock_DES_index[0].lockForWrite();
        if(DES_Adj.vetPop.at(0).size()&&!DES_index[0])
        {
            isOk=true;
            tokenPop = DES_Adj.vetPop[0].takeFirst();
        }
        lock_DES_index[0].unlock();
        ////////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////
    idPipeLine = 2;
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else{DES_Adj.isCriado=true;DES_justThread[0].release(DES_TH_size-1);justSync.wakeAll();}
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    //Inicia o processo do DE
    ////////////////////////////////////////////////////////////////////////////
    forever
    {
        isOk=false;
        lock_DES_index[idPipeLine].lockForWrite();
        if(DES_Adj.vetPop.at(idPipeLine).size()&&!DES_index[idPipeLine])
        {
            isOk=true;
            tokenPop = DES_Adj.vetPop[idPipeLine].takeFirst();
        }
        lock_DES_index[idPipeLine].unlock();
        while(isOk)
        {
            for(idSaida=0;idSaida<qtSaidas;idSaida++)
            {
                ////////////////////////////////////////////////////////////////////////////
                //Cria um cromossomo para cada popula��o altera��o aleatoriamente.
                count0 = DES_RG.randInt(0,tamPop-1);
                do{count1 = DES_RG.randInt(0,tamPop-1);}while(count1 == count0);
                do{count2 = DES_RG.randInt(0,tamPop-1);}while((count2 == count0)||(count2 == count1));
                //count3 = DES_RG.randInt(0,DES_BufferSR.at(idPipeLine).at(idSaida).size()-1);
                //do{count3 = DES_RG.randInt(0,tamPop-1);}while((count3 == count1)||(count3 == count2)||(count3 == count0));
                lock_DES_Elitismo[TAMPIPELINE].lockForRead();
                cr0Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count0);
                cr1Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count1);
                cr2Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count2);
                lock_DES_Elitismo[TAMPIPELINE].unlock();
                lock_DES_BufferSR.lockForRead();
                cr0=DES_Adj.Pop.at(idSaida).at(cr0Point);
                cr1=DES_Adj.Pop.at(idSaida).at(cr1Point);
                cr2=DES_Adj.Pop.at(idSaida).at(cr2Point);
                //cr3=DES_BufferSR.at(idPipeLine).at(idSaida).size()?DES_BufferSR[idPipeLine][idSaida].at(count3):DES_criaCromossomo(idSaida);
                lock_DES_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////
                //Faz a outra parcela da muta��o e o cruzamento. 
                if(tokenPop == DES_idChange.at(idPipeLine).at(idSaida))
                {
                    lock_DES_BufferSR.lockForWrite();
                    DES_Adj.Pop[idSaida][tokenPop]= DES_BufferSR.at(idPipeLine).at(idSaida).at(0);
                    lock_DES_BufferSR.unlock();
                }
                ////////////////////////////////////////////////////////////////////////////
                //DES_CruzMut(DES_Adj.Pop[idSaida][tokenPop],DES_crMut[idPipeLine][idSaida],DES_criaCromossomo(idSaida),cr0,cr1);
                //DES_CruzMut(DES_Adj.Pop[idSaida][tokenPop],cr0,DES_criaCromossomo(idSaida),cr1,cr2);
                DES_CruzMut(DES_Adj.Pop[idSaida][tokenPop],cr0,DES_crMut[idPipeLine][idSaida],cr1,cr2);
                ////////////////////////////////////////////////////////////////////////////
                //Calcula o SSE medio.
                lock_DES_index[idPipeLine].lockForWrite();
                DES_somaSSE[idPipeLine][idSaida] += DES_Adj.Pop.at(idSaida).at(tokenPop).erro;
                lock_DES_index[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
            }
            ////////////////////////////////////////////////////////////////////////////
            //Insere o cromossomo no final do vetor produtor.
            lock_DES_index[idPipeLine==0?TAMPIPELINE-1:idPipeLine-1].lockForWrite();
            DES_Adj.vetPop[idPipeLine==0?TAMPIPELINE-1:idPipeLine-1].append(tokenPop);
            lock_DES_index[idPipeLine==0?TAMPIPELINE-1:idPipeLine-1].unlock();
            ////////////////////////////////////////////////////////////////////////////
            //Checa se tem que continuar ou parar
            isOk=false;
            lock_DES_index[idPipeLine].lockForWrite();
            if(DES_Adj.vetPop.at(idPipeLine).size()&&!DES_index[idPipeLine])
            {
                isOk=true;
                tokenPop = DES_Adj.vetPop[idPipeLine].takeFirst();
            }
            lock_DES_index[idPipeLine].unlock();
            ////////////////////////////////////////////////////////////////////////////
        }
        lock_DES_index[idPipeLine].lockForWrite();
        DES_index[idPipeLine]++;
        isOk = (DES_idParadaJust[idPipeLine]!=DES_idParada_Th[idPipeLine]);//Garante que a mesma thread n�o passe duas vezes pela mesma chamada
        lock_DES_index[idPipeLine].unlock();
        if(isOk)
        {
            DES_idParada_Th[idPipeLine] = DES_idParada_Th[idPipeLine]?false:true;
            ////////////////////////////////////////////////////////////////////////////
            //Uma thread fica para fazer algumas pondera��es e as outras v�o fazer as proximas instru��es.
            if(!DES_justThread[idPipeLine].tryAcquire())
            {
                DES_justThread[idPipeLine].release(DES_TH_size-1);
                ////////////////////////////////////////////////////////////////////////////
                //O elitismo � feito colocando os melhores (menor BIC) no inicio do vetor.
                isOk = true;
                for(idSaida=0;idSaida<qtSaidas;idSaida++)
                {
                    //lock_DES_Elitismo[idPipeLine].lockForWrite();
                    cr2Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);
                    qSortPop(DES_Adj.vetElitismo[idPipeLine][idSaida].begin(),DES_Adj.vetElitismo[idPipeLine][idSaida].end(),idSaida);
                    cr0Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);
                    //cr1Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(DES_RG.randInt(0,(tamPop-1)<10?tamPop-1:10));
                    //lock_DES_Elitismo[idPipeLine].unlock();

                    lock_DES_BufferSR.lockForRead();
                    crBest[idSaida]=DES_Adj.Pop.at(idSaida).at(cr0Point);
                    if((DES_Adj.melhorAptidaoAnt.at(idSaida)-crBest.at(idSaida).aptidao)>=DES_Adj.jnrr) DES_Adj.melhorAptidaoAnt[idSaida] = crBest.at(idSaida).aptidao;
                    else isOk = false;
                    //DES_idChange[idPipeLine][idSaida]=cr1Point;
                    //cr1 = DES_Adj.Pop.at(idSaida).at(cr1Point);
                    cr2 = DES_Adj.Pop.at(idSaida).at(cr2Point);
                    isPrint = isPrint||(crBest.at(idSaida).aptidao<cr2.aptidao);
                    lock_DES_BufferSR.unlock();

                    //DES_SuperResp(cr1,DES_BufferSR[idPipeLine][idSaida]);
                    //DES_Mutacao(DES_crMut[idPipeLine][idSaida],cr1,DES_criaCromossomo(idSaida));
                    //DES_crMut[idPipeLine][idSaida] = crBest.at(idSaida);//DES_criaCromossomo(idSaida);
                    DES_crMut[idPipeLine][idSaida] = DES_criaCromossomo(idSaida);
                }
                ////////////////////////////////////////////////////////////////////////////
                lock_DES_BufferSR.lockForWrite();
                DES_Adj.iteracoes++;
                if(isOk) DES_Adj.iteracoesAnt = DES_Adj.iteracoes;
                else if(DES_Adj.iteracoes>=DES_Adj.iteracoesAnt+DES_Adj.numeroCiclos) slot_DES_Estado(2);
                isOk = (((DES_Adj.tp.secsTo(QTime::currentTime()) >= 6)&&isPrint)||(DES_Adj.tp.secsTo(QTime::currentTime()) >= 60));
                if(isOk) DES_Adj.tp = QTime::currentTime();
                lock_DES_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////                
                if(isOk)
                {
                    for(idSaida=0;idSaida<qtSaidas;idSaida++)
                        DES_MontaSaida(crBest[idSaida],DES_vcalc[idPipeLine][idSaida],DES_residuos[idPipeLine][idSaida]);
                    emit signal_DES_SetStatus(DES_Adj.iteracoes,&DES_somaSSE.at(idPipeLine),&DES_vcalc.at(idPipeLine),&DES_residuos.at(idPipeLine),&crBest); //Este tem que ser feito numa conex�o direta
                    if(DES_isEquacaoEscrita.loadAcquire()) {emit signal_DES_EscreveEquacao();DES_isEquacaoEscrita.storeRelaxed(0);}
                    if(DES_isStatusSetado.loadAcquire()) {emit signal_DES_Desenha();DES_isStatusSetado.storeRelaxed(0);}//Este pode ser feito numa conexao livre.
                    isPrint = false;
                }
                ////////////////////////////////////////////////////////////////////////////
                lock_DES_index[idPipeLine].lockForWrite();
                DES_idParadaJust[idPipeLine] = DES_idParadaJust[idPipeLine]?false:true;//Garante que a mesma thread n�o passe duas vezes pela mesma chamada
                for(idSaida=0;idSaida<qtSaidas;idSaida++) DES_somaSSE[idPipeLine][idSaida]=0.0f;
                DES_index[idPipeLine] = 0;
                lock_DES_index[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
            }
        }
        idPipeLine = idPipeLine==0?TAMPIPELINE-1:idPipeLine-1;
        ////////////////////////////////////////////////////////////////////////////
        //Checa se tem que parar a thread, em caso positivo ela para e escreve os valores.
        lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH==2;lock_DES_modeOper_TH.unlock();
        if(isOk)
        {
            mutex.lock();
            if(DES_waitThread.tryAcquire()) waitSync.wait(&mutex);
            else
            {
                DES_waitThread.release(DES_TH_size-1);
                for(idSaida=0;idSaida<qtSaidas;idSaida++)
                {
                    DES_calAptidao(crBest[idSaida],20);
                    DES_MontaSaida(crBest[idSaida],DES_vcalc[idPipeLine][idSaida],DES_residuos[idPipeLine][idSaida]);
                }
                emit signal_DES_SetStatus(DES_Adj.iteracoes,&DES_somaSSE.at(idPipeLine),&DES_vcalc.at(idPipeLine),&DES_residuos.at(idPipeLine),&crBest); //Este tem que ser feito numa conex�o direta
                emit signal_DES_Finalizar();
                emit signal_DES_Parado();
                waitSync.wait(&mutex);
            }
            mutex.unlock();
        }        
        ////////////////////////////////////////////////////////////////////////////
        //Checa se tem que finalizar a tarefa.
        lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
        if(isOk)
            return;
        ////////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const Cromossomo DEStruct::DES_criaCromossomo(const qint32 &idSaida) const
{
    Cromossomo cr;
    QVector<compTermo> vetTermo;
    MTRand RG(QTime::currentTime().msec());
    qint32 tamCrom, tamRegress, i;
    compTermo vlrTermo;
    //valores - Matriz onde Linha � as variaveis (sendo a linha 0 a variavel de saida) e coluna os atrasos.
    const quint32 numVariaveis = DES_Adj.Dados.variaveis.valores.numLinhas(),
                  numAtrasos = (DES_Adj.Dados.variaveis.valores.numColunas())/2,
                  vlrMaxAtras= numAtrasos < 30 ? numAtrasos:30;
    cr.idSaida = idSaida;
   //Inicializa os coeficientes constantes.
    vlrTermo.vTermo.tTermo1.atraso = 0;//Apesar de n�o ter variavel � interessante que ela seja diferente de 0.
    vlrTermo.vTermo.tTermo1.nd = 1;    //Indica que � do numerador
    vlrTermo.vTermo.tTermo1.reg = 0;   //O regressor 0 � indicando o coeficiente constante
    vlrTermo.vTermo.tTermo1.var = 1;   //Apesar de n�o ter variavel � interessante que ela seja diferente de 0.
    vlrTermo.expoente = 1;
    vetTermo.append(vlrTermo);
    cr.regress.append(vetTermo);
    vetTermo.clear();
   //Inicializa os coeficientes normais.
    tamCrom = RG.randInt(3); //Come�a com uma quantidade de termos de no maximo 4 elementos.
    while(tamCrom>=0)
    {
        //Gera se vai ser numerador (1) ou denominador (0)
        vlrTermo.vTermo.tTermo1.nd = DES_Adj.isRacional?RG.randInt(0,1):1;
        vlrTermo.vTermo.tTermo1.reg = RG.randInt(1,(MASKREG/2)-1);//O regressor 0 � indicando o coeficiente constante
        tamRegress = RG.randInt(3); //Come�a com uma quantidade de regressores de no maximo 4 elementos.
        while(tamRegress>=0)
        {
            vlrTermo.vTermo.tTermo1.var = RG.randInt(1,numVariaveis);//Escolhe uma variavel ate o tamanho m�ximo de variaveis do sistema.
            vlrTermo.vTermo.tTermo1.atraso = RG.randInt(1,vlrMaxAtras);
            if(static_cast<qint32>(vlrTermo.vTermo.tTermo1.atraso)>cr.maiorAtraso) cr.maiorAtraso = vlrTermo.vTermo.tTermo1.atraso;
            vlrTermo.expoente = (qreal) RG.randInt(1,10);
            if(!vlrTermo.expoente) vlrTermo.expoente=1;//Elimina a chance de gerar um expoente inicial igual a zero.
            vetTermo.append(vlrTermo);
            tamRegress--;
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Ordena os termos por ordem decrescente.
        qSort(vetTermo.begin(),vetTermo.end(),CmpMaiorTerm);
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Concatena termos exatamente iguais.
        for(i=1;i<vetTermo.size();i++)
            if(vetTermo.at(i).vTermo.tTermo0 == vetTermo.at(i-1).vTermo.tTermo0) vetTermo.remove(i--);
        ///////////////////////////////////////////////////////////////////////////////////////////////
        cr.regress.append(vetTermo);
        vetTermo.clear();
        tamCrom--;
    }
    cr.err.fill(-1,cr.regress.size());
    //Calcula o melhor coeficiente, encontra o erro e calcula a Aptid�o pelo BIC.
    DES_calAptidao(cr);
    return cr;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_CruzMut(Cromossomo &crAvali,  const Cromossomo &cr0, const Cromossomo &crNew, const Cromossomo &cr1, const Cromossomo &cr2) const
{
    ////////////////////////////////////////////////////////////////////////////////
    //JMathVar<qreal> vlrsMedido,vlrsRegress;
    Cromossomo crAvali0,crA1;//,crA2;
    lock_DES_BufferSR.lockForRead();
    crAvali0=crAvali;
    lock_DES_BufferSR.unlock();
    ////////////////////////////////////////////////////////////////////////////////
    MTRand RG(QTime::currentTime().msec());
    const qreal multBase = RG.randReal(-2,2);
    QVector<compTermo> termosAnalisados,vetTermo1,vetTermo2,vetTermo3,termoAv(6,compTermo());
    QVector<QVector<compTermo> > matTermo;
    QVector<qint32> posTermosAnalisados;
    compTermo *tr,auxTermo;
    qreal expo;
    qint32 i,j,size1,size2,size3,count=0,countTermos=0,testeSize=0,teste,*pr;
    teste = RG.randInt(0,1);if(teste&1) count++;
    for(testeSize=1;count<6;testeSize++){teste=(teste<<1)+RG.randInt(0,1);if(teste&1)count++;}
    ////////////////////////////////////////////////////////////////////////////////
    for(i=0;i<cr0.regress.size();i++) {termosAnalisados+=cr0.regress.at(i);posTermosAnalisados+=QVector<qint32>(cr0.regress.at(i).size(),0);}
    //for(i=0;i<cr0.regressResid.size();i++) {termosAnalisados+=cr0.regressResid.at(i);posTermosAnalisados+=QVector<qint32>(cr0.regressResid.at(i).size(),0);}
    for(i=0;i<crNew.regress.size();i++) {termosAnalisados+=crNew.regress.at(i);posTermosAnalisados+=QVector<qint32>(crNew.regress.at(i).size(),1);}
    //for(i=0;i<crNew.regressResid.size();i++) {termosAnalisados+=crNew.regressResid.at(i);posTermosAnalisados+=QVector<qint32>(crNew.regressResid.at(i).size(),1);}
    for(i=0;i<cr1.regress.size();i++) {termosAnalisados+=cr1.regress.at(i);posTermosAnalisados+=QVector<qint32>(cr1.regress.at(i).size(),2);}
    //for(i=0;i<cr1.regressResid.size();i++) {termosAnalisados+=cr1.regressResid.at(i);posTermosAnalisados+=QVector<qint32>(cr1.regressResid.at(i).size(),2);}
    for(i=0;i<cr2.regress.size();i++) {termosAnalisados+=cr2.regress.at(i);posTermosAnalisados+=QVector<qint32>(cr2.regress.at(i).size(),3);}
    //for(i=0;i<cr2.regressResid.size();i++) {termosAnalisados+=cr2.regressResid.at(i);posTermosAnalisados+=QVector<qint32>(cr2.regressResid.at(i).size(),3);}
    for(i=0;i<crAvali0.regress.size();i++) {termosAnalisados+=crAvali0.regress.at(i);posTermosAnalisados+=QVector<qint32>(crAvali0.regress.at(i).size(),4);}
    //for(i=0;i<crAvali0.regressResid.size();i++) {termosAnalisados+=crAvali0.regressResid.at(i);posTermosAnalisados+=QVector<qint32>(crAvali0.regressResid.at(i).size(),4);}
    ////////////////////////////////////////////////////////////////////////////////
    qSortDuplo(termosAnalisados.begin(),termosAnalisados.end(),posTermosAnalisados.begin(),posTermosAnalisados.end(),CmpMaiorTerm);
    ////////////////////////////////////////////////////////////////////////////////
    termoAv[5] = termosAnalisados.at(0);
    termoAv[posTermosAnalisados.at(0)] = termosAnalisados.at(0);
    ////////////////////////////////////////////////////////////////////////////////
    for(tr=termosAnalisados.begin()+1,pr=posTermosAnalisados.begin()+1;(tr<=termosAnalisados.end());tr++,pr++)
    {
        ////////////////////////////////////////////////////////////////////////////////
        if(tr<termosAnalisados.end() ? termoAv.at(5).vTermo.tTermo0 != tr->vTermo.tTermo0 : true) //Para os termos iguais
        {
            ////////////////////////////////////////////////////////////////////////////////
            if(termoAv.at(0).vTermo.tTermo0||termoAv.at(1).vTermo.tTermo0||termoAv.at(2).vTermo.tTermo0||termoAv.at(3).vTermo.tTermo0)
            {
                if(termoAv.at(0).vTermo.tTermo0) auxTermo.vTermo.tTermo0 =  termoAv.at(0).vTermo.tTermo0;
                if(termoAv.at(1).vTermo.tTermo0) auxTermo.vTermo.tTermo0 =  termoAv.at(1).vTermo.tTermo0;
                if(termoAv.at(2).vTermo.tTermo0) auxTermo.vTermo.tTermo0 =  termoAv.at(2).vTermo.tTermo0;
                if(termoAv.at(3).vTermo.tTermo0) auxTermo.vTermo.tTermo0 =  termoAv.at(3).vTermo.tTermo0;
                auxTermo.expoente = termoAv.at(0).expoente+ multBase*(termoAv.at(1).expoente-termoAv.at(0).expoente)+multBase*(termoAv.at(2).expoente-termoAv.at(3).expoente);
                teste = (teste>>1)|((teste&1)<<testeSize);//Rotaciona os bits.
                if((teste>>i)&3)
                {
                    expo = (qint32) auxTermo.expoente;
                    expo +=((auxTermo.expoente-expo)>=0.5)?1:(((auxTermo.expoente-expo)<=-0.5)?-1:0);
                    if(((teste>>i)&3)==1) auxTermo.expoente = fabs(expo);
                }
                if(auxTermo.expoente)
                    vetTermo1.append(auxTermo);
                termoAv[0].vTermo.tTermo0 = 0;termoAv[0].expoente = 0.0f;
                termoAv[1].vTermo.tTermo0 = 0;termoAv[1].expoente = 0.0f;
                termoAv[2].vTermo.tTermo0 = 0;termoAv[2].expoente = 0.0f;
                termoAv[3].vTermo.tTermo0 = 0;termoAv[3].expoente = 0.0f;
            }
            ////////////////////////////////////////////////////////////////////////////////
            if(termoAv.at(4).vTermo.tTermo0)
            {
                vetTermo2.append(termoAv.at(4));
                termoAv[4].vTermo.tTermo0 = 0;termoAv[4].expoente = 0.0f;
            }
            ////////////////////////////////////////////////////////////////////////////////
        }
        ////////////////////////////////////////////////////////////////////////////////
        if(tr<termosAnalisados.end() ? termoAv.at(5).vTermo.tTermo2.idReg!= tr->vTermo.tTermo2.idReg:true)
        {
            countTermos = 0;
            size1 = vetTermo1.size();
            size2 = vetTermo2.size();
            size3 = size1+size2;
            if(size1||size2)
            {
                vetTermo3.clear();
                teste = (teste>>1)|((teste&1)<<testeSize);//Rotaciona os bits.
                for(i=0;i<size1;i++) if((teste>>i)&1) vetTermo3.append(vetTermo1.at(i));
                crA1.regress.append(vetTermo3);

                if(vetTermo3.size()?vetTermo3.at(0).vTermo.tTermo1.reg:true)
                {
                    vetTermo3.clear();
                    teste = (teste>>1)|((teste&1)<<testeSize);//Rotaciona os bits.
                    for(i=0;i<size2;i++) if((teste>>i)&1) vetTermo3.append(vetTermo2.at(i));
                    crA1.regress.append(vetTermo3);

                    if(vetTermo3.size()?vetTermo3.at(0).vTermo.tTermo1.reg:true)
                    {
                        vetTermo3.clear();
                        teste = (teste>>1)|((teste&1)<<testeSize);//Rotaciona os bits.
                        vetTermo1+=vetTermo2;
                        for(i=0;i<size3;i++) if((teste>>i)&1) vetTermo3.append(vetTermo1.at(i));
                        qSort(vetTermo3.begin(),vetTermo3.end(),CmpMaiorTerm);//Ordena os termos por ordem decrescente.
                        for(i=1;i<vetTermo3.size();i++)//Concatena termos exatamente iguais.
                            if(vetTermo3.at(i).vTermo.tTermo0 == vetTermo3.at(i-1).vTermo.tTermo0) vetTermo3.remove(i--);
                        crA1.regress.append(vetTermo3);
                    }
                }
                vetTermo1.clear();
                vetTermo2.clear();
            }
        }
        ////////////////////////////////////////////////////////////////////////////////
        if(tr<termosAnalisados.end())
        {
            termoAv[5] = *tr;
            if(termoAv.at(*pr).vTermo.tTermo0==tr->vTermo.tTermo0) termoAv[*pr].expoente += tr->expoente;
            else termoAv[*pr] = *tr;
        }
        ////////////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Faz uma sele��o de quais regressores ir�o participar do teste final
    const qint32 qtdeAtrasos = (DES_Adj.Dados.variaveis.valores.numColunas()/(2*DES_Adj.decimacao.at(crA1.idSaida)))-27;
    size1 = crA1.regress.size();
    matTermo = crA1.regress;
    crA1.regress.clear();
    for(count=0,i=0;(i<size1)&&(i<=testeSize)&&(count<qtdeAtrasos);i++){
        if((teste>>i)&1) {
            crA1.regress.append(matTermo.at(i));
            count++;
        }
    }
    for(i=testeSize+1;(i<size1)&&(count<qtdeAtrasos);i++){
        if(RG.randInt(0,1)){
            crA1.regress.append(matTermo.at(i));
            count++;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    DES_CalcERR(crA1,DES_Adj.serr);
    DES_calAptidao(crA1);
    ////////////////////////////////////////////////////////////////////////////////
    size1 = crA1.regress.size();
    if(size1)
    {
        for(i=0;i<size1;i++)
        {
            const qint32 size2 = crA1.regress.at(i).size();
            for(j=0;j<size2;j++) if(crA1.regress.at(i).at(j).vTermo.tTermo1.reg) crA1.regress[i][j].vTermo.tTermo1.reg = (size1-i);
        }
        if((crA1.aptidao <= crAvali.aptidao)||(crAvali.aptidao!=crAvali.aptidao)) {lock_DES_BufferSR.lockForWrite();crAvali = crA1;lock_DES_BufferSR.unlock();}
    }
    ////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_MontaVlrs(Cromossomo &cr,JMathVar<qreal> &vlrsRegress,JMathVar<qreal> &vlrsMedido,const bool &isValidacao,const bool &isLinearCoef) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JStrSet jst;
    qint32 i=0,j=0,countRegress=0,variavel=0,atraso=0;
    qreal expo=0.;
    JMathVar<qreal> matAux;
    //////////////////////////////////////////////////////////////////////////////////
    for(cr.maiorAtraso=0,i=0;i<cr.regress.size();i++)
    {
        if(cr.regress.at(i).size())
        {
            for(j=0;j<cr.regress.at(i).size();j++)
                if(static_cast<qint32>(cr.regress.at(i).at(j).vTermo.tTermo1.atraso)>cr.maiorAtraso)
                    cr.maiorAtraso=cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
        }
        else {cr.regress.remove(i);i--;}
    }
    //////////////////////////////////////////////////////////////////////////////////
    const qint32 qtdeAtrasos = (DES_Adj.Dados.variaveis.valores.numColunas())/((isValidacao?1:2)*DES_Adj.decimacao.at(cr.idSaida)),
                 posIniAtrasos = cr.maiorAtraso*DES_Adj.decimacao.at(cr.idSaida),
                 tam = qtdeAtrasos-cr.maiorAtraso;
    //////////////////////////////////////////////////////////////////////////////////
    //Monta a matrix valores dos regressores
    vlrsMedido.replace(DES_Adj.Dados.variaveis.valores,jst.set("(:,0)=(0,%1:%2:%3)'").argInt(posIniAtrasos).argInt(DES_Adj.decimacao.at(cr.idSaida)).argInt(posIniAtrasos+tam*DES_Adj.decimacao.at(cr.idSaida)));
    for(countRegress=0;(countRegress<cr.regress.size())&&(countRegress<(tam-2));countRegress++) //Varre todos os termos para aquele cromossomo
    {
        for(i=0;i<cr.regress.at(countRegress).size();i++)
        {
            variavel = cr.regress.at(countRegress).at(i).vTermo.tTermo1.var;  //Obtem a variavel
            if(!DES_Adj.isTipoExpo) expo = cr.regress.at(countRegress).at(i).expoente; //Obtem o expoente deste termo (Grau da N�o-Linearidade)
            else //Obtem expoente Inteiros ou Naturais
            {
                expo = (qint32) cr.regress.at(countRegress).at(i).expoente; //Obtem o expoente deste termo (Grau da N�o-Linearidade)
                expo +=((cr.regress.at(countRegress).at(i).expoente-expo)>=0.5)?1:(((cr.regress.at(countRegress).at(i).expoente-expo)<=-0.5)?-1:0);
                if(DES_Adj.isTipoExpo==2) expo = fabs(expo); //Obtem um expoente natural
            }
            if((expo!=0.)||(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg))
            {
                atraso = cr.regress.at(countRegress).at(i).vTermo.tTermo1.atraso; //Obtem o atraso deste regressor
                if(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg?matAux.fill(1,tam,1):matAux.replace(DES_Adj.Dados.variaveis.valores,jst.set("(:,:)=(%1,%2:%3:%4)'^%f1").argInt(variavel-1).argInt(posIniAtrasos-atraso*DES_Adj.decimacao.at(cr.idSaida)).argInt(DES_Adj.decimacao.at(cr.idSaida)).argInt(posIniAtrasos+(tam-atraso)*DES_Adj.decimacao.at(cr.idSaida)).argReal(expo))||isValidacao)
                {
                    if((i<=0)&&(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.nd)&&isLinearCoef)
                        matAux.replace(vlrsMedido,jst.set("(:,:)*=-1*(:,:)"));//Multiplica pela saida quando os valores s�o do denominador.
                    vlrsRegress.replace(matAux,jst.set((QString("(:,%1)")+QString((i==0)?"=":"*=")+QString("(:,:)")).toLatin1()).argInt(countRegress));
                }
                else cr.regress[countRegress].remove(i--);//Se o termo leva a um valor incoerente ele remove o termo.
            }
            else cr.regress[countRegress].remove(i--);//Se o termo leva a um valor incoerente ele remove o termo.
        }
        if(!cr.regress.at(countRegress).size()) cr.regress.remove(countRegress--);
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline void DEStruct::DES_CalcVlrsEstimado(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsCoefic,JMathVar<qreal> &vlrsEstimado,qint32 &tamNum,qint32 &tamDen) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JStrSet jst;
    JMathVar<qreal> vlrsRegressNum,vlrsRegressDen,vlrsCoeficNum,vlrsCoeficDen,a,b;
    qint32 i=0;
    ////////////////////////////////////////////////////////////////////////////////
    //Separa em regressores do numerador e do denominador.
    for(tamNum=0,tamDen=0,i=0;i<cr.regress.size();i++) //Varre todos os termos para aquele cromossomo
    {
        if(cr.regress.at(i).at(0).vTermo.tTermo1.nd)
        {
            vlrsCoeficNum.copy(vlrsCoefic,jst.set("(:,%1)=(:,%2)").argInt(tamNum).argInt(i));
            vlrsRegressNum.copy(vlrsRegress,jst.set("(:,%1)=(:,%2)").argInt(tamNum++).argInt(i));
        }
        else
        {
            vlrsCoeficDen.copy(vlrsCoefic,jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            vlrsRegressDen.copy(vlrsRegress,jst.set("(:,%1)=(:,%2)").argInt(tamDen++).argInt(i));
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Faz o c�lculo do valor estimado.
    vlrsEstimado = vlrsRegressNum(vlrsCoeficNum,jst.set("(:,:)*(:,:)'"));
    b = vlrsRegressDen(vlrsCoeficDen,jst.set("(:,:)*(:,:)'"));
    a.fill(1,b.numLinhas(),b.numColunas());
    a.copy(b,jst.set("(:,:)+=(:,:)"));
    vlrsEstimado.copy(a,jst.set("(:,:)/=(:,:)"));
    ////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//Calcula a taxa de redu��o do erro e seleciona apartir de uma valor serr desejado
void DEStruct::DES_CalcERR(Cromossomo &cr,const qreal &metodoSerr) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JStrSet jst;
    qint32 i=0,j=0,k=0,aux,start=0;
    qreal vlrsMedidoQuad=0.,serr=0.,u=0.;
    JMathVar<qreal> vlrsMedido,vlrsRegress,A,a,c,x,v;
    //////////////////////////////////////////////////////////////////////////////////
    //Monta a matrix valores dos regressores
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido);
    ////////////////////////////////////////////////////////////////////////////////
    if(cr.regress.size())
    {
        A = vlrsRegress;
        A.replace(vlrsMedido,jst.set("(:,%1)=(:,:)").argInt(A.numColunas()));
        const qint32 n = A.numColunas()-1;
        vlrsMedidoQuad = vlrsMedido(vlrsMedido,jst.set("(:)'*(:)")).at(0);
        c.resize(n);
        cr.err.fill(0,n);
        for(j=0;j<n;j++)
        {
            for(k=j;k<n;k++) //Ate completar o numero de termos candidatos
            {
                x = (A(A,jst.set("(%1:,%2)'*(%1:,%3)^2").argInt(j).argInt(k).argInt(n)));
                v = (A(A,jst.set("%f1*(%1:,%2)'*(%1:,%2)").argReal(vlrsMedidoQuad).argInt(j).argInt(k))); //err do regressor k;
                c(k) = x.at(0)/v.at(0);
            }
            cr.err(j) = c.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n),start,aux);
            if(aux!=j)
            {
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux)); // troca a coluna atual com a de maior err
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j],cr.regress[aux]);
            }
            v = A(jst.set("(%1:,%1)'").argInt(j));
            u=v.Norma(2);
            if(u!=0)
            {
                u = ((v.at(0)>1e-10)||(v.at(0)<-1e-10)?v.at(0):(v.at(0)>0)?1e-10:-1e-10) + sign(v.at(0))*u;
                v.replace(v,jst.set("(1:)=%f1*(1:)").argReal(1/u));
            }
            v(0)=1;
            a = A(jst.set("(%1:,%1:)").argInt(j));
            u = -2/v(v,jst.set("(:)*(:)'")).at(0);    //-2/(v'*v)
            x = a(a,jst.set("(0:,0:)=%f1*(0:,0:)").argReal(u));  //x=a*b
            x = x(v,jst.set("(:,:)'*(:,:)'"));        //x=x'*v
            x = v(x,jst.set("(:,:)'*(:,:)'"));        //x=v*x'
            a.replace(x,jst.set("(:,:)+=(:,:)"));        //a+=x
            A.replace(a,jst.set("(%1,%1)=(:,:)").argInt(j));
        }
        ////////////////////////////////////////////////////////////////////////////////
        for(j=0;j<n-1;j++)
        {
            cr.err.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n),start,aux);
            if(aux!=j)
            {
                cr.err.swap(jst.set("(%1)=(%2)").argInt(j).argInt(aux));
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux)); // pivota a coluna dos regressores com maior err
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j],cr.regress[aux]);
            }
        }
        //////////////////////////////////////////////////////////////////////////////////
        //Pega da matriz vlrsRegress apenas o que � interessante.
        a.clear();
        for(j=0,serr=0.0;(j<cr.err.size())&&((serr<metodoSerr)||(cr.err.at(j)>0.001))&&(cr.err.at(j)>0.0009)&&(cr.err.at(j)==cr.err.at(j));j++)
            serr+=cr.err.at(j);
        vlrsRegress = vlrsRegress(jst.set("(:,0:1:%1)").argInt(j));
        cr.err = cr.err(jst.set("(:,0:1:%1)").argInt(j));
        for(;j<cr.regress.size();) cr.regress.remove(j);
        //////////////////////////////////////////////////////////////////////////////////
        //Verifica quem � o maior atraso e o atualiza.
        for(cr.maiorAtraso=0,i=0;i<cr.regress.size();i++)
            for(j=0;j<cr.regress.at(i).size();j++)
                if(static_cast<qint32>(cr.regress.at(i).at(j).vTermo.tTermo1.atraso)>cr.maiorAtraso)
                    cr.maiorAtraso=cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
        ////////////////////////////////////////////////////////////////////////////////                               
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_calAptidao(Cromossomo &cr,const qint32 &tamTestErro) const
{
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    bool isOk1,isOk2,isOk=true;
    JStrSet jst;
    qreal var=0,var1=0,erroDepois=9e99;
    //MTRand RG(QTime::currentTime().msec());
    QVector<QVector<compTermo > > regressNum, regressDen;
    JMathVar<qreal> vlrsRegress,vlrsRegress1,vlrsRegressNum,vlrsRegressDen,vlrsRegressDenAux,vlrsCoefic,vlrsCoefic1,
                    vlrsEstimado,vlrsResiduo,vlrsMedido,
                    errNum,errDen,auxDen,
                    sigma1,sigma2,aux1,aux2,x,v;
    qint32 i,tamNum=0,tamDen=0,count1=0,count2=0,size=0;
    //////////////////////////////////////////////////////////////////////////////////
    cr.erro      = 9e99;
    cr.aptidao   = 9e99;
    errNum.remove('C',0);
    errDen.remove('C',0);
    //////////////////////////////////////////////////////////////////////////////////
    //Monta a matrix valores dos regressores
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,false,false);
    const qint32 qtdeAtrasos = vlrsMedido.numLinhas();
    for(i=0;i<cr.regress.size();i++) size+=cr.regress.at(i).size();
    ////////////////////////////////////////////////////////////////////////////////
    //Separa em regressores do numerador e do denominador.
    for(tamNum=0,tamDen=0,i=0;i<cr.regress.size();i++) //Varre todos os termos para aquele cromossomo
    {
        if(cr.regress.at(i).at(0).vTermo.tTermo1.nd)
        {
            regressNum.append(cr.regress.at(i));
            errNum.append('C',cr.err.at(i));
            vlrsRegressNum.copy(vlrsRegress,jst.set("(:,%1)=(:,%2)").argInt(tamNum++).argInt(i));
        }
        else
        {
            regressDen.append(cr.regress.at(i));
            errDen.append('C',cr.err.at(i));
            vlrsRegressDen.copy(vlrsRegress,jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            auxDen.copy(vlrsRegress,jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            auxDen.replace(vlrsMedido,jst.set("(:,%1)*=-1*(:,:)").argInt(tamDen++));//Multiplica pela saida e -1 pseudolineariza��o.
        }
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Se n�o tiver nada no Numerador coloca pelo menos uma constante.
    if(!tamNum)
    {
        tamNum++;
        compTermo vlrTermo;
        vlrTermo.vTermo.tTermo1.atraso = 0;//Apesar de n�o ter variavel � interessante que ela seja diferente de 0.
        vlrTermo.vTermo.tTermo1.nd = 1;    //Indica que � do numerador
        vlrTermo.vTermo.tTermo1.reg = 0;   //O regressor 0 � indicando o coeficiente constante
        vlrTermo.vTermo.tTermo1.var = 1;   //Apesar de n�o ter variavel � interessante que ela seja diferente de 0.
        vlrTermo.expoente = 1;
        QVector<compTermo> vetTermo;
        vetTermo.append(vlrTermo);
        regressNum.prepend(vetTermo);
        errNum.prepend('C',-1);
        vlrsRegressNum.fill(1,qtdeAtrasos,1);
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Monta o cr.regress agora em ordem numerador + denominador.
    cr.regress.clear();cr.regress += regressNum;cr.regress += regressDen;
    cr.err.clear();cr.err.append('c',errNum);cr.err.append('c',errDen);
    cr.err.setNumColunas(errNum.numColunas()+errDen.numColunas());
    cr.vlrsCoefic.fill(0,tamNum+tamDen);
    vlrsRegress = vlrsRegressNum;
    vlrsRegress1= vlrsRegressNum;
    if((vlrsRegressDen.numLinhas()==qtdeAtrasos)||(vlrsRegress.numLinhas()==1))
    {
        vlrsRegress1.copy(vlrsRegressDen,jst.set("(:,%1)=(:,:)").argInt(tamNum));//Valores sem estar multiplicados pela saida e sem o -1.
        vlrsRegress.copy(auxDen,jst.set("(:,%1)=(:,:)").argInt(tamNum));//Valores multiplicados por -1 e a saida.
        vlrsRegressDenAux.replace(auxDen,jst.set("(:,:)=-1*(:,:)"));//Valores multiplicados pela saida sem o -1.
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Faz quando � Racional.
    if(vlrsRegressDenAux.numLinhas()==qtdeAtrasos)
    {
        ////////////////////////////////////////////////////////////////////////////////
        //Calcula o sigma1
        aux1 = vlrsRegressDenAux(vlrsRegressDenAux,jst.set("(:,:)'*(:,:)"));
        sigma1.copy(aux1,jst.set("(%1,%1)=(:,:)").argInt(tamNum));
        ////////////////////////////////////////////////////////////////////////////////
        //Calcula o sigma2
        aux2.resize(vlrsRegressDenAux.numColunas(),1);
        for(i=0;i<vlrsRegressDenAux.numColunas();i++)
            aux2(i,0)=-1*sum(vlrsRegressDenAux(jst.set("(:,%1)'").argInt(i)));
        sigma2.copy(aux2,jst.set("(%1,:)=(:,:)").argInt(tamNum));
        ////////////////////////////////////////////////////////////////////////////////
    }
    else //Faz quando � Polinomial.
    {
        sigma1.fill(0,vlrsRegress.numColunas(),vlrsRegress.numColunas());
        sigma2.fill(0,vlrsRegress.numColunas(),1);
    }
    ////////////////////////////////////////////////////////////////////////////////
    //Inicializa o count1
    count1=0;
    do
    {
        ////////////////////////////////////////////////////////////////////////////////
        //Atualiza o erro, a aptid�o e os vlrsCoefic.
        if(count1)
        {
            cr.vlrsCoefic = vlrsCoefic; //Atualiza os coeficientes.
            cr.erro       = erroDepois;
            cr.aptidao    = DES_Adj.Dados.variaveis.valores.numColunas()*qLn(cr.erro) + (2*size+cr.regress.size())*qLn(DES_Adj.Dados.variaveis.valores.numColunas());
        }
        ////////////////////////////////////////////////////////////////////////////////
        //Insere os termos do residuo no sistema
        for(i=0;(i<count1)&&(vlrsResiduo.numLinhas()>1);i++)
        {
            vlrsResiduo.prepend('L',0);
            vlrsResiduo.remove('L',vlrsResiduo.numLinhas()-1); //e(k-(i+1))
            vlrsRegress.copy(vlrsResiduo,jst.set("(:,%1)=(:,:)").argInt(tamNum+tamDen+i));  //Insere termos do residuo
        }
        ////////////////////////////////////////////////////////////////////////////////
        //Inicializa o count2
        count2 = 0;
        do
        {
            ////////////////////////////////////////////////////////////////////////////////
            if(count2)
            {
                var        = var1;
                vlrsCoefic = vlrsCoefic1; //Atualiza os coeficientes.
            }
            ////////////////////////////////////////////////////////////////////////////////
            //Calcula vlrsCoefic por [A'*A-COV(e)sigma1]*x = [A'*b-COV(e)*sigma2] -> M�todo dos m�nimos Quadrados estendido
            v = vlrsRegress(vlrsRegress,jst.set("(:,:)'*(:,:)"));   //A'*A
            v.copy(sigma1,jst.set("(:,:)-=%f1*(:,:)").argReal(var));//v-var*sigma1
            x = vlrsRegress(vlrsMedido,jst.set("(:,:)'*(:,:)"));    //A'*b
            x.copy(sigma2,jst.set("(:,:)-=%f1*(:,:)").argReal(var));//x-var*sigma2
            vlrsCoefic1 = v.SistemaLinear(x,isOk);                  //Calcula os coeficientes V*vlrsCoefic=X.
            ////////////////////////////////////////////////////////////////////////////////
            if(isOk)
            {
                DES_CalcVlrsEstimado(cr,vlrsRegress1,vlrsCoefic1,vlrsEstimado,tamNum,tamDen);
                vlrsResiduo  = vlrsMedido(vlrsEstimado,jst.set("(:)-(:)"));   //residuo = medido - estimado :Faz o c�lculo dos residuos                ////////////////////////////////////////////////////////////////////////////////
                var1 = cov(vlrsResiduo);
                ////////////////////////////////////////////////////////////////////////////////
                isOk1 = compara(vlrsCoefic,vlrsCoefic1,1e-3);
                isOk2 = (var-var1)==0?true:(var-var1)>0?(var-var1)<1e-3:(var-var1)>-1e-3;
                count2++;
                ////////////////////////////////////////////////////////////////////////////////
            }
        }
        while(isOk&&(!(isOk1&&isOk2))&&count2<=20);
        erroDepois = vlrsResiduo(vlrsResiduo,jst.set("(:,:)'*(:,:)")).at(0)/qtdeAtrasos; //r'*r :Faz o c�lculo do erro quadr�tico m�dio.
        count1++;//Incrementa a variavel do tamanho do erro.
    }
    while(isOk&&(count1<=tamTestErro)&&(erroDepois<cr.erro));
    ////////////////////////////////////////////////////////////////////////////////
    //Elimina regressores com valor de coeficiente espurio. dentro da faixa de +-1e-5 ou acima da faixa +- 1e+5.
    for(isOk=false,i=0;i<cr.vlrsCoefic.size();i++) //Varre todos os termos para aquele cromossomo
    {
        if(((cr.vlrsCoefic.at(i)!=0.0)&&(i<cr.regress.size()))?((cr.vlrsCoefic.at(i)<=1e-5)&&(cr.vlrsCoefic.at(i)>=-1e-5))||(cr.vlrsCoefic.at(i)>=1e+5)||(cr.vlrsCoefic.at(i)<=-1e+5):false) {cr.regress.remove(i);cr.err.remove('C',i);isOk=true;}
    }
    if(isOk) DES_calAptidao(cr,tamTestErro);   
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_MontaSaida(const Cromossomo &crPrinc,QVector<qreal> &vplotar,QVector<qreal> &resid) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JStrSet jst;
    Cromossomo cr = crPrinc;
    JMathVar<qreal> vlrsRegress,vlrsEstimado,vlrsResiduo,vlrsMedido;
    qint32 i=0,atraso=0,tamNum=0,tamDen=0;
    qreal *estimado,*medido,*residuo;
    //////////////////////////////////////////////////////////////////////////////////
    //Monta a matrix valores dos regressores
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true,false);
    DES_CalcVlrsEstimado(cr,vlrsRegress,cr.vlrsCoefic,vlrsEstimado,tamNum,tamDen);
    //Insere os termos do residuo no sistema se houver
    const qint32 tamErro = cr.vlrsCoefic.numColunas()-(tamNum+tamDen);
    if(tamErro)
    {
        vlrsResiduo.fill(0,vlrsMedido.numLinhas(),1);
        ////////////////////////////////////////////////////////////////////////////////
        //Calcula os valores estimados dos residuos com os regressores do residuo.
        const qint32 tamvlrsRegress = (tamNum+tamDen);
        for(atraso=0,estimado=vlrsEstimado.begin(),residuo=vlrsResiduo.begin(),medido=vlrsMedido.begin();medido < vlrsMedido.end();medido++,residuo++,estimado++,atraso++)
        {
            for(i=0;i<tamErro;i++)
                *estimado += cr.vlrsCoefic.at(tamvlrsRegress+i)*((atraso-i)>=0?*(residuo-i):0);
            *residuo = *medido - *estimado;
        }
    }
    else vlrsResiduo = vlrsMedido(vlrsEstimado,jst.set("(:)-(:)"));
    ////////////////////////////////////////////////////////////////////////////////
    //Prenche os vetores de saida.
    vplotar.clear(); vplotar += (QVector<qreal> ) vlrsEstimado;
    resid.clear(); resid += (QVector<qreal> ) vlrsResiduo;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline void DEStruct::DES_isOk(const Cromossomo &cr,const QString &str1) const
{
    (void)str1; (void)cr;  // Stub function - interface not compatible
    /*
    if(!cr.termos.size())
    {
        qDebug() << str1+ " cr.termos.size()==0";
        return;
    }
    //valores - Matriz onde Linha � as variaveis (sendo a linha 0 a variavel de saida) e coluna os atrasos.
    const compTermo  *vlrTermo;
    qint32 i=0,maiorAtraso=0;
    //Analisa se a inser��o dos regressores esta correta.
    for(vlrTermo = cr.termos.begin(); vlrTermo < cr.termos.end(); vlrTermo++,i++)
    {
        if(!vlrTermo->vTermo.tTermo1.atraso)
        {
           qDebug() << str1+QString(" Atraso do item %1 igual a zero").arg(i);
           return;
        }
        if(vlrTermo->vTermo.tTermo1.atraso>maiorAtraso) maiorAtraso = vlrTermo->vTermo.tTermo1.atraso;
        if(!vlrTermo->vTermo.tTermo0||!vlrTermo->coefic||!vlrTermo->expoente)
        {
            if(i?vlrTermo->vTermo.tTermo3.idTermo == (vlrTermo-1)->vTermo.tTermo3.idTermo:false) //Concatena Termos iguais
            qDebug() << str1+QString(" Elemento em zero index: %1").arg(i);
            return;
        }
    }
    if((maiorAtraso!=cr.maiorAtraso)&&(maiorAtraso!=(cr.maiorAtraso-1)))
        qDebug() << str1+QString(" Maior atraso tem que ser: %1").arg(maiorAtraso);*/
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_SuperResp(Cromossomo &crResult,QVector<Cromossomo> &vetorResult) const
{
    (void)crResult; (void)vetorResult;  // Stub function
    /*
    MTRand RG;
    Cromossomo cr0(crResult);
    ////////////////////////////////////////////////////////////////////////////////
    qreal auxValor,auxPasso,auxReal;
    const qreal stepReal=RG.randReal()*0.5;
    const qint32 stepInt=RG.randInt(0,3);
    bool isOk,isSum=false;
    qint32 i,countVetResult=0,auxInt,count;
    ////////////////////////////////////////////////////////////////////////////////
    JMathVar<qreal> matCoefic,matCoeficAux;
    XVetor<qreal> vetResult((3*cr0.termos.size()+3),0.0f),
                  vetPasso,
                  vetErro;//Cada elemento que sera regredido.
    QVector<bool> vetIsSum;
    ////////////////////////////////////////////////////////////////////////////////
    vetResult[0]=1.0;//Referente aos termos contantes do meta modelo
    vetPasso.append(0);
    vetIsSum.append(false);
    ////////////////////////////////////////////////////////////////////////////////
    //For�a o ponto central colocando ele 4 vezes na matriz.
    for(i=0;i<4;i++) {matCoefic.append('c',vetResult);vetErro.append(cr0.aptidao);}
    ////////////////////////////////////////////////////////////////////////////////
    cr0.coeficNum+=stepReal;
    DES_calAptidao(cr0);
    vetIsSum.append(cr0.aptidao<crResult.aptidao);
    vetResult[1]=1;
    matCoefic.append('c',vetResult);
    vetErro.append(cr0.aptidao);
    cr0.coeficNum-=2*stepReal;
    DES_calAptidao(cr0);
    vetResult[1]=-1;
    matCoefic.append('c',vetResult);
    vetErro.append(cr0.aptidao);
    cr0.coeficNum+=stepReal;
    vetResult[1]=0;
    vetPasso.append(stepReal);
    ////////////////////////////////////////////////////////////////////////////////
    cr0.coeficDen+=stepReal;
    DES_calAptidao(cr0);
    vetIsSum.append(cr0.aptidao<crResult.aptidao);
    vetResult[2]=1;
    matCoefic.append('c',vetResult);
    vetErro.append(cr0.aptidao);
    cr0.coeficDen-=2*stepReal;
    DES_calAptidao(cr0);
    vetResult[2]=-1;
    matCoefic.append('c',vetResult);
    vetErro.append(cr0.aptidao);
    cr0.coeficDen+=stepReal;
    vetResult[2]=0;
    vetPasso.append(stepReal);
    ////////////////////////////////////////////////////////////////////////////////
    for(i=0;i<cr0.termos.size();i++)
    {
        ////////////////////////////////////////////////////////////////////////////////
        if(((cr0.termos[i].vTermo.tTermo1.atraso+stepInt)<30)&&((cr0.termos[i].vTermo.tTermo1.atraso-stepInt)>0))
        {
            cr0.termos[i].vTermo.tTermo1.atraso+=stepInt;
            DES_calAptidao(cr0);
            vetIsSum.append(cr0.aptidao<crResult.aptidao);
            vetResult[3*i+3]=1;
            matCoefic.append('c',vetResult);
            vetErro.append(cr0.aptidao);

            cr0.termos[i].vTermo.tTermo1.atraso-=2*stepInt;
            DES_calAptidao(cr0);
            vetResult[3*i+3]=-1;
            matCoefic.append('c',vetResult);
            vetErro.append(cr0.aptidao);

            vetResult[3*i+3]=0;
        }
        else
        {
            vetResult[3*i+3]=0;
            matCoefic.append('c',vetResult);
            vetErro.append(crResult.aptidao);
            matCoefic.append('c',vetResult);
            vetErro.append(crResult.aptidao);
        }

        vetPasso.append(stepInt);
        ////////////////////////////////////////////////////////////////////////////////
        cr0.termos[i].coefic+=stepReal;
        vetIsSum.append(cr0.aptidao<crResult.aptidao);
        DES_calAptidao(cr0);
        vetResult[3*i+4]=1;
        matCoefic.append('c',vetResult);
        vetErro.append(cr0.aptidao);
        cr0.termos[i].coefic-=2*stepReal;
        DES_calAptidao(cr0);
        vetResult[3*i+4]=-1;
        matCoefic.append('c',vetResult);
        vetErro.append(cr0.aptidao);
        cr0.termos[i].coefic+=stepReal;
        vetResult[3*i+4]=0;
        vetPasso.append(stepReal);
        ////////////////////////////////////////////////////////////////////////////////
        cr0.termos[i].expoente+=stepReal;
        vetIsSum.append(cr0.aptidao<crResult.aptidao);
        DES_calAptidao(cr0);
        vetResult[3*i+5]=1;
        matCoefic.append('c',vetResult);
        vetErro.append(cr0.aptidao);
        cr0.termos[i].expoente-=2*stepReal;
        DES_calAptidao(cr0);
        vetResult[3*i+5]=-1;
        matCoefic.append('c',vetResult);
        vetErro.append(cr0.aptidao);
        cr0.termos[i].expoente+=stepReal;
        vetResult[3*i+5]=0;
        vetPasso.append(stepReal);
        ////////////////////////////////////////////////////////////////////////////////
    }
    //Calcula o vetor resultado por AT*A*x = AT*b
    matCoeficAux = AoQuadTrans<qreal>(matCoefic);//AT*A : sendo matCoefic=AT
    vetResult = MultMatVet<qreal>(matCoefic,vetErro);//AT*b : sendo matCoefic=AT
    vetResult = SistemaLinear(isOk,matCoeficAux,vetResult);
    //Procura o maior coeficiente encontrado desconsiderando o termo constante.
    if(isOk)
    {
        //Procura o maior coeficente com exece��o do termo constante
        auxValor = vetResult.at(1);
        auxPasso=vetPasso.at(1);
        for(i=2;i<vetResult.size();i++) if(vetResult.at(i)>auxValor)
        {
            isSum = vetIsSum.at(i);
            auxValor=vetResult.at(i);
            auxPasso=vetPasso.at(i);
        }
        //Divide todos os coeficientes com exce��o do constante pelo maior coeficiente e multiplica pelo passo
        for(i=1;i<vetResult.size();i++) vetResult[i] = (vetResult.at(i)/auxValor)*auxPasso;
        countVetResult=0;
        do
        {
            if(countVetResult>0) {if(cr0.aptidao < crResult.aptidao) crResult=cr0;}
            ////Preenche o cromossomo decrementando este passo pois o problema visa diminuir.
            auxReal = cr0.coeficNum + (isSum?1:-1)*vetResult.at(1);
            if(auxReal==auxReal) cr0.coeficNum = auxReal;
            auxReal = cr0.coeficDen +(isSum?1:-1)*vetResult.at(2);
            if(auxReal==auxReal) cr0.coeficDen = auxReal;
            count=3;
            for(i=0;i<cr0.termos.size();i++)
            {
                auxInt = cr0.termos.at(i).vTermo.tTermo1.atraso +(isSum?1:-1)*vetResult.at(count);
                if((auxInt==auxInt)&&(auxInt>0)&&(auxInt<30))
                {
                    cr0.termos[i].vTermo.tTermo1.atraso = auxInt;
                    if(auxInt>cr0.maiorAtraso) cr0.maiorAtraso = auxInt;
                }
                auxReal = cr0.termos.at(i).coefic +(isSum?1:-1)*vetResult.at(count+1);
                if(auxReal==auxReal) cr0.termos[i].coefic = auxReal;
                auxReal = cr0.termos.at(i).expoente +(isSum?1:-1)*vetResult.at(count+2);
                if(auxReal==auxReal) cr0.termos[i].expoente = auxReal;
                count+=3;
            }
            DES_calAptidao(cr0);
            vetorResult[countVetResult%vetorResult.size()]=cr0;
            countVetResult++;
        }
        while((cr0.aptidao < crResult.aptidao)?true:countVetResult<10);
        vetorResult[0]=crResult;
    }
    else qDebug("Func: DES_SuperResp - N�o conseguiu calcular Sistema Linear");*/
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_MinimizarLevMarq(Cromossomo &cr) const
{
    (void)cr;  // Stub function
    /*
    /////////////////////Inicializa�ao das constantes////////////
    const qint32 numMaxIteracoes = 10;
    const qreal  tolerancia      = 1.0e-5f,//1.0e-8f,
                 xProximo        = 1.0e-3f;
    const qint32 qtdeAvaliada = (2*cr.termos.size())+2,
                 qtdeAtrasos = DES_Adj.Dados.variaveis.valores.numColunas();
    ////////////////////////Declaracao de variaveis/////////////
    bool isOk=false;
    Cromossomo crNovo;
    qint32 i=0,
           v = 2,
           numIteracoes = 0,
           achou = 0;
    qreal normaInfG = 1.0f,
          maxDiag   = 1.0f,
          paramAmortecimento = 1.0f,
          normaHlm = 1.0f,
          normaX = 1.0f,
          ganho = 1.0f,
          fx = 1.0f,
          fxnovo = 1.0f,
          soma = 1.0f;
    XVetor<qreal> parametro,
                  xnovo(qtdeAvaliada),
                  hlm(qtdeAvaliada),
                  //residuos - Vetor com o valor do residuo (F(X)Real-F(X)Calculado) para cada atraso
                  residuos(qtdeAtrasos);
                   //Jacobiana - Matriz Derivada cuja linha = n� de atrasos e coluna = n� de Regressores
    JMathVar<qreal> jacobiana(qtdeAtrasos,qtdeAvaliada),
                   //hessiana - Transposta(Jacobiana)*Jacobiana - linha = n� de atrasos e coluna = n� de atrasos
                   hessiana(qtdeAvaliada,qtdeAvaliada),
                   //quasiGradi - Transposta(Jacobiana) * F(X) - linha = n� de atrasos; coluna = 1
                   quasiGradi(qtdeAvaliada,1),
                   matauxiliar(qtdeAvaliada,qtdeAvaliada);
                   //Gradiente - Transposta(Jacobiana) * F(X) + Hessiana * H(x) - linha = n� de atrasos; coluna = 1
    ////////////////////////////////////////////////////////////
    //valores - Matriz onde Linha � as variaveis (sendo a linha 0 a variavel de saida) e coluna os atrasos.
    //vlrRegressores - Matriz onde linha � os regressores e coluna � os atrasos.
    ////////////////////////////////////////////////////////////
    //Adquiri o ponto inicial
    parametro.append(cr.coeficNum);
    parametro.append(cr.coeficDen);
    for(i=0;i<cr.termos.size();i++)
    {
        parametro.append(cr.termos.at(i).coefic);
        parametro.append(cr.termos.at(i).expoente);
    }
    ////////////////////////////////////////////////////////////
    //DES_calAptidao(cr,&residuos);
    fx = cr.aptidao;
    DES_CalcJacob(jacobiana,parametro,cr);
    DES_CalcHessi(hessiana, jacobiana);
    DES_CalcGrad(quasiGradi,jacobiana,residuos);
    ////////////////////////////////////////////////////////////
    //Se o maior crescimento da fun��o no ponto x for menor que e1 ent�o encontrou o m�nimo local procurado
    normaInfG = quasiGradi.NormaInf();
    if (normaInfG <= tolerancia) achou = 1;
    else
    {
        //O par�metro de amortecimento � dado baseando-se na matriz hessiana
        maxDiag = hessiana.MaiorElemDiagPrin();
        paramAmortecimento = xProximo * maxDiag;
    }
    //Procura o ponto cr�tico at� encontr�-lo ou at� atingir o n�mero m�ximo de itera��es
    while ((achou == 0) && (numIteracoes < numMaxIteracoes))
    {
        //Incrementa a itera��o
        numIteracoes++;
        //Faz uma copia da matriz hessiana para uma matriz auxiliar
        qCopy(hessiana.begin(),hessiana.end(),matauxiliar.begin());
        //A soma do par�metro de amortecimento na matriz hessiana garante que a matriz � positiva definida e seja invert�vel
        for(i = 0; i < parametro.size(); i++)
        {
                matauxiliar(i,i) = matauxiliar.at(i,i) + paramAmortecimento;
                hlm[i] = -1.0 * quasiGradi.at(i,0);
        }
        //Calcula o passo em uma dire��o m�xima de descida
        //hlm = matauxiliar.SistLinear(isOk,hlm);
        if(isOk)
        {
            normaHlm = hlm.normal();
            normaX   = parametro.normal();
            //Se a dire��o m�xima de descida for menor do que e2 * (normaX + e2) ent�o encontrou-se o ponto cr�tico procurado
            if (normaHlm <= (tolerancia * (normaX + tolerancia))) achou = 1;
            else
            {
                //Se n�o achou incrementa o passo e continua procurando
                for (i = 0; i < parametro.size(); i++) xnovo[i] = parametro.at(i) + hlm.at(i);
                //Imforma os resultados
                crNovo = cr;
                crNovo.coeficNum  = parametro.at(0);
                crNovo.coeficDen  = parametro.at(1);
                for(i=0;i<crNovo.termos.size();i+=2)
                {
                    crNovo.termos[i].coefic  = parametro.at(i+2);
                    crNovo.termos[i].expoente  = parametro.at(i+3);
                }
                //Calcula o ganho ao dar o passo
                DES_calAptidao(crNovo,&residuos);
                fxnovo = crNovo.aptidao;
                DES_CalcJacob(jacobiana,parametro,crNovo);
                DES_CalcGL(soma, hlm, quasiGradi, paramAmortecimento,parametro.size());
                ganho    = (fx - fxnovo) / (soma);
                //Se o ganho for maior do que 0 ent�o v� se encontra x ao dar o passo e diminue o par�metro de amortecimento
                if (ganho > 0)
                {
                    cr = crNovo;
                    //Adquiri o novo ponto inicial
                    parametro.clear();
                    parametro.append(cr.coeficNum);
                    parametro.append(cr.coeficDen);
                    for(i=0;i<cr.termos.size();i++)
                    {
                        parametro.append(cr.termos.at(i).coefic);
                        parametro.append(cr.termos.at(i).expoente);
                    }
                    xnovo.resize(parametro.size());
                    qCopy(xnovo.begin(),xnovo.end(),parametro.begin());
                    fx = fxnovo;
                    DES_CalcJacob(jacobiana,parametro,cr);
                    DES_CalcGrad(quasiGradi,jacobiana,residuos);
                    normaInfG = quasiGradi.NormaInf();
                    //Se o maior crescimento da fun��o no ponto x for menor que e1 ent�o encontrou o m�nimo local procurado
                    if(normaInfG <= tolerancia) achou = 1;
                    else
                    {
                        DES_CalcHessi(hessiana, jacobiana);
                        paramAmortecimento = paramAmortecimento * (MAX(1/3.0, 1 - pow(((2 * ganho) - 1), 3)));
                        v = 2;
                    }
                }
                //Se n�o for, aumenta o par�metro de amortecimento e calcula novamente.
                else{paramAmortecimento = paramAmortecimento * v;v = 2* v;}
            }
        }
    }*/
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_CalcJacob(JMathVar<qreal> &matJacob,const XVetor<qreal> &coefic,const Cromossomo &cr) const
{
    (void)matJacob; (void)coefic; (void)cr;  // Stub function
    /*
    const qint32 auxNumLinhas  = DES_Adj.Dados.variaveis.valores.numLinhas(),
                 auxNumColunas = DES_Adj.Dados.variaveis.valores.numColunas();

    qint32 linha, coluna;
    qreal auxRealNum, auxRealDenom;
    //Para a matriz jacobiana deve come�ar na linha 0.
    for(linha = 0; linha < auxNumLinhas; linha++)
    {
        auxRealNum = 0.0f;
        auxRealDenom = 0.0f;
        for(coluna = 0; coluna < auxNumColunas;coluna++)
        {
            if(cr.numDenom<cr.regressores.at(linha)) auxRealNum += coefic.at(linha+2)*cr.vlrRegressores.item(linha,coluna);
            else auxRealDenom += coefic.at(linha+2)*cr.vlrRegressores.item(linha,coluna);
        }
        if(auxRealNum==0) auxRealNum = 1.0f;
        if(auxRealDenom==0) auxRealDenom = 1.0f;
        for(coluna = 0; coluna < auxNumColunas;coluna++)
        {
            if(cr.numDenom<cr.regressores.at(linha)) matJacob(linha,coluna) = (cr.vlrRegressores.item(linha,coluna)/auxRealDenom);
            else matJacob(linha,coluna) = ((-1)*(cr.vlrRegressores.item(linha,coluna)/auxRealDenom)*(auxRealNum/auxRealDenom));
        }
    }*/
}
//////////////////////////////////////////////////////////////////////////////////////////
inline void DEStruct::DES_CalcHessi(JMathVar<qreal> &matHessi,const JMathVar<qreal> &matJacob) const
{
    //Preenche a matriz hessiana com o quadrado da matriz jacobiana.
    matHessi = AoQuadTrans(matJacob);
}
//////////////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_CalcGrad(JMathVar<qreal> &matGrad,const JMathVar<qreal> &matJacob,const XVetor<qreal> &residuos) const
{
    //Calcula a transposta da jacobiana e multiplica pela fun��o residuos
    //matGrad = matJacob.Trans()*residuos;
    //<M12, M21> = <M12, M11> * <M22,1>
    for (qint32 linha = 0; linha < matJacob.numColunas(); linha++)
    {
        qreal sum = 0.0f;
        for (int aux = 0; aux < matJacob.numLinhas(); aux++)
            sum += matJacob.at(aux,linha) * residuos.at(aux);
        matGrad(linha,0) = (sum!=sum)||(sum>1e9f) ? 1e9f: sum;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_CalcGL(qreal &res,const XVetor<qreal> &hlm,const JMathVar<qreal> &gradiente,const qreal &paramAmortecimento,const qint32 &nArgs) const
{
    qint32 i;
    res = 0.0f;
    //Calcula o ganho obtido com um modelo linear
    for (i = 0; i < nArgs; i++) res += (hlm.at(i) * (hlm.at(i) * paramAmortecimento - gradiente.at(i,0)));
    res *= (0.5);
}
//////////////////////////////////////////////////////////////////////////////////////////
