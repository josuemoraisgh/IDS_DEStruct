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
#define BASISTYPE   3
#define ATRASOS     10
#define VARIAVEL    9
#define MASKREG     ((1<<REGRESS)-1)
////////////////////////////////////////////////////////////////////////////

#ifndef TTERM1
#define TTERM1
struct tTerm1
{
    quint32 basisType : BASISTYPE;
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
    quint32 idVar   : BASISTYPE+VARIAVEL+ATRASOS;
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
    compTermo(){vTermo.tTermo0=0;expoente=0.0f;}
    compTermo(const compTermo &termo){vTermo.tTermo0=termo.vTermo.tTermo0;expoente=termo.expoente;}
    void operator=(const compTermo &termo)
    {
        vTermo.tTermo0=termo.vTermo.tTermo0;
        expoente=termo.expoente;
    }
};

#endif //compTermo

#ifndef CROMOSSOMO
#define CROMOSSOMO
class Cromossomo
{
public:
    qreal erro,
          aptidao,                //Aptidao calculada pelo metodo BIC
          theta0,                 //Offset fora da fracao: yhat = g/d + theta0
          rmse2,                  //RMSE(2) normalizado por RMS(delta2(y))
          jn2,                    //Jn2 = MSE/(varAuxDelta2 + eps)
          penDen,                 //Penalidade acumulada de denominador pequeno
          minAbsDen,              //Menor |denominador| observado
          ratioIn,                //Influencia relativa de entrada (ratio_in)
          improvementPersist;     //Melhoria vs baseline de persistencia
    qint32 idSaida,               //Indentificador da saida para este cromossomo
           maiorAtraso;
    JMathVar<qreal> err,vlrsCoefic;
    QVector<QVector<compTermo > > regress;
    //QVector<QVector<compTermo > > regressResid;
    Cromossomo(){erro=0.0f;aptidao=0.0f;theta0=0.0f;rmse2=0.0f;jn2=0.0f;penDen=0.0f;minAbsDen=0.0f;ratioIn=0.0f;improvementPersist=0.0f;idSaida=0;maiorAtraso=0;vlrsCoefic.clear();err.clear();regress.clear();}
    void operator=(const Cromossomo &cr)
    {
        erro=cr.erro;
        aptidao=cr.aptidao;
        theta0=cr.theta0;
        rmse2=cr.rmse2;
        jn2=cr.jn2;
        penDen=cr.penDen;
        minAbsDen=cr.minAbsDen;
        ratioIn=cr.ratioIn;
        improvementPersist=cr.improvementPersist;
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
    bool    isResiduo;               //true - inclui termos de residuo no modelo; false - desliga residuos
    qreal   pesoBIC;                 //0.0 a 0.999 - peso da parcimonia no BIC (0=so erro, 0.999=BIC padrao)
    qreal   epsNormJn2,              //eps para normalizacao Jn2
            epsDen,                  //eps do clamp do denominador
            denWarn,                 //limiar de aviso de denominador pequeno
            wDen,                    //peso da penalidade de denominador no fitness
            ratioMinIn,              //minimo de influencia relativa de entrada
            wInDom,                  //peso da penalidade ratio_in
            improvMinBase,           //melhoria minima sobre baseline persistente
            wBase,                   //peso da penalidade de baseline
            deltaMinResid,           //ganho minimo para aceitar termos de residuo
            wCe,                     //peso L2 dos coeficientes de residuo
            wOrder,                  //peso da ordem dos termos de residuo
            tolRatioResid,           //tolerancia de degradacao do ratio_in no estagio 2
            deF,                     //fator de exploracao atual do DE
            deCR,                    //taxa de cruzamento atual do DE
            deFMin, deFMax,          //clamps para F
            deCRMin, deCRMax,        //clamps para CR
            divGapLow, divGapHigh,   //limiares de diversidade (gap relativo)
            divFracReinit;           //fracao da populacao para reinicializacao parcial
    bool    enableDiversityControl,  //liga controle adaptativo de diversidade
            divUseJn2;               //true: usa Jn2 no gap; false: usa MSE bruto
    qint32  divStreakLow,            //geracoes consecutivas para gap baixo
            divStreakHigh,           //geracoes consecutivas para gap alto
            divCooldownGen,          //cooldown entre reinicializacoes
            divEliteCount,           //quantidade de elites preservadas na reinicializacao
            maxErroRefine;           //ordem maxima de residuo usada no estagio 2 (0=usa tamErroEfetivo)
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

    Config()
        : isIniciaEnabled(false),
          salvarAutomati(false),
          isRacional(false),
          isCriado(false),
          iteracoes(0),
          iteracoesAnt(0),
          modeOper_TH(0),
          isPararContinuarEnabled(0),
          isTipoExpo(0),
          isResiduo(true),
          pesoBIC(0.999),
          epsNormJn2(1e-12),
          epsDen(1e-8),
          denWarn(1e-7),
          wDen(1.0),
          ratioMinIn(0.05),
          wInDom(0.0),
          improvMinBase(0.05),
          wBase(0.0),
          deltaMinResid(0.01),
          wCe(0.0),
          wOrder(0.0),
          tolRatioResid(0.02),
          deF(1.0),
          deCR(0.5),
          deFMin(0.2),
          deFMax(1.0),
          deCRMin(0.1),
          deCRMax(0.95),
          divGapLow(0.05),
          divGapHigh(1.00),
          divFracReinit(0.2),
          enableDiversityControl(true),
          divUseJn2(false),
          divStreakLow(5),
          divStreakHigh(5),
          divCooldownGen(10),
          divEliteCount(3),
          maxErroRefine(0),
          qdadeCrSalvos(0),
          salvDadosCarre(0),
          numeroCiclos(0),
          serr(0.0),
          jnrr(0.0)
    {}
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
