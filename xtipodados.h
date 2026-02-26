#ifndef XTIPODADOS_H
#define XTIPODADOS_H

#include <QtCore/QObject>
#include <QReadWriteLock>
#include <QTimer>
#include <QDateTime>
#include <algorithm>
#include "xmatriz.h"
#include "xvetor.h"

////////////////////////////////////////////////////////////////////////////
#define NUMDENOM    1
#define REGRESS     9
#define ATRASOS     11
#define VARIAVEL    11
#define MASKREG     ((1<<REGRESS)-1)
////////////////////////////////////////////////////////////////////////////

#ifndef TTERM1
#define TTERM1
struct tTerm1
{
    quint32 var    : VARIAVEL;
    quint32 atraso : ATRASOS;
    quint32 reg    : REGRESS;
    quint32 nd     : NUMDENOM; //1 Numerador, 0 Denominador
};

#endif //TTERM1

#ifndef TTERM2
#define TTERM2
struct tTerm2
{
    quint32 idVar   : ATRASOS+VARIAVEL;
    quint32 idReg   : NUMDENOM+REGRESS;
};

#endif //TTERM2

#ifndef TERMO
#define TERMO
union Termo{
    quint32 tTermo0;
    tTerm1  tTermo1;
    tTerm2  tTermo2;
};

#endif //TERMO

#ifndef COMPTERMO
#define COMPTERMO
// Tipos de base (basisType):
//   0 = pow(x, e)          - Original: x^e
//   1 = |x|^e              - Valor absoluto elevado a e
//   2 = log(1+|x|)^e       - Logaritmo elevado a e
//   3 = exp(alpha*x)        - Exponencial (expoente = alpha)
//   4 = tanh(x)^e           - Tangente hiperbolica elevada a e
#define BASIS_POW    0
#define BASIS_ABS    1
#define BASIS_LOG    2
#define BASIS_EXP    3
#define BASIS_TANH   4
#define BASIS_COUNT  5
class compTermo
{   
public:
    Termo vTermo;
    qreal expoente;
    quint8 basisType;
    compTermo(){vTermo.tTermo0=0;expoente=0.0f;basisType=BASIS_POW;}
    compTermo(const compTermo &termo){vTermo.tTermo0=termo.vTermo.tTermo0;expoente=termo.expoente;basisType=termo.basisType;}
    void operator=(const compTermo &termo)
    {
        vTermo.tTermo0=termo.vTermo.tTermo0;
        expoente=termo.expoente;
        basisType=termo.basisType;
    }
};

#endif //compTermo

#ifndef CROMOSSOMO
#define CROMOSSOMO
class Cromossomo
{
public:
    qreal erro,
          aptidao;                //Aptid�o calculada pelo m�todo BIC
    qint32 idSaida,               //Indentificador da saida para este cromossomo
           maiorAtraso;
    JMathVar<qreal> err,vlrsCoefic;
    QVector<QVector<compTermo > > regress;
    //QVector<QVector<compTermo > > regressResid;
    Cromossomo(){erro=0.0f;aptidao=0.0f;idSaida=0;maiorAtraso=0;vlrsCoefic.clear();err.clear();regress.clear();}
    void operator=(const Cromossomo &cr)
    {
        erro=cr.erro;
        aptidao=cr.aptidao;
        idSaida=cr.idSaida;
        maiorAtraso=cr.maiorAtraso;        
        vlrsCoefic.resize(cr.vlrsCoefic.size());
        std::copy(cr.vlrsCoefic.begin(),cr.vlrsCoefic.end(),vlrsCoefic.begin());
        err.resize(cr.err.size());
        std::copy(cr.err.begin(),cr.err.end(),err.begin());
        regress.resize(cr.regress.size());
        for(qint32 i=0;i<cr.regress.size();i++)
        {
            regress[i].resize(cr.regress.at(i).size());
            std::copy(cr.regress[i].begin(),cr.regress[i].end(),regress[i].begin());
        }
    }
    Cromossomo(const Cromossomo &cr) {*this = cr;}
};
#endif //CROMOSSOMO

#ifndef PSIM
#define PSIM
struct PSim{
    qint32 qtSaidas;        //Quantidade de saidas
    QList<QString> nome;    //Vetor com os nomes dos pontos de opera��o
    QList<qreal>   Vmaior;  //Vetor com os maiores pontos de opera��o
    QList<qreal>   Vmenor;  //Vetor com os menores pontos de opera��o
    JMathVar<qreal> valores; //Matriz onde Linha � as variaveis e coluna os atrasos.
                            //(sendo as primeiras linhas as variaveis de saida)
                            //(sendo as ultimas linhas as entradas)
};
#endif //PSIM

#ifndef INFAGS
#define INFAGS
struct infAGS{
    PSim   variaveis;            //Estrutura de dados das variaveis    
    qint32 iElitismo,            //Indice do elitismo das variaveis
           qtdadeVarAnte;        //Grava a quantidade de variaveis anteriores
    qint32 tamPop;               //Tamanho da Popula��o
    //qreal  timeAmost;            //Tempo de Amostragem em segundos
    qreal  timeInicial;          //Tempo Inicial em segundos
    qreal  timeFinal;            //Tempo Final em segundos
    qint16 volatile isElitismo;  //0 - n�o fazer nada; 1 - Se vai aproveitar o elementos do elitismos anteriores; 2 - Come�ar do zero a popula��o
};
#endif //INFAGS

#ifndef CONFIG
#define CONFIG
struct Config
{
    bool   isIniciaEnabled,         //0 - Desabilitado; 1 - Habilitado;
           salvarAutomati,
           isRacional,
           isCriado;
    QDateTime segundos;
    QString nomeArqConfTxT;
    QTime           tp;
    volatile qint64 iteracoes,
                    iteracoesAnt;
    volatile qint16 modeOper_TH;
    infAGS          Dados;
    quint8  isPararContinuarEnabled, //0 - Desabilitado; 1 - Parar; 2 - Continuar;
            isTipoExpo;              //0 - Expoente Real; 1 - Expoente Interio; 2 - Expoente Natural;
    quint32 qdadeCrSalvos,
            salvDadosCarre,
            numeroCiclos;
    qreal   serr,
            jnrr;
    QVector<qreal> melhorAptidaoAnt;//� a melhor aptidao no ultimo reset de cada saida

    QVector<qint32 > decimacao,
                     talDecim;
    //Saidas - Cromossomo
    QList<QVector<Cromossomo > > Pop;
    //PIPELINE - Cromossomo
    QList<QList<qint32 > >  vetPop;
    //PIPELINE - Saidas - Cromossomo   
    QList<QList<QList<bool> > > isSR;
    QList<QList<QVector<qint32 > > > vetElitismo;                     
};
#endif //CONFIG

#ifndef ANALISADOR
#define ANALISADOR
struct Analisador{
    qint32 index;
    const qreal *erro;
    const Termo *endtermoFinal, *endtermo;
    const qreal *endCoeficFinal, *endCoefic;
};
#endif //ANALISADOR

#endif //XTIPODADOS_H
