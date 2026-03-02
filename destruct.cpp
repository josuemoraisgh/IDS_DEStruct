#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSemaphore>
#include <QDateTime>
#include <QRegularExpression>
#include <math.h>
#include <qmath.h>
#include <algorithm>
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
volatile qint32 DEStruct::DES_index[TAMPIPELINE] = {0,0,0,0};
bool            DEStruct::DES_isCarregar,
                DEStruct::DES_idParadaJust[TAMPIPELINE] = {false,false,false,false};
volatile qint64 DEStruct::tamArquivo=0;
volatile qint16 DEStruct::DES_TH_size = 0,
                DEStruct::DES_countSR = 0;
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
    DEStruct::lock_DES_BufferSR.lockForRead();
    const Cromossomo cr1 = DEStruct::DES_Adj.Pop.at(idSaida).at(countCr1);
    const Cromossomo cr2 = DEStruct::DES_Adj.Pop.at(idSaida).at(countCr2);
    DEStruct::lock_DES_BufferSR.unlock();
    return(cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*void crCopy(Cromossomo &cr1,const Cromossomo &cr2)
{
    cr1.termos.resize(cr2.termos.size());
    std::copy(cr2.termos.begin(),cr2.termos.end(),cr1.termos.begin());

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
    mutex.lock();
    DES_TH_id = DES_TH_size;
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
    DES_TH_size++;
    mutex.unlock();
    DES_Adj.Dados.variaveis.nome.clear();
    DES_Adj.Dados.variaveis.valores.clear();
    DES_Adj.Dados.variaveis.Vmaior.clear();
    DES_Adj.Dados.variaveis.Vmenor.clear();
    DES_RG.seed(QTime::currentTime().msec());
    DES_isEquacaoEscrita=true;
    DES_isStatusSetado=true;
    DES_vlrRegressores = nullptr; // Ponteiro inicializado como nulo (era comentado)
    start();//Inicia thread
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DEStruct::~DEStruct()
{
    mutex.lock();
    DES_Adj.modeOper_TH = 0;
    DES_TH_size--;
    mutex.unlock();
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
    qint32 index=0,i=0,j=0,ncoluna=0,idSaida=0;//indexIni=0;
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
    mutex.lock();
    if(DES_justThread[0].tryAcquire()) justSync.wait(&mutex);
    else
    {
        DES_justThread[0].release(DES_TH_size-1);
        DES_index[0] = 0;
        if(DES_isCarregar)
        {
            DES_Adj.Dados.variaveis.nome.clear();
            DES_Adj.Dados.variaveis.valores.clear();
        }
        //else indexIni = DES_Adj.Dados.variaveis.valores.numColunas();
        DES_Adj.Dados.variaveis.Vmaior.clear();
        DES_Adj.Dados.variaveis.Vmenor.clear();
        DES_mediaY = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList();
        DES_mediaY2 = QVector<qreal>(DES_Adj.Dados.variaveis.qtSaidas,0.0f).toList();
        if (file.open(QFile::ReadOnly))
        {
            tamArquivo = file.size();
            file.close();
        }
        else qDebug() << "Func:DES_Carregar - N�o abriu arquivo para ler tamanho";
        emit signal_DES_Status(0);
        //Le a variavel sem o QReadWriteLock pois apenas uma thread esta rodando.
        if(DES_Adj.modeOper_TH==2) waitSync.wait(&mutex);
        justSync.wakeAll();
    }
    mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    //Verifica se � para fechar o programa.
    lock_DES_modeOper_TH.lockForRead();isOk=DES_Adj.modeOper_TH<=1;lock_DES_modeOper_TH.unlock();
    if(isOk) return;
    ////////////////////////////////////////////////////////////////////////////
    //Monta a divisao para cada thread
    const qint32 tamCadaTh = ((DES_TH_id+1)==DES_TH_size)?tamArquivo - (DES_TH_id*(tamArquivo/DES_TH_size)):(tamArquivo/DES_TH_size);//Tamanho para cada thread sendo a ultima diferente
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
            if(lineMeio.size()?(lineMeio.right(1)!="\n")&&(lineMeio.right(1)!="\0"):true)
            {
                line = file.read(1);
                while((line != "\n")&&(line != "\0"))
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
                while((line != "\n")&&(line != "\0"))
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
            lineList = QString(/*lineAntes+*/lineMeio+lineDepois).remove('\r').split('\n',Qt::SkipEmptyParts);
            ////////////////////////////////////////////////////////////////////////////
            for(i=0/*(DES_TH_id||index)?1:0*/;i<lineList.size();i++)
            {
                if(lineList.at(i).size())
                {
                    strList = lineList.at(i).split(QString(QChar(32)),Qt::SkipEmptyParts);
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
                                QRegularExpression rx("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*),([-+]?\\d*)\\)"); //Float com ponto opcional e o exp opcional
                                QRegularExpressionMatch match = rx.match(str);
                                if(match.hasMatch())
                                {
                                    DES_Adj.Dados.variaveis.Vmaior.append(match.captured(1).toDouble());
                                    DES_Adj.Dados.variaveis.Vmenor.append(match.captured(2).toDouble());
                                    DES_Adj.decimacao.append(match.captured(3).toInt());
                                    str.replace(rx,"");
                                }
                                else
                                {
                                    QRegularExpression rx2("\\(([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\,([-+]?\\d*\\.?\\d*[eE]?[-+]?\\d*)\\)"); //Float com ponto opcional
                                    match = rx2.match(str);
                                    if(match.hasMatch())
                                    {
                                        DES_Adj.Dados.variaveis.Vmaior.append(match.captured(1).toDouble());
                                        DES_Adj.Dados.variaveis.Vmenor.append(match.captured(2).toDouble());
                                        str.replace(rx2,"");
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
{
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
                DES_Adj.Dados.variaveis.valores(j,index) = (QString("%1").arg(
                   DES_Adj.Dados.variaveis.valores(j,index)>=DES_Adj.Dados.variaveis.Vmenor.at(j)?
                     DES_Adj.Dados.variaveis.valores(j,index)<=DES_Adj.Dados.variaveis.Vmaior.at(j)?
                         0.99*((DES_Adj.Dados.variaveis.valores.at(j,index)-DES_Adj.Dados.variaveis.Vmenor.at(j))/(DES_Adj.Dados.variaveis.Vmaior.at(j)-DES_Adj.Dados.variaveis.Vmenor.at(j)))+0.01:1:0.01)).toDouble();
            }
        }
        else break;
    }
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
    DES_isEquacaoEscrita = true;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::slot_DES_StatusSetado()
{
    DES_isStatusSetado=true;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// =============================================================================
// DES_AlgDiffEvol — Algoritmo de Evolução Diferencial (DE) Canônico Adaptado
//
// Fluxo principal:
//   1. INIT: Cria população inicial (ou reusa se isElitismo).
//   2. LOOP GERAÇÕES (pipeline multi-thread):
//      Para cada indivíduo (tokenPop) na subpopulação do pipeline:
//        a) Seleção de doadores: pbest (top-p%), r1, r2 — todos distintos e != target.
//        b) Leitura thread-safe de target, pbest, r1, r2.
//        c) Geração do trial via DES_GenerateTrial (mutação current-to-pbest/1 + crossover binomial).
//        d) Seleção 1-a-1: se aptidão(trial) <= aptidão(target), substitui target por trial.
//   3. BARREIRA (1 thread): Atualiza ranking (vetElitismo), detecta estagnação,
//      emite sinais de status/desenho. DES_crMut armazena o best (não mais aleatório).
//   4. PARADA: Estagnação por janela (numeroCiclos gerações sem melhora relativa/absoluta)
//      ou comando externo (modeOper_TH).
//
// Parâmetros DE em DES_Adj.deParams: { F, CR, pbest_rate, strategy, stagnation_window, tol_rel }
// =============================================================================
void DEStruct::DES_AlgDiffEvol()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 qtSaidas = DES_Adj.Dados.variaveis.qtSaidas;
    const qint32 tamPop = DES_Adj.Dados.tamPop;
    qint32 tokenPop;
    JMathVar<qreal> m1(10,10,5.0),m2(10,10,5.0);
    QVector<Cromossomo > crBest(qtSaidas);
    ////////////////////////////////////////////////////////////////////////////
    bool isOk=false,isPrint=true;
    qint32 count0=0,count2=0,idSaida=0,idPipeLine = 0;
    qint32 pbestPoint, r1Point, r2Point, cr2Point; // Índices dos doadores (DE canônico)
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
            DES_Adj.melhorAptidaoAnt.clear();
            for(idSaida=0;idSaida<qtSaidas;idSaida++)
            {
                DES_idChange[idPipeLine][idSaida]=tamPop+1;
                DES_Adj.vetElitismo[idPipeLine][idSaida].append(1);
                DES_Adj.vetElitismo[idPipeLine][idSaida].append(0);
                for(count0=2;count0<tamPop;count0++)
                    DES_Adj.vetElitismo[idPipeLine][idSaida].append(count0);
                DES_crMut[idPipeLine][idSaida] = DES_criaCromossomo(idSaida);//� o melhor cromossomo inicial
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
                // DE Canônico: Seleção de doadores (pbest, r1, r2)
                // pbest: aleatório do top-p% do ranking (vetElitismo)
                // r1, r2: aleatórios da população, todos distintos e != target (tokenPop)
                ////////////////////////////////////////////////////////////////////////////
                const qint32 topP = qMax((qint32)1, (qint32)(DES_Adj.deParams.pbest_rate * tamPop));
                qint32 pbest_rank, r1_rank, r2_rank;

                lock_DES_Elitismo[idPipeLine].lockForRead();
                // Seleciona pbest do top-p%, garantindo != target
                do {
                    pbest_rank = DES_RG.randInt(0, topP - 1);
                    pbestPoint = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(pbest_rank);
                } while(pbestPoint == tokenPop && topP > 1);
                // Seleciona r1 distinto de target e pbest
                do {
                    r1_rank = DES_RG.randInt(0, tamPop - 1);
                    r1Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(r1_rank);
                } while(r1Point == tokenPop || r1Point == pbestPoint);
                // Seleciona r2 distinto de target, pbest e r1
                do {
                    r2_rank = DES_RG.randInt(0, tamPop - 1);
                    r2Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(r2_rank);
                } while(r2Point == tokenPop || r2Point == pbestPoint || r2Point == r1Point);
                lock_DES_Elitismo[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
                // Leitura thread-safe dos cromossomos doadores e do target
                ////////////////////////////////////////////////////////////////////////////
                Cromossomo target_cr, pbest_cr, r1_cr, r2_cr;
                lock_DES_BufferSR.lockForRead();
                target_cr = DES_Adj.Pop.at(idSaida).at(tokenPop);
                pbest_cr  = DES_Adj.Pop.at(idSaida).at(pbestPoint);
                r1_cr     = DES_Adj.Pop.at(idSaida).at(r1Point);
                r2_cr     = DES_Adj.Pop.at(idSaida).at(r2Point);
                lock_DES_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////
                // Gera trial via mutação current-to-pbest/1 + crossover binomial
                ////////////////////////////////////////////////////////////////////////////
                Cromossomo trial = DES_GenerateTrial(target_cr, pbest_cr, r1_cr, r2_cr,
                                                     DES_Adj.deParams.F, DES_Adj.deParams.CR);
                ////////////////////////////////////////////////////////////////////////////
                // Seleção 1-a-1: trial vs target
                // Substitui SOMENTE se trial melhor (<=) ou target é NaN
                ////////////////////////////////////////////////////////////////////////////
                if(trial.regress.size() > 0 &&
                   (trial.aptidao <= target_cr.aptidao || target_cr.aptidao != target_cr.aptidao))
                {
                    lock_DES_BufferSR.lockForWrite();
                    DES_Adj.Pop[idSaida][tokenPop] = trial;
                    lock_DES_BufferSR.unlock();
                }
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
                //O elitismo é feito colocando os melhores (menor BIC) no inicio do vetor.
                // Ranking serve para obter x_best e top-p% (para pbest na próxima geração).
                // A substituição é LOCAL (trial vs target), feita acima no loop.
                isOk = true;
                for(idSaida=0;idSaida<qtSaidas;idSaida++)
                {
                    cr2Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);
                    qSortPop(DES_Adj.vetElitismo[idPipeLine][idSaida].begin(),DES_Adj.vetElitismo[idPipeLine][idSaida].end(),idSaida);
                    qint32 bestPoint = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);

                    lock_DES_BufferSR.lockForRead();
                    crBest[idSaida]=DES_Adj.Pop.at(idSaida).at(bestPoint);
                    // Critério de melhora: tolerância relativa OU absoluta (jnrr)
                    {
                        const qreal delta = DES_Adj.melhorAptidaoAnt.at(idSaida) - crBest.at(idSaida).aptidao;
                        const qreal base_val = qMax(qAbs(DES_Adj.melhorAptidaoAnt.at(idSaida)), 1e-12);
                        const bool improved = (delta / base_val >= DES_Adj.deParams.tol_rel) || (delta >= DES_Adj.jnrr);
                        if(improved) DES_Adj.melhorAptidaoAnt[idSaida] = crBest.at(idSaida).aptidao;
                        else isOk = false;
                    }
                    {
                        Cromossomo crOldBest = DES_Adj.Pop.at(idSaida).at(cr2Point);
                        isPrint = isPrint || (crBest.at(idSaida).aptidao < crOldBest.aptidao);
                    }
                    lock_DES_BufferSR.unlock();

                    // DES_crMut agora armazena o best (NÃO mais aleatório).
                    // Serve como snapshot do melhor cromossomo para display/logging.
                    DES_crMut[idPipeLine][idSaida] = crBest.at(idSaida);
                }
                ////////////////////////////////////////////////////////////////////////////
                lock_DES_BufferSR.lockForWrite();
                DES_Adj.iteracoes++;
                if(isOk) DES_Adj.iteracoesAnt = DES_Adj.iteracoes;
                // Estagnação por janela: usa o menor entre numeroCiclos e deParams.stagnation_window
                else {
                    const qint64 janela = qMin((qint64)DES_Adj.numeroCiclos, (qint64)DES_Adj.deParams.stagnation_window);
                    if(DES_Adj.iteracoes >= DES_Adj.iteracoesAnt + janela) slot_DES_Estado(2);
                }
                isOk = (((DES_Adj.tp.secsTo(QTime::currentTime()) >= 6)&&isPrint)||(DES_Adj.tp.secsTo(QTime::currentTime()) >= 60));
                if(isOk) DES_Adj.tp = QTime::currentTime();
                lock_DES_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////                
                if(isOk)
                {
                    for(idSaida=0;idSaida<qtSaidas;idSaida++)
                    {
                        DES_calAptidao(crBest[idSaida],15);
                        DES_MontaSaida(crBest[idSaida],DES_vcalc[idPipeLine][idSaida],DES_residuos[idPipeLine][idSaida]);
                    }
                    emit signal_DES_SetStatus(DES_Adj.iteracoes,&DES_somaSSE.at(idPipeLine),&DES_vcalc.at(idPipeLine),&DES_residuos.at(idPipeLine),&crBest); //Este tem que ser feito numa conex�o direta
                    if(DES_isEquacaoEscrita) {emit signal_DES_EscreveEquacao();DES_isEquacaoEscrita = false;}
                    if(DES_isStatusSetado) {emit signal_DES_Desenha();DES_isStatusSetado=false;}//Este pode ser feito numa conexao livre.
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
                    DES_calAptidao(crBest[idSaida],15);
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
            if((qint32)vlrTermo.vTermo.tTermo1.atraso>cr.maiorAtraso) cr.maiorAtraso = vlrTermo.vTermo.tTermo1.atraso;
            vlrTermo.expoente = (qreal) RG.randInt(1,10);
            if(!vlrTermo.expoente) vlrTermo.expoente=1;//Elimina a chance de gerar um expoente inicial igual a zero.
            vetTermo.append(vlrTermo);
            tamRegress--;
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Ordena os termos por ordem decrescente.
        std::sort(vetTermo.begin(),vetTermo.end(),CmpMaiorTerm);
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
// =============================================================================
// DES_GenerateTrial — DE Canônico adaptado para cromossomo estrutural (NARX/racional)
//
// Fluxo:
//   1. Coleta termos (genes) de target, pbest, r1, r2
//   2. Mutação current-to-pbest/1:
//        exponent_mutant = exponent_target + F*(exponent_pbest - exponent_target)
//                                          + F*(exponent_r1 - exponent_r2)
//      (Se o termo estiver ausente em algum doador, expoente tratado como 0.)
//   3. Crossover binomial com CR e jrand:
//        Para cada gene do universo (target ∪ mutant):
//          se rand < CR ou i == jrand → pega do mutant
//          senão → pega do target
//   4. Agrupa termos por idReg para reconstruir regress[]
//   5. Poda via DES_CalcERR + avalia aptidão via DES_calAptidao
//
// Parâmetros F ∈ [0.4, 0.9] e CR ∈ [0, 1] vêm de DES_Adj.deParams.
// =============================================================================
const Cromossomo DEStruct::DES_GenerateTrial(const Cromossomo &target, const Cromossomo &pbest,
                                              const Cromossomo &r1, const Cromossomo &r2,
                                              double F, double CR) const
{
    ////////////////////////////////////////////////////////////////////////////////
    MTRand RG(QTime::currentTime().msec());
    Cromossomo trial;
    trial.idSaida = target.idSaida;
    trial.maiorAtraso = 0;
    ////////////////////////////////////////////////////////////////////////////////
    // Phase 1: Coleta termos de todos os doadores com identificação de fonte
    // pos: 0=target (x_i), 1=pbest (x_pbest), 2=r1 (x_r1), 3=r2 (x_r2)
    ////////////////////////////////////////////////////////////////////////////////
    QVector<compTermo> termosAnalisados;
    QVector<qint32> posTermosAnalisados;
    qint32 i, j;

    for(i=0; i<target.regress.size(); i++) {
        termosAnalisados += target.regress.at(i);
        posTermosAnalisados += QVector<qint32>(target.regress.at(i).size(), 0);
    }
    for(i=0; i<pbest.regress.size(); i++) {
        termosAnalisados += pbest.regress.at(i);
        posTermosAnalisados += QVector<qint32>(pbest.regress.at(i).size(), 1);
    }
    for(i=0; i<r1.regress.size(); i++) {
        termosAnalisados += r1.regress.at(i);
        posTermosAnalisados += QVector<qint32>(r1.regress.at(i).size(), 2);
    }
    for(i=0; i<r2.regress.size(); i++) {
        termosAnalisados += r2.regress.at(i);
        posTermosAnalisados += QVector<qint32>(r2.regress.at(i).size(), 3);
    }

    if(termosAnalisados.isEmpty()) {
        trial = target;
        return trial;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Phase 2: Ordena por tTermo0 e computa expoentes mutados (current-to-pbest/1)
    //          Separa termos do target para uso no crossover binomial
    ////////////////////////////////////////////////////////////////////////////////
    qSortDuplo(termosAnalisados.begin(), termosAnalisados.end(),
               posTermosAnalisados.begin(), posTermosAnalisados.end(), CmpMaiorTerm);

    // Vetores flat: termos mutados e termos do target (para crossover)
    QVector<compTermo> mutTermos, tgtTermos;

    // Acumuladores por fonte para o tTermo0 corrente
    // [0]=target, [1]=pbest, [2]=r1, [3]=r2, [4]=chave corrente
    QVector<compTermo> termoAv(5, compTermo());
    compTermo auxTermo;

    termoAv[4] = termosAnalisados.at(0);
    termoAv[posTermosAnalisados.at(0)] = termosAnalisados.at(0);

    compTermo *tr;
    qint32 *pr;

    for(tr = termosAnalisados.begin()+1, pr = posTermosAnalisados.begin()+1;
        tr <= termosAnalisados.end(); tr++, pr++)
    {
        ////////////////////////////////////////////////////////////////////////////////
        // Quando muda tTermo0 (ou fim da lista): processa grupo acumulado
        if(tr < termosAnalisados.end() ? termoAv.at(4).vTermo.tTermo0 != tr->vTermo.tTermo0 : true)
        {
            // Algum doador tem este termo?
            if(termoAv.at(0).vTermo.tTermo0 || termoAv.at(1).vTermo.tTermo0 ||
               termoAv.at(2).vTermo.tTermo0 || termoAv.at(3).vTermo.tTermo0)
            {
                // Identidade estrutural: pega do primeiro doador que possui
                if(termoAv.at(0).vTermo.tTermo0)      auxTermo.vTermo.tTermo0 = termoAv.at(0).vTermo.tTermo0;
                else if(termoAv.at(1).vTermo.tTermo0)  auxTermo.vTermo.tTermo0 = termoAv.at(1).vTermo.tTermo0;
                else if(termoAv.at(2).vTermo.tTermo0)  auxTermo.vTermo.tTermo0 = termoAv.at(2).vTermo.tTermo0;
                else                                    auxTermo.vTermo.tTermo0 = termoAv.at(3).vTermo.tTermo0;

                // Mutação current-to-pbest/1:
                //   v = target + F*(pbest - target) + F*(r1 - r2)
                // Se doador ausente, expoente = 0 (default do compTermo)
                auxTermo.expoente = termoAv.at(0).expoente
                                  + F * (termoAv.at(1).expoente - termoAv.at(0).expoente)
                                  + F * (termoAv.at(2).expoente - termoAv.at(3).expoente);

                if(auxTermo.expoente != 0.0)
                    mutTermos.append(auxTermo);
            }

            // Coleta termo do target para crossover (se existir)
            if(termoAv.at(0).vTermo.tTermo0)
                tgtTermos.append(termoAv.at(0));

            // Reset acumuladores
            termoAv[0].vTermo.tTermo0 = 0; termoAv[0].expoente = 0.0;
            termoAv[1].vTermo.tTermo0 = 0; termoAv[1].expoente = 0.0;
            termoAv[2].vTermo.tTermo0 = 0; termoAv[2].expoente = 0.0;
            termoAv[3].vTermo.tTermo0 = 0; termoAv[3].expoente = 0.0;
        }
        ////////////////////////////////////////////////////////////////////////////////
        // Acumula próximo termo
        if(tr < termosAnalisados.end())
        {
            termoAv[4] = *tr;
            if(termoAv.at(*pr).vTermo.tTermo0 == tr->vTermo.tTermo0)
                termoAv[*pr].expoente += tr->expoente;
            else
                termoAv[*pr] = *tr;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Phase 3: Crossover binomial com CR e jrand
    //   Universo = mutTermos ∪ tgtTermos (por tTermo0)
    //   jrand garante pelo menos 1 gene do mutant
    ////////////////////////////////////////////////////////////////////////////////

    // Ordena ambas as listas por tTermo0 (decrescente) para merge eficiente
    std::sort(mutTermos.begin(), mutTermos.end(), CmpMaiorTerm);
    std::sort(tgtTermos.begin(), tgtTermos.end(), CmpMaiorTerm);

    // Merge das duas listas em um universo unificado
    // Para cada tTermo0 único: armazena expoente mutant e/ou target
    QVector<quint32> uKey;
    QVector<qreal> uMutExp, uTgtExp;
    QVector<qint32> uHasMut, uHasTgt; // 0 ou 1

    qint32 mi = 0, ti = 0;
    while(mi < mutTermos.size() || ti < tgtTermos.size())
    {
        quint32 mk = (mi < mutTermos.size()) ? mutTermos.at(mi).vTermo.tTermo0 : 0;
        quint32 tk = (ti < tgtTermos.size()) ? tgtTermos.at(ti).vTermo.tTermo0 : 0;

        if(mi >= mutTermos.size()) {
            // Só target restante
            uKey.append(tk); uMutExp.append(0.0); uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(0); uHasTgt.append(1);
            ti++;
        } else if(ti >= tgtTermos.size()) {
            // Só mutant restante
            uKey.append(mk); uMutExp.append(mutTermos.at(mi).expoente); uTgtExp.append(0.0);
            uHasMut.append(1); uHasTgt.append(0);
            mi++;
        } else if(mk > tk) {
            // Mutant vem primeiro (ordem decrescente)
            uKey.append(mk); uMutExp.append(mutTermos.at(mi).expoente); uTgtExp.append(0.0);
            uHasMut.append(1); uHasTgt.append(0);
            mi++;
        } else if(tk > mk) {
            // Target vem primeiro
            uKey.append(tk); uMutExp.append(0.0); uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(0); uHasTgt.append(1);
            ti++;
        } else {
            // Mesmo tTermo0
            uKey.append(mk);
            uMutExp.append(mutTermos.at(mi).expoente);
            uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(1); uHasTgt.append(1);
            mi++; ti++;
        }
    }

    if(uKey.isEmpty()) {
        trial = target;
        return trial;
    }

    // Aplica crossover binomial
    const qint32 jrand = RG.randInt(0, uKey.size() - 1);
    QVector<compTermo> trialTermos;

    for(i = 0; i < uKey.size(); i++)
    {
        bool fromMutant = (RG.randReal(0.0, 1.0) < CR) || (i == jrand);
        compTermo t;
        t.vTermo.tTermo0 = uKey.at(i);

        if(fromMutant) {
            if(uHasMut.at(i)) {
                t.expoente = uMutExp.at(i);
                trialTermos.append(t);
            }
            // Mutant não tem este termo → decisão estrutural: não inclui
        } else {
            if(uHasTgt.at(i)) {
                t.expoente = uTgtExp.at(i);
                trialTermos.append(t);
            }
            // Target não tem este termo → não inclui
        }
    }

    // Garante que trial não fique vazio (força pelo menos o termo jrand)
    if(trialTermos.isEmpty()) {
        compTermo t;
        t.vTermo.tTermo0 = uKey.at(jrand);
        t.expoente = uHasMut.at(jrand) ? uMutExp.at(jrand) : uTgtExp.at(jrand);
        trialTermos.append(t);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Phase 4: Agrupa termos por idReg → reconstrói trial.regress[]
    ////////////////////////////////////////////////////////////////////////////////

    // Ordena por tTermo0 (agrupa naturalmente por idReg nos bits altos)
    std::sort(trialTermos.begin(), trialTermos.end(), CmpMaiorTerm);

    trial.regress.clear();
    QVector<compTermo> currentGroup;
    quint32 currentIdReg = trialTermos.at(0).vTermo.tTermo2.idReg;

    for(i = 0; i < trialTermos.size(); i++)
    {
        quint32 thisIdReg = trialTermos.at(i).vTermo.tTermo2.idReg;

        if(thisIdReg != currentIdReg) {
            if(!currentGroup.isEmpty()) {
                // Remove duplicatas dentro do grupo
                for(j = 1; j < currentGroup.size(); j++)
                    if(currentGroup.at(j).vTermo.tTermo0 == currentGroup.at(j-1).vTermo.tTermo0)
                        currentGroup.remove(j--);
                trial.regress.append(currentGroup);
            }
            currentGroup.clear();
            currentIdReg = thisIdReg;
        }

        currentGroup.append(trialTermos.at(i));

        if((qint32)trialTermos.at(i).vTermo.tTermo1.atraso > trial.maiorAtraso)
            trial.maiorAtraso = trialTermos.at(i).vTermo.tTermo1.atraso;
    }
    // Último grupo
    if(!currentGroup.isEmpty()) {
        for(j = 1; j < currentGroup.size(); j++)
            if(currentGroup.at(j).vTermo.tTermo0 == currentGroup.at(j-1).vTermo.tTermo0)
                currentGroup.remove(j--);
        trial.regress.append(currentGroup);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Phase 5: Limita regressores, poda via ERR, avalia aptidão
    ////////////////////////////////////////////////////////////////////////////////

    const qint32 qtdeAtrasos = (DES_Adj.Dados.variaveis.valores.numColunas()/(2*DES_Adj.decimacao.at(trial.idSaida)))-27;
    while(trial.regress.size() > qtdeAtrasos && trial.regress.size() > 1)
        trial.regress.removeLast();

    trial.err.fill(-1, trial.regress.size());

    DES_CalcERR(trial, DES_Adj.serr);
    DES_calAptidao(trial);

    // Renumera regressores (mesma lógica do código original)
    const qint32 size1 = trial.regress.size();
    if(size1)
    {
        for(i = 0; i < size1; i++)
        {
            const qint32 size2 = trial.regress.at(i).size();
            for(j = 0; j < size2; j++)
                if(trial.regress.at(i).at(j).vTermo.tTermo1.reg)
                    trial.regress[i][j].vTermo.tTermo1.reg = (size1 - i);
        }
    }

    return trial;
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
                if((qint32)cr.regress.at(i).at(j).vTermo.tTermo1.atraso>cr.maiorAtraso)
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
            if((expo>1e-5)||(expo<-1e-5)||(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg))
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
inline void DEStruct::DES_CalcVlrsEstRes(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsCoefic,const JMathVar<qreal> &vlrsMedido,JMathVar<qreal> &vlrsResiduo,JMathVar<qreal> &vlrsEstimado) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JStrSet jst;
    JMathVar<qreal> vlrsRegressNum,vlrsRegressDen,vlrsCoeficNum,vlrsCoeficDen,a,b;
    qint32 i=0,atraso=0,tamNum=0,tamDen=0;
    qreal *estimado,*residuo;
    const qreal *medido;
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
    //Equa��o correta (consistente com o ELS Estendido):
    //  y = (Rnum*Cnum + E*Ce) / (1 + Rden*Cden)
    vlrsEstimado = vlrsRegressNum(vlrsCoeficNum,jst.set("(:,:)*(:,:)'"));
    b = vlrsRegressDen(vlrsCoeficDen,jst.set("(:,:)*(:,:)'"));
    a.fill(1,vlrsEstimado.numLinhas(),vlrsEstimado.numColunas());
    if(b.numLinhas() > 0)
        a.copy(b,jst.set("(:,:)+=(:,:)"));
    ////////////////////////////////////////////////////////////////////////////////
    //Insere os termos do residuo no sistema se houver
    const qint32 tamErro = vlrsCoefic.numColunas()-(tamNum+tamDen);
    if(tamErro)
    {
        vlrsResiduo.fill(0,vlrsMedido.numLinhas(),1);
        ////////////////////////////////////////////////////////////////////////////////
        //Calcula os valores estimados dos residuos com os regressores do residuo.
        //Os termos de erro sao somados ao numerador ANTES da divisao pelo denominador.
        const qint32 tamvlrsRegress = (tamNum+tamDen);
        const qreal *denVal = a.begin();
        for(atraso=0,estimado=vlrsEstimado.begin(),residuo=vlrsResiduo.begin(),medido=vlrsMedido.begin();medido < vlrsMedido.end();medido++,residuo++,estimado++,atraso++,denVal++)
        {
            for(i=0;i<tamErro;i++)
                *estimado += vlrsCoefic.at(tamvlrsRegress+i)*((atraso-i)>=0?*(residuo-i):0);
            *estimado /= *denVal;
            *residuo = *medido - *estimado;
        }
    }
    else
    {
        vlrsEstimado.copy(a,jst.set("(:,:)/=(:,:)"));
        vlrsResiduo = vlrsMedido(vlrsEstimado,jst.set("(:)-(:)"));
    }
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
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true);
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
                if((qint32)cr.regress.at(i).at(j).vTermo.tTermo1.atraso>cr.maiorAtraso)
                    cr.maiorAtraso=cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
        ////////////////////////////////////////////////////////////////////////////////                               
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_calAptidao(Cromossomo &cr, const quint32 &tamErro) const
{
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    bool isOk1,isOk2,isOk=false;
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
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true,false);
    const qint32 qtdeAtrasos = vlrsMedido.numLinhas();
    ////////////////////////////////////////////////////////////////////////////////
    // Proteção: se não há dados suficientes, retorna com aptidão máxima (pior)
    if (qtdeAtrasos <= 0 || cr.regress.size() == 0) {
        return;
    }
    ////////////////////////////////////////////////////////////////////////////////
    // Remove regressores excedentes que não foram processados em DES_MontaVlrs
    // (quando cr.regress.size() > vlrsRegress.numColunas(), as colunas extras não existem)
    while (cr.regress.size() > vlrsRegress.numColunas()) {
        cr.regress.removeLast();
        cr.err.remove('C', cr.err.numColunas()-1);
    }
    ////////////////////////////////////////////////////////////////////////////////
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
        if(count1&&isOk&&(erroDepois<cr.erro))
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
            vlrsRegress1.copy(vlrsResiduo,jst.set("(:,%1)=(:,:)").argInt(tamNum+tamDen+i));  //Insere termos do residuo
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
                DES_CalcVlrsEstRes(cr,vlrsRegress1,vlrsCoefic1,vlrsMedido,vlrsResiduo,vlrsEstimado);
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
    while((quint32) count1 <= tamErro );
    ////////////////////////////////////////////////////////////////////////////////
    //Elimina regressores com valor de coeficiente espurio. dentro da faixa de +-1e-5 ou acima da faixa +- 1e+5.
    for(isOk=false,i=0;i<cr.regress.size();i++) //Varre todos os regressores para aquele cromossomo menos os do residuo final
        if(cr.vlrsCoefic.at(i)!=0.0?((cr.vlrsCoefic.at(i)<=1e-3)&&(cr.vlrsCoefic.at(i)>=-1e-3))||(cr.vlrsCoefic.at(i)>=1e+3)||(cr.vlrsCoefic.at(i)<=-1e+3):false) {cr.regress.remove(i);cr.err.remove('C',i);isOk=true;}
    if(isOk) DES_calAptidao(cr);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_MontaSaida(Cromossomo &cr,QVector<qreal> &vplotar,QVector<qreal> &resid) const
{
    //////////////////////////////////////////////////////////////////////////////////
    JMathVar<qreal> vlrsRegress,vlrsEstimado,vlrsResiduo,vlrsMedido;
    //////////////////////////////////////////////////////////////////////////////////
    //Monta a matrix valores dos regressores
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true,false);
    DES_CalcVlrsEstRes(cr,vlrsRegress,cr.vlrsCoefic,vlrsMedido,vlrsResiduo,vlrsEstimado);
    ////////////////////////////////////////////////////////////////////////////////
    //Prenche os vetores de saida.
    vplotar.clear(); vplotar += (QVector<qreal> ) vlrsEstimado;
    resid.clear(); resid += (QVector<qreal> ) vlrsResiduo;
}
