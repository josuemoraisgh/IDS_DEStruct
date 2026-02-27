#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSemaphore>
#include <QDateTime>
#include <QRegularExpression>
#include <math.h>
#include <cmath>
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
namespace
{
constexpr qreal kInvalidCost = std::numeric_limits<qreal>::max();
constexpr qreal kDenClampEpsDefault = 1e-8;
constexpr qreal kGammaPenaltyDefault = 1.0;
constexpr qreal kLmTolStepDefault = 1e-4;
constexpr qint32 kLmMaxIterDefault = 2;
constexpr qreal kLmLambdaInit = 1e-3;
constexpr qreal kLmLambdaMin = 1e-12;
constexpr qreal kLmLambdaMax = 1e12;
constexpr qreal kLmDiagFloor = 1e-12;
constexpr qreal kPivotGuard = 1e-10;
constexpr qreal kExpoZeroTol = 1e-5;
constexpr qreal kSerrKeepThreshold = 1e-3;
constexpr qreal kSerrMinThreshold = 9e-4; // 0.0009
constexpr qreal kNoInputPenaltyFactor = 100.0;
}
////////////////////////////////////////////////////////////////////////////
qreal sign(const qreal &x)
{
    if(x>0) return 1;
    else if(x==0) return 0;
    else return -1; //if(x<0)
}
////////////////////////////////////////////////////////////////////////////////
static inline bool isFiniteReal(const qreal &v)
{
    return std::isfinite(static_cast<long double>(v));
}
////////////////////////////////////////////////////////////////////////////////
static inline bool isEvalLogEnabled()
{
    static const bool enabled = []() -> bool
    {
        bool ok = false;
        const int v = qEnvironmentVariableIntValue("DES_LOG_EVAL",&ok);
        if(ok) return v != 0;
        return qEnvironmentVariableIsSet("DES_LOG_EVAL");
    }();
    return enabled;
}
////////////////////////////////////////////////////////////////////////////////
static inline qreal safeDenClamp(const qreal &d,const qreal &eps)
{
    const qreal ad = fabs(d);
    if(ad >= eps) return d;
    return d>=0 ? eps : -eps;
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
    //Protecao contra NaN: NaN vai para o final (pior aptidao)
    const bool nan1 = (cr1.aptidao!=cr1.aptidao), nan2 = (cr2.aptidao!=cr2.aptidao);
    if(nan1 || nan2) return (!nan1 && nan2); //NaN e sempre "pior" (maior)
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
      try {
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
      } catch(const std::exception &ex) {
          qCritical("DEStruct thread exception: %s", ex.what());
          DES_Adj.modeOper_TH = 0; // encerra thread com seguranca
      } catch(...) {
          qCritical("DEStruct thread unknown exception");
          DES_Adj.modeOper_TH = 0;
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
                //Cria um cromossomo para cada popula��o altera��o aleatoriamente.
                count0 = DES_RG.randInt(0,tamPop-1);
                do{count1 = DES_RG.randInt(0,tamPop-1);}while(count1 == count0);
                do{count2 = DES_RG.randInt(0,tamPop-1);}while((count2 == count0)||(count2 == count1));
                //count3 = DES_RG.randInt(0,DES_BufferSR.at(idPipeLine).at(idSaida).size()-1);
                //do{count3 = DES_RG.randInt(0,tamPop-1);}while((count3 == count1)||(count3 == count2)||(count3 == count0));
                lock_DES_Elitismo[idPipeLine].lockForRead();
                cr0Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count0);
                cr1Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count1);
                cr2Point = DES_Adj.vetElitismo.at(idPipeLine).at(idSaida).at(count2);
                lock_DES_Elitismo[idPipeLine].unlock();
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
            vlrTermo.vTermo.tTermo1.basisType = RG.randInt(0, BASIS_COUNT-1); //Escolhe aleatoriamente o tipo de base
            vetTermo.append(vlrTermo);
            tamRegress--;
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Ordena os termos por ordem decrescente.
        std::sort(vetTermo.begin(),vetTermo.end(),CmpMaiorTerm);
        ///////////////////////////////////////////////////////////////////////////////////////////////
        //Concatena termos exatamente iguais (mesma variavel E mesmo basisType).
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
    //qreal expo,auxReal;
    qint32 i,j,size1,size2,size3,count=0,testeSize=0,teste,*pr;//countTermos=0;
    teste = RG.randInt(0,1);if(teste&1) count++;
    for(testeSize=1;count<5;testeSize++){teste=(teste<<1)+RG.randInt(0,1);if(teste&1)count++;}
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
        if(tr<termosAnalisados.end() ? (termoAv.at(5).vTermo.tTermo0 != tr->vTermo.tTermo0) : true) //Para os termos iguais (mesma variavel E mesmo basisType)
        {
            ////////////////////////////////////////////////////////////////////////////////
            if(termoAv.at(0).vTermo.tTermo0||termoAv.at(1).vTermo.tTermo0||termoAv.at(2).vTermo.tTermo0||termoAv.at(3).vTermo.tTermo0)
            {
                if(termoAv.at(0).vTermo.tTermo0) {auxTermo.vTermo.tTermo0 =  termoAv.at(0).vTermo.tTermo0;}
                if(termoAv.at(1).vTermo.tTermo0) {auxTermo.vTermo.tTermo0 =  termoAv.at(1).vTermo.tTermo0;}
                if(termoAv.at(2).vTermo.tTermo0) {auxTermo.vTermo.tTermo0 =  termoAv.at(2).vTermo.tTermo0;}
                if(termoAv.at(3).vTermo.tTermo0) {auxTermo.vTermo.tTermo0 =  termoAv.at(3).vTermo.tTermo0;}
                auxTermo.expoente = termoAv.at(0).expoente+ multBase*(termoAv.at(1).expoente-termoAv.at(0).expoente)+multBase*(termoAv.at(2).expoente-termoAv.at(3).expoente);
                // Mutacao do basisType: com 20% de chance, muda aleatoriamente
                if(RG.randInt(0,4) == 0) auxTermo.vTermo.tTermo1.basisType = RG.randInt(0, BASIS_COUNT-1);
                //expo = (qint32) auxTermo.expoente;
                //auxReal = auxTermo.expoente-expo;
                teste = (teste>>1)|((teste&1)<<testeSize);//Rotaciona os bits.
                //if(/*((teste>>i)&3)||*/(auxReal>=0?((auxReal)<=0.01)||((auxReal)>=0.95):((auxReal)>=-0.01)||((auxReal)<=-0.95)))
                //{
                //    expo +=(auxReal>=0.5)?1:((auxReal<=-0.5)?-1:0);
                //    if(((teste>>i)&3)==1) auxTermo.expoente = fabs(expo);
                //    else auxTermo.expoente = expo;
                //}
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
            //countTermos = 0;
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
                        std::sort(vetTermo3.begin(),vetTermo3.end(),CmpMaiorTerm);//Ordena os termos por ordem decrescente.
                        for(i=1;i<vetTermo3.size();i++)//Concatena termos exatamente iguais (mesma variavel E mesmo basisType).
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
    for(count=0,i=0;(i<size1)&&(i<=testeSize)&&(count<qtdeAtrasos);i++) if((teste>>i)&1) {crA1.regress.append(matTermo.at(i));count++;}
    for(i=testeSize+1;(i<size1)&&(count<qtdeAtrasos);i++)
        if(RG.randInt(0,1)){crA1.regress.append(matTermo.at(i));count++;}
    ////////////////////////////////////////////////////////////////////////////////
    {
        JMathVar<qreal> vlrsRegress,vlrsMedido;
        DES_MontaVlrs(crA1,vlrsRegress,vlrsMedido,true,false);
        DES_CalcERRPrepared(crA1,DES_Adj.serr,vlrsRegress,vlrsMedido);
        DES_calAptidaoPrepared(crA1,DES_Adj.isResiduo ? 1 : 0,vlrsRegress,vlrsMedido);
    }
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
// Aplica a transformacao de base a um vetor de dados (elemento a elemento).
// Retorna false se algum valor for NaN ou infinito.
static bool applyBasisTransform(JMathVar<qreal> &mat, const quint8 basisType, const qreal expo)
{
    if(basisType == BASIS_POW) return true; // Ja calculado pelo replace (pow padrao)
    bool isOk = true;
    qreal *ptr = mat.begin();
    const qreal *ptrEnd = mat.end();
    switch(basisType)
    {
        case BASIS_ABS: // |x|^e
            while(ptr < ptrEnd)
            {
                *ptr = qPow(fabs(*ptr), expo);
                if(!isFiniteReal(*ptr)) {isOk = false;}
                ptr++;
            }
            break;
        case BASIS_LOG: // log(1+|x|)^e
            while(ptr < ptrEnd)
            {
                *ptr = qPow(log(1.0 + fabs(*ptr)), expo);
                if(!isFiniteReal(*ptr)) {isOk = false;}
                ptr++;
            }
            break;
        case BASIS_EXP: // exp(alpha*x), expo=alpha
            while(ptr < ptrEnd)
            {
                *ptr = exp(expo * (*ptr));
                if(!isFiniteReal(*ptr)) {isOk = false;}
                ptr++;
            }
            break;
        case BASIS_TANH: // tanh(x)^e
            while(ptr < ptrEnd)
            {
                *ptr = qPow(tanh(*ptr), expo);
                if(!isFiniteReal(*ptr)) {isOk = false;}
                ptr++;
            }
            break;
        default:
            break;
    }
    return isOk;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool DEStruct::BuildEvalContextFromChromosome(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido,const qint32 &mErroEfetivo,ModelEvalContext &ctx) const
{
    ctx = ModelEvalContext();
    ctx.pVlrsRegress = &vlrsRegress;
    ctx.pY = &vlrsMedido;
    ctx.N = vlrsMedido.numLinhas();
    if(ctx.N <= 0) return false;

    const qint32 colsUse = qMin((qint32)cr.regress.size(),vlrsRegress.numColunas());
    for(qint32 i=0;i<colsUse;i++)
    {
        if(!cr.regress.at(i).size()) continue;
        if(cr.regress.at(i).at(0).vTermo.tTermo1.nd) ctx.idxNum.append(i);
        else ctx.idxDen.append(i);
    }

    ctx.hasNumConst = ctx.idxNum.isEmpty();
    ctx.tamNumSel = ctx.idxNum.size();
    ctx.tamDenSel = ctx.idxDen.size();
    ctx.mErro = qMax(0,mErroEfetivo);
    ctx.p = ctx.tamNumSel + ctx.tamDenSel + 1 + ctx.mErro; // +theta0

    ctx.numVec.resize(ctx.tamNumSel);
    ctx.denVec.resize(ctx.tamDenSel);
    ctx.e_hist.fill(0.0,ctx.mErro);
    ctx.s.resize(ctx.p);
    ctx.s_hist.resize(ctx.p);
    for(qint32 i=0;i<ctx.p;i++) ctx.s_hist[i].fill(0.0,ctx.mErro);

    ctx.minAbsDen = kInvalidCost;
    ctx.penDen = 0.0;
    return ctx.p > 0;
}
////////////////////////////////////////////////////////////////////////////////
bool DEStruct::EvaluateStreaming_1plusDen(ModelEvalContext &ctx,
                                          const QVector<qreal> &Theta,
                                          const bool &needTrace,
                                          const bool &needLM,
                                          const qreal &eps,
                                          const qreal &/*gamma*/,
                                          qreal &outSSE,
                                          qreal &outPen,
                                          qreal &outMinAbsDen,
                                          JMathVar<qreal> *outJTJ,
                                          QVector<qreal> *outJTr,
                                          JMathVar<qreal> *outYhat,
                                          JMathVar<qreal> *outE) const
{
    outSSE = 0.0;
    outPen = 0.0;
    outMinAbsDen = kInvalidCost;
    if(!ctx.pVlrsRegress || !ctx.pY || ctx.N<=0 || Theta.size()!=ctx.p) return false;
    if(needLM && (!outJTJ || !outJTr)) return false;
    if(needTrace && (!outYhat || !outE)) return false;

    const qint32 offNum = 0;
    const qint32 offDen = offNum + ctx.tamNumSel;
    const qint32 offTheta0 = offDen + ctx.tamDenSel;
    const qint32 offCe = offTheta0 + 1;

    if(needLM)
    {
        outJTJ->fill(0.0,ctx.p,ctx.p);
        outJTr->fill(0.0,ctx.p);
    }
    if(needTrace)
    {
        outYhat->fill(0.0,ctx.N,1);
        outE->fill(0.0,ctx.N,1);
    }

    for(qint32 i=0;i<ctx.mErro;i++) ctx.e_hist[i]=0.0;
    if(needLM)
    {
        for(qint32 pIdx=0;pIdx<ctx.p;pIdx++)
            for(qint32 i=0;i<ctx.mErro;i++)
                ctx.s_hist[pIdx][i]=0.0;
    }

    const qreal denWarn = 10.0*eps;
    qint32 lowDenHits = 0;
    ctx.minAbsDen = kInvalidCost;
    ctx.penDen = 0.0;

    for(qint32 k=0;k<ctx.N;k++)
    {
        const qreal y = ctx.pY->at(k,0);
        qreal n = ctx.hasNumConst ? 1.0 : 0.0;
        for(qint32 j=0;j<ctx.tamNumSel;j++)
        {
            const qreal x = ctx.pVlrsRegress->at(k,ctx.idxNum.at(j));
            ctx.numVec[j]=x;
            n += Theta.at(offNum+j)*x;
        }

        qreal g = n;
        for(qint32 i=0;i<ctx.mErro;i++) g += Theta.at(offCe+i)*ctx.e_hist.at(i);

        qreal d = 1.0;
        for(qint32 j=0;j<ctx.tamDenSel;j++)
        {
            const qreal x = ctx.pVlrsRegress->at(k,ctx.idxDen.at(j));
            ctx.denVec[j]=x;
            d += Theta.at(offDen+j)*x;
        }

        const qreal absD = fabs(d);
        if(absD < ctx.minAbsDen) ctx.minAbsDen = absD;
        if(absD < denWarn)
        {
            const qreal r = (denWarn-absD)/denWarn;
            outPen += r*r;
            lowDenHits++;
        }
        const qreal dSafe = safeDenClamp(d,eps);
        const qreal yhat = (g/dSafe) + Theta.at(offTheta0);
        const qreal e = y - yhat;

        if(!isFiniteReal(yhat) || !isFiniteReal(e)) return false;
        outSSE += e*e;
        if(!isFiniteReal(outSSE)) return false;

        if(needTrace)
        {
            (*outYhat)(k,0)=yhat;
            (*outE)(k,0)=e;
        }

        if(needLM)
        {
            for(qint32 pIdx=0;pIdx<ctx.p;pIdx++)
            {
                qreal sRat = 0.0;
                if(pIdx < offDen)
                {
                    sRat = ctx.numVec.at(pIdx)/dSafe;
                }
                else if(pIdx < offTheta0)
                {
                    const qint32 j = pIdx-offDen;
                    sRat = -(g/(dSafe*dSafe))*ctx.denVec.at(j);
                }
                else if(pIdx == offTheta0)
                {
                    sRat = 1.0;
                }

                qreal sval = sRat;
                for(qint32 i=0;i<ctx.mErro;i++)
                    sval -= Theta.at(offCe+i)*ctx.s_hist[pIdx].at(i);

                if(pIdx >= offCe)
                {
                    const qint32 ceIdx = pIdx-offCe;
                    if((ceIdx>=0)&&(ceIdx<ctx.mErro)) sval += ctx.e_hist.at(ceIdx);
                }

                if(!isFiniteReal(sval)) return false;
                ctx.s[pIdx]=sval;
            }

            for(qint32 i=0;i<ctx.p;i++)
            {
                const qreal si = ctx.s.at(i);
                (*outJTr)[i] += si*e;
                for(qint32 j=i;j<ctx.p;j++)
                    (*outJTJ)(i,j) += si*ctx.s.at(j);
            }

            for(qint32 pIdx=0;pIdx<ctx.p;pIdx++)
            {
                for(qint32 i=ctx.mErro-1;i>0;i--) ctx.s_hist[pIdx][i]=ctx.s_hist[pIdx].at(i-1);
                if(ctx.mErro>0) ctx.s_hist[pIdx][0]=ctx.s.at(pIdx);
            }
        }

        for(qint32 i=ctx.mErro-1;i>0;i--) ctx.e_hist[i]=ctx.e_hist.at(i-1);
        if(ctx.mErro>0) ctx.e_hist[0]=e;

        if((lowDenHits>32) && ((lowDenHits*4)>(k+1))) return false;
    }

    if(needLM)
        for(qint32 i=0;i<ctx.p;i++)
            for(qint32 j=i+1;j<ctx.p;j++)
                (*outJTJ)(j,i)=outJTJ->at(i,j);

    outMinAbsDen = ctx.minAbsDen;
    ctx.penDen = outPen;
    return true;
}
////////////////////////////////////////////////////////////////////////////////
void DEStruct::InitThetaByLS(const ModelEvalContext &ctx,QVector<qreal> &Theta) const
{
    Theta.fill(0.0,ctx.p);
    if(ctx.N<=0 || !ctx.pY || !ctx.pVlrsRegress) return;

    const qint32 offDen = ctx.tamNumSel;
    const qint32 offTheta0 = offDen + ctx.tamDenSel;

    qreal mediaY = 0.0;
    for(qint32 k=0;k<ctx.N;k++) mediaY += ctx.pY->at(k,0);
    mediaY /= ctx.N;

    if(ctx.tamNumSel<=0)
    {
        Theta[offTheta0] = mediaY - (ctx.hasNumConst ? 1.0 : 0.0);
        return;
    }

    const qint32 pLS = ctx.tamNumSel + 1; // +theta0
    JMathVar<qreal> A;
    JMathVar<qreal> b;
    A.fill(0.0,pLS,pLS);
    b.fill(0.0,pLS,1);

    for(qint32 k=0;k<ctx.N;k++)
    {
        for(qint32 i=0;i<ctx.tamNumSel;i++)
        {
            const qreal xi = ctx.pVlrsRegress->at(k,ctx.idxNum.at(i));
            for(qint32 j=i;j<ctx.tamNumSel;j++)
            {
                const qreal xj = ctx.pVlrsRegress->at(k,ctx.idxNum.at(j));
                A(i,j) += xi*xj;
            }
            A(i,ctx.tamNumSel) += xi;
            b(i,0) += xi*ctx.pY->at(k,0);
        }
        A(ctx.tamNumSel,ctx.tamNumSel) += 1.0;
        b(ctx.tamNumSel,0) += ctx.pY->at(k,0);
    }
    for(qint32 i=0;i<pLS;i++)
        for(qint32 j=i+1;j<pLS;j++)
            A(j,i)=A.at(i,j);

    bool isOk=false;
    JMathVar<qreal> sol = A.SistemaLinear(b,isOk);
    if(isOk && (sol.size()>=pLS))
    {
        bool ok2=true;
        for(qint32 i=0;i<ctx.tamNumSel;i++)
        {
            if(!isFiniteReal(sol.at(i))) {ok2=false;break;}
            Theta[i]=sol.at(i);
        }
        if(ok2 && isFiniteReal(sol.at(ctx.tamNumSel)))
        {
            Theta[offTheta0] = sol.at(ctx.tamNumSel);
            return;
        }
    }
    Theta[offTheta0] = mediaY - (ctx.hasNumConst ? 1.0 : 0.0);
}
////////////////////////////////////////////////////////////////////////////////
bool DEStruct::LMRefineBudget_AllChromosomes(ModelEvalContext &ctx,
                                             QVector<qreal> &Theta,
                                             const qint32 &maxIterLM,
                                             const qreal &eps,
                                             const qreal &gamma,
                                             const qreal &tolStep,
                                             qreal &outSSE,
                                             qreal &outPen,
                                             qreal &outMinAbsDen,
                                             qreal &outLambdaFinal,
                                             qint32 &outIterUsed) const
{
    outSSE = kInvalidCost;
    outPen = 0.0;
    outMinAbsDen = 0.0;
    outLambdaFinal = kLmLambdaInit;
    outIterUsed = 0;
    if(Theta.size()!=ctx.p) return false;

    qreal sse=0.0,pen=0.0,minAbsDen=0.0;
    if(!EvaluateStreaming_1plusDen(ctx,Theta,false,false,eps,gamma,sse,pen,minAbsDen,NULL,NULL,NULL,NULL))
        return false;

    qreal J = sse + gamma*pen;
    qreal lambda = kLmLambdaInit;
    const qint32 p = ctx.p;
    JMathVar<qreal> JTJ,A,b,deltaMat;
    QVector<qreal> JTr,ThetaTry,delta;
    ThetaTry.resize(p);
    delta.resize(p);

    for(qint32 iter=0;iter<maxIterLM;iter++)
    {
        if(!EvaluateStreaming_1plusDen(ctx,Theta,false,true,eps,gamma,sse,pen,minAbsDen,&JTJ,&JTr,NULL,NULL))
            break;

        A = JTJ;
        b.fill(0.0,p,1);
        for(qint32 i=0;i<p;i++)
        {
            qreal di = A.at(i,i);
            if(!isFiniteReal(di) || fabs(di)<kLmDiagFloor) di = 1.0;
            A(i,i) = A.at(i,i) + lambda*di;
            b(i,0) = JTr.at(i);
        }

        bool isOk=false;
        deltaMat = A.SistemaLinear(b,isOk);
        if(!isOk || (deltaMat.size()<p))
        {
            lambda *= 10.0;
            if(lambda>kLmLambdaMax) break;
            continue;
        }

        qreal normDelta=0.0,normTheta=0.0;
        bool isFiniteDelta=true;
        for(qint32 i=0;i<p;i++)
        {
            delta[i]=deltaMat.at(i);
            if(!isFiniteReal(delta.at(i))) {isFiniteDelta=false;break;}
            ThetaTry[i]=Theta.at(i)+delta.at(i);
            if(!isFiniteReal(ThetaTry.at(i))) {isFiniteDelta=false;break;}
            normDelta += delta.at(i)*delta.at(i);
            normTheta += Theta.at(i)*Theta.at(i);
        }
        if(!isFiniteDelta)
        {
            lambda *= 10.0;
            if(lambda>kLmLambdaMax) break;
            continue;
        }

        qreal sseTry=0.0,penTry=0.0,minAbsTry=0.0;
        if(!EvaluateStreaming_1plusDen(ctx,ThetaTry,false,false,eps,gamma,sseTry,penTry,minAbsTry,NULL,NULL,NULL,NULL))
        {
            lambda *= 10.0;
            if(lambda>kLmLambdaMax) break;
            continue;
        }

        const qreal Jtry = sseTry + gamma*penTry;
        if(Jtry < J)
        {
            Theta = ThetaTry;
            J = Jtry;
            sse = sseTry;
            pen = penTry;
            minAbsDen = minAbsTry;
            lambda *= 0.1;
            if(lambda<kLmLambdaMin) lambda=kLmLambdaMin;
            outIterUsed = iter+1;
            const qreal stepRel = sqrt(normDelta)/(sqrt(normTheta)+kLmDiagFloor);
            if(stepRel < tolStep) break;
        }
        else
        {
            lambda *= 10.0;
            if(lambda>kLmLambdaMax) break;
        }
    }

    outSSE = sse;
    outPen = pen;
    outMinAbsDen = minAbsDen;
    outLambdaFinal = lambda;
    return isFiniteReal(outSSE) && (outSSE>=0.0);
}
////////////////////////////////////////////////////////////////////////////////
qreal DEStruct::CalcPenalidadeSemEntrada(const Cromossomo &cr,const qint32 &qtdeAtrasos) const
{
    bool temEntrada = false;
    const quint32 qtS = (quint32)DES_Adj.Dados.variaveis.qtSaidas;
    for(qint32 r=0; r<cr.regress.size() && !temEntrada; r++)
        for(qint32 t=0; t<cr.regress.at(r).size() && !temEntrada; t++)
            if(cr.regress.at(r).at(t).vTermo.tTermo1.reg)
                if(cr.regress.at(r).at(t).vTermo.tTermo1.var > qtS) temEntrada = true;
    return temEntrada ? 0.0 : (kNoInputPenaltyFactor*qtdeAtrasos);
}
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_CalcERRPrepared(Cromossomo &cr,const qreal &metodoSerr,JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido) const
{
    // Nucleo original de ERR reaproveitando vlrsRegress/vlrsMedido previamente montados.
    JStrSet jst;
    qint32 i=0,j=0,k=0,aux,start=0;
    qreal vlrsMedidoQuad=0.,serr=0.,u=0.;
    JMathVar<qreal> A,a,c,x,v;

    if(cr.regress.size())
    {
        A = vlrsRegress;
        // Para compatibilidade com o ERR antigo, lineariza apenas para o calculo de ERR:
        // termos de denominador recebem -y sem alterar vlrsRegress base.
        for(i=0;i<cr.regress.size() && i<A.numColunas();i++)
            if(cr.regress.at(i).size() && !cr.regress.at(i).at(0).vTermo.tTermo1.nd)
                for(j=0;j<A.numLinhas();j++)
                    A(j,i) = -A.at(j,i)*vlrsMedido.at(j,0);

        A.replace(vlrsMedido,jst.set("(:,%1)=(:,:)").argInt(A.numColunas()));
        const qint32 n = A.numColunas()-1;
        vlrsMedidoQuad = vlrsMedido(vlrsMedido,jst.set("(:)'*(:)")).at(0);
        c.resize(n);
        cr.err.fill(0,n);
        for(j=0;j<n;j++)
        {
            for(k=j;k<n;k++)
            {
                x = (A(A,jst.set("(%1:,%2)'*(%1:,%3)^2").argInt(j).argInt(k).argInt(n)));
                v = (A(A,jst.set("%f1*(%1:,%2)'*(%1:,%2)").argReal(vlrsMedidoQuad).argInt(j).argInt(k)));
                c(k) = x.at(0)/v.at(0);
            }
            cr.err(j) = c.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n),start,aux);
            if(aux!=j)
            {
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j],cr.regress[aux]);
            }
            v = A(jst.set("(%1:,%1)'").argInt(j));
            u=v.Norma(2);
            if(u!=0)
            {
                u = ((v.at(0)>kPivotGuard)||(v.at(0)<-kPivotGuard)?v.at(0):(v.at(0)>0)?kPivotGuard:-kPivotGuard) + sign(v.at(0))*u;
                v.replace(v,jst.set("(1:)=%f1*(1:)").argReal(1/u));
            }
            v(0)=1;
            a = A(jst.set("(%1:,%1:)").argInt(j));
            u = -2/v(v,jst.set("(:)*(:)'")).at(0);
            x = a(a,jst.set("(0:,0:)=%f1*(0:,0:)").argReal(u));
            x = x(v,jst.set("(:,:)'*(:,:)'"));
            x = v(x,jst.set("(:,:)'*(:,:)'"));
            a.replace(x,jst.set("(:,:)+=(:,:)"));
            A.replace(a,jst.set("(%1,%1)=(:,:)").argInt(j));
        }
        for(j=0;j<n-1;j++)
        {
            cr.err.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n),start,aux);
            if(aux!=j)
            {
                cr.err.swap(jst.set("(%1)=(%2)").argInt(j).argInt(aux));
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j],cr.regress[aux]);
            }
        }
        for(j=0,serr=0.0;(j<cr.err.size())&&((serr<metodoSerr)||(cr.err.at(j)>kSerrKeepThreshold))&&(cr.err.at(j)>kSerrMinThreshold)&&(cr.err.at(j)==cr.err.at(j));j++)
            serr+=cr.err.at(j);
        vlrsRegress = vlrsRegress(jst.set("(:,0:1:%1)").argInt(j));
        cr.err = cr.err(jst.set("(:,0:1:%1)").argInt(j));
        for(;j<cr.regress.size();) cr.regress.remove(j);
        for(cr.maiorAtraso=0,i=0;i<cr.regress.size();i++)
            for(j=0;j<cr.regress.at(i).size();j++)
                if((qint32)cr.regress.at(i).at(j).vTermo.tTermo1.atraso>cr.maiorAtraso)
                    cr.maiorAtraso=cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
    }
}
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_calAptidaoPrepared(Cromossomo &cr,const quint32 &tamErroEfetivo,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido) const
{
    cr.erro = kInvalidCost;
    cr.aptidao = kInvalidCost;
    cr.theta0 = 0.0;

    const qint32 N = vlrsMedido.numLinhas();
    if(N<=0)
    {
        if(isEvalLogEnabled()) qDebug().nospace() << "DES_EVAL abort sid=" << cr.idSaida << " reason=N<=0";
        return;
    }

    while(cr.regress.size()>vlrsRegress.numColunas())
    {
        cr.regress.removeLast();
        if(cr.err.numColunas()>0) cr.err.remove('C',cr.err.numColunas()-1);
    }

    if(cr.err.numColunas()<cr.regress.size())
        for(qint32 i=cr.err.numColunas();i<cr.regress.size();i++) cr.err.append('C',-1);
    while(cr.err.numColunas()>cr.regress.size()) cr.err.remove('C',cr.err.numColunas()-1);

    const qreal eps = kDenClampEpsDefault;
    const qreal gamma = kGammaPenaltyDefault;
    const qreal tolStep = kLmTolStepDefault;
    const qint32 maxIterLM = kLmMaxIterDefault; // Budget leve para todos os cromossomos

    ModelEvalContext ctx;
    if(!BuildEvalContextFromChromosome(cr,vlrsRegress,vlrsMedido,(qint32)tamErroEfetivo,ctx))
    {
        if(isEvalLogEnabled()) qDebug().nospace() << "DES_EVAL abort sid=" << cr.idSaida << " reason=invalid_context";
        return;
    }

    QVector<qreal> Theta;
    InitThetaByLS(ctx,Theta);

    qreal sse=kInvalidCost,pen=0.0,minAbsDen=0.0,lambdaFinal=0.0;
    qint32 itLM=0;
    const bool okLM = LMRefineBudget_AllChromosomes(ctx,Theta,maxIterLM,eps,gamma,tolStep,sse,pen,minAbsDen,lambdaFinal,itLM);
    if(!okLM)
    {
        if(isEvalLogEnabled()) qDebug().nospace() << "DES_EVAL abort sid=" << cr.idSaida << " reason=lm_failed";
        return;
    }

    const qint32 offDen = ctx.tamNumSel;
    const qint32 offTheta0 = offDen + ctx.tamDenSel;
    const qint32 offCe = offTheta0 + 1;

    cr.vlrsCoefic.fill(0.0,cr.regress.size()+ctx.mErro);
    for(qint32 i=0;i<ctx.tamNumSel;i++)
        if(i<ctx.idxNum.size())
            cr.vlrsCoefic(ctx.idxNum.at(i)) = Theta.at(i);
    for(qint32 i=0;i<ctx.tamDenSel;i++)
        if(i<ctx.idxDen.size())
            cr.vlrsCoefic(ctx.idxDen.at(i)) = Theta.at(offDen+i);
    for(qint32 i=0;i<ctx.mErro;i++)
        cr.vlrsCoefic(cr.regress.size()+i) = Theta.at(offCe+i);
    cr.theta0 = Theta.at(offTheta0);

    const qreal mse = sse/N;
    if(!isFiniteReal(mse) || (mse<=0.0))
    {
        if(isEvalLogEnabled()) qDebug().nospace() << "DES_EVAL abort sid=" << cr.idSaida << " reason=invalid_mse mse=" << mse;
        return;
    }
    cr.erro = mse;

    // Penalizacao de tamanho (BIC): considera todos os parametros do modelo,
    // incluindo termos de residuo (ce).
    const qint32 kBIC = ctx.p;
    cr.aptidao = N*qLn(cr.erro) + DES_Adj.pesoBIC*kBIC*qLn(N);
    cr.aptidao += CalcPenalidadeSemEntrada(cr,N);
    if(!isFiniteReal(cr.aptidao))
    {
        cr.aptidao=kInvalidCost;
        cr.erro=kInvalidCost;
        if(isEvalLogEnabled()) qDebug().nospace() << "DES_EVAL abort sid=" << cr.idSaida << " reason=invalid_bic";
        return;
    }

    if(isEvalLogEnabled())
    {
        qDebug().nospace()
            << "DES_EVAL sid=" << cr.idSaida
            << " N=" << N
            << " p=" << ctx.p
            << " kBIC=" << kBIC
            << " SSE=" << sse
            << " penDen=" << pen
            << " minAbsDen=" << minAbsDen
            << " lambdaFinal=" << lambdaFinal
            << " iterLM=" << itLM
            << " mse=" << cr.erro
            << " bic=" << cr.aptidao;
    }
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
            if((expo>kExpoZeroTol)||(expo<-kExpoZeroTol)||(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg))
            {
                atraso = cr.regress.at(countRegress).at(i).vTermo.tTermo1.atraso; //Obtem o atraso deste regressor
                const quint8 bt = cr.regress.at(countRegress).at(i).vTermo.tTermo1.basisType;
                // Para bases nao-pow, obtem x^1 (valores brutos) e depois aplica a transformacao
                const qreal expoReplace = (bt == BASIS_POW) ? expo : 1.0;
                if(!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg?matAux.fill(1,tam,1):matAux.replace(DES_Adj.Dados.variaveis.valores,jst.set("(:,:)=(%1,%2:%3:%4)'^%f1").argInt(variavel-1).argInt(posIniAtrasos-atraso*DES_Adj.decimacao.at(cr.idSaida)).argInt(DES_Adj.decimacao.at(cr.idSaida)).argInt(posIniAtrasos+(tam-atraso)*DES_Adj.decimacao.at(cr.idSaida)).argReal(expoReplace))||isValidacao)
                {
                    // Aplica transformacao de base (|x|^e, log, exp, tanh) se nao for pow padrao
                    if(bt != BASIS_POW && cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg)
                    {
                        if(!applyBasisTransform(matAux, bt, expo) && !isValidacao)
                        {
                            cr.regress[countRegress].remove(i--);
                            continue;
                        }
                    }
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
    const qint32 tamErro = qMax(0,vlrsCoefic.numColunas()-cr.regress.size());
    ModelEvalContext ctx;
    if(!BuildEvalContextFromChromosome(cr,vlrsRegress,vlrsMedido,tamErro,ctx))
    {
        vlrsEstimado.fill(0.0,vlrsMedido.numLinhas(),1);
        vlrsResiduo = vlrsMedido;
        return;
    }

    const qint32 offNum = 0;
    const qint32 offDen = offNum + ctx.tamNumSel;
    const qint32 offTheta0 = offDen + ctx.tamDenSel;
    const qint32 offCe = offTheta0 + 1;

    QVector<qreal> Theta;
    Theta.fill(0.0,ctx.p);

    for(qint32 i=0;i<ctx.tamNumSel;i++)
    {
        const qint32 idReg = ctx.idxNum.at(i);
        if(idReg<vlrsCoefic.numColunas()) Theta[i] = vlrsCoefic.at(idReg);
    }
    for(qint32 i=0;i<ctx.tamDenSel;i++)
    {
        const qint32 idReg = ctx.idxDen.at(i);
        if(idReg<vlrsCoefic.numColunas()) Theta[offDen+i] = vlrsCoefic.at(idReg);
    }
    Theta[offTheta0] = cr.theta0;
    for(qint32 i=0;i<ctx.mErro;i++)
    {
        const qint32 idCoef = cr.regress.size()+i;
        if(idCoef<vlrsCoefic.numColunas()) Theta[offCe+i] = vlrsCoefic.at(idCoef);
    }

    qreal sse=0.0,pen=0.0,minAbsDen=0.0;
    if(!EvaluateStreaming_1plusDen(ctx,Theta,true,false,kDenClampEpsDefault,kGammaPenaltyDefault,sse,pen,minAbsDen,NULL,NULL,&vlrsEstimado,&vlrsResiduo))
    {
        vlrsEstimado.fill(0.0,vlrsMedido.numLinhas(),1);
        vlrsResiduo = vlrsMedido;
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//Calcula a taxa de redu��o do erro e seleciona apartir de uma valor serr desejado
void DEStruct::DES_CalcERR(Cromossomo &cr,const qreal &metodoSerr) const
{
    JMathVar<qreal> vlrsMedido,vlrsRegress;
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true,false);
    DES_CalcERRPrepared(cr,metodoSerr,vlrsRegress,vlrsMedido);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DEStruct::DES_calAptidao(Cromossomo &cr, const quint32 &tamErro) const
{
    const quint32 tamErroEfetivo = DES_Adj.isResiduo ? tamErro : 0;
    JMathVar<qreal> vlrsRegress,vlrsMedido;
    DES_MontaVlrs(cr,vlrsRegress,vlrsMedido,true,false);
    DES_calAptidaoPrepared(cr,tamErroEfetivo,vlrsRegress,vlrsMedido);
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
