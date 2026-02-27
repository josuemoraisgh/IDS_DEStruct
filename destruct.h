#ifndef AGSTRUCT_H
#define AGSTRUCT_H

#include <QVector>
#include <QString>
#include <QThread>
#include <QSemaphore>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>

#include <QtCore/QObject>

#include "mtrand.h"
#include "xtipodados.h"

#define TAMPIPELINE 4
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class QTextStream;
class QFile;

struct ModelEvalContext
{
    // Dados base (sem copia)
    const JMathVar<qreal> *pVlrsRegress;
    const JMathVar<qreal> *pY;
    qint32 N;

    // Selecao e mapeamento
    QVector<qint32> idxNum;
    QVector<qint32> idxDen;
    bool hasNumConst;

    // Dimensoes do vetor de parametros interno: [theta_num, theta_den, theta0, ce]
    qint32 tamNumSel;
    qint32 tamDenSel;
    qint32 mErro;
    qint32 p;

    // Buffers reutilizados
    QVector<qreal> numVec;
    QVector<qreal> denVec;
    QVector<qreal> e_hist;
    QVector< QVector<qreal> > s_hist;
    QVector<qreal> s;

    // Metricas auxiliares
    qreal minAbsDen;
    qreal penDen;

    ModelEvalContext()
        : pVlrsRegress(NULL),
          pY(NULL),
          N(0),
          hasNumConst(false),
          tamNumSel(0),
          tamDenSel(0),
          mErro(0),
          p(0),
          minAbsDen(0.0),
          penDen(0.0)
    {}
};

struct DiversityControlState
{
    qint32 lowGapStreak;
    qint32 highGapStreak;
    qint32 cooldown;
    qreal F_current;
    qreal CR_current;

    DiversityControlState()
        : lowGapStreak(0),
          highGapStreak(0),
          cooldown(0),
          F_current(1.0),
          CR_current(0.5)
    {}
};

class DEStruct : public QThread
{

    Q_OBJECT

public:
    ////////////////////////////////////////////////////////////////////////////
    DEStruct();
    ////////////////////////////////////////////////////////////////////////////
    ~DEStruct();
    ////////////////////////////////////////////////////////////////////////////
    static QSemaphore DES_justThread[TAMPIPELINE],DES_waitThread;
    static QReadWriteLock lock_DES_index[TAMPIPELINE],
                          lock_DES_Elitismo[TAMPIPELINE],
                          lock_DES_BufferSR,
                          lock_DES_modeOper_TH,
                          LerDados;
                          //lock_DES_pop[TAMPIPELINE],
                          //lock_DES_Pior[TAMPIPELINE],
    static QWaitCondition justSync,waitSync;
    static QMutex         mutex;
    ////////////////////////////////////////////////////////////////////////////
    static QList<QList<QVector<Cromossomo> > > DES_BufferSR;
    static QList<QVector<qint32 > > DES_idChange;
    static volatile qint32 DES_index[TAMPIPELINE];
    static volatile qint64 tamArquivo;//,DES_iteracoes;
    static QString DES_fileName;
    static QList<qint32> DES_cVariaveis;
    static QList<QVector<Cromossomo > > DES_crMut;
    static QList<QVector<qreal> > DES_somaSSE;
    static QList<QVector<qreal> > DES_somaJN2;
    static QList<QVector<DiversityControlState> > DES_divState;
    static QList<qreal > DES_mediaY,DES_mediaY2;
    static QList<QList<QVector<qreal> > > DES_residuos,
                                          DES_vcalc;
    static bool DES_isCarregar,DES_idParadaJust[TAMPIPELINE];//DES_isDesenhar,
    //static QTime DES_tp;
    static Config DES_Adj;
    MTRand DES_RG;
    ////////////////////////////////////////////////////////////////////////////
    void qSortPop(qint32 *start, qint32 *end, const qint32 &idSaida) const;
    ////////////////////////////////////////////////////////////////////////////
    const Cromossomo DES_criaCromossomo(const qint32 &idSaida) const;
    void DES_CruzMut(Cromossomo &crAvali,  const Cromossomo &cr0, const Cromossomo &crNew, const Cromossomo &cr1, const Cromossomo &cr2, const qint32 &idPipeLine) const;
    void DES_CalcERR(Cromossomo &cr,const qreal &metodoSerr) const;
    void DES_MontaVlrs(Cromossomo &cr,JMathVar<qreal> &vlrsRegress,JMathVar<qreal> &vlrsMedido,const bool &isValidacao=false,const bool &isLinearCoef=true) const;
    void DES_CalcVlrsEstRes(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsCoefic,const JMathVar<qreal> &vlrsMedido,JMathVar<qreal> &vlrsResiduo,JMathVar<qreal> &vlrsEstimado) const;
    ////////////////////////////////////////////////////////////////////////////
    void DES_calAptidao(Cromossomo &cr, const quint32 &tamErro=1) const;
    void DES_MontaSaida(Cromossomo &cr, QVector<qreal> &vplotar, QVector<qreal> &resid) const;
    ////////////////////////////////////////////////////////////////////////////
    void DES_removeTermo(Cromossomo &cr,const qint32 &indexTermo) const;
    void DES_removeRegress(Cromossomo &cr,const qint32 &indRegress) const;
    ////////////////////////////////////////////////////////////////////////////
    void run();
    ////////////////////////////////////////////////////////////////////////////
    static qint16 DES_modo_Oper_TH()
    {
        qint16 resp;
        lock_DES_modeOper_TH.lockForWrite();
        resp = DES_Adj.modeOper_TH;
        lock_DES_modeOper_TH.unlock();
        return resp;
    }
signals:
    void signal_DES_EscreveEquacao();
    void signal_DES_closed();
    void signal_DES_Finalizado();
    void signal_DES_Parado();
    void signal_DES_SetStatus(const volatile qint64 &iteracoes,const QVector<qreal> *somaEr,const QList<QVector<qreal> > *resObtido,const QList<QVector<qreal> > *residuo,const QVector<Cromossomo> *crBest) const;
    void signal_DES_Desenha() const;
    void signal_DES_Finalizar() const;
    void signal_DES_Status(const quint16 index) const;
    void signal_DES_Tam();
    ////////////////////////////////////////////////////////////////////////////
private slots:
    void slot_DES_Normalizar();
    void slot_DES_EquacaoEscrita();
    void slot_DES_StatusSetado();
    void slot_DES_Estado(const quint16 &std);
    ////////////////////////////////////////////////////////////////////////////
private:
    //QWaitCondition waitSync;
    XMatriz<qreal> *DES_vlrRegressores;
    volatile bool DES_isEquacaoEscrita,DES_isStatusSetado;
    void DES_Carregar();
    void DES_AlgDiffEvol();
    const JMathVar<qreal> MultMatrizResiduo(Cromossomo &cr,JMathVar<qreal> &VetResiduo, const JMathVar<qreal> &matrizRegress, const JMathVar<qreal> &vetorCoefic, const JMathVar<qreal> &vetorMedido, const QVector<QVector<qreal> > &matResiduo) const;
    void DES_CalcERRPrepared(Cromossomo &cr,const qreal &metodoSerr,JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido) const;
    void DES_calAptidaoPrepared(Cromossomo &cr,const quint32 &tamErroEfetivo,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido) const;
    bool BuildEvalContextFromChromosome(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsMedido,const qint32 &mErroEfetivo,ModelEvalContext &ctx) const;
    bool EvaluateStreaming_1plusDen(ModelEvalContext &ctx,
                                    const QVector<qreal> &Theta,
                                    const bool &needTrace,
                                    const bool &needLM,
                                    const qreal &eps,
                                    const qreal &gamma,
                                    qreal &outSSE,
                                    qreal &outPen,
                                    qreal &outMinAbsDen,
                                    JMathVar<qreal> *outJTJ=NULL,
                                    QVector<qreal> *outJTr=NULL,
                                    JMathVar<qreal> *outYhat=NULL,
                                    JMathVar<qreal> *outE=NULL) const;
    void InitThetaByLS(const ModelEvalContext &ctx,QVector<qreal> &Theta) const;
    qreal ComputeVarAuxDelta2(const JMathVar<qreal> &y,const qreal &eps) const;
    qreal ComputeMSEBaselinePersist(const JMathVar<qreal> &y,const qreal &eps) const;
    void ComputeRatioInOut(const Cromossomo &cr,const JMathVar<qreal> &vlrsCoefic,const qint32 &mErroUsed,const qint32 &qtSaidas,qreal &outEin,qreal &outEout,qreal &outRatioIn) const;
    bool EvaluateChromosomeWithErrorOrder(const Cromossomo &crBase,
                                          const JMathVar<qreal> &vlrsRegress,
                                          const JMathVar<qreal> &vlrsMedido,
                                          const qint32 &mErroEfetivo,
                                          const qreal &epsNorm,
                                          const qreal &epsDen,
                                          const qreal &wDen,
                                          const qreal &tolStep,
                                          const qint32 &maxIterLM,
                                          const Cromossomo *pSeed,
                                          Cromossomo &outCr,
                                          qreal &outCE2,
                                          qreal &outRatioIn,
                                          qreal &outLambdaFinal,
                                          qint32 &outIterLM) const;
    bool LMRefineBudget_AllChromosomes(ModelEvalContext &ctx,
                                       QVector<qreal> &Theta,
                                       const qint32 &maxIterLM,
                                       const qreal &eps,
                                       const qreal &gamma,
                                       const qreal &tolStep,
                                       qreal &outSSE,
                                       qreal &outPen,
                                       qreal &outMinAbsDen,
                                       qreal &outLambdaFinal,
                                       qint32 &outIterUsed) const;
    qreal CalcPenalidadeSemEntrada(const Cromossomo &cr,const qint32 &qtdeAtrasos) const;
    qint32 PartialReinitializePopulation(const qint32 &idPipeLine,const qint32 &idSaida,const qint32 &eliteCount,const qreal &fracReinit) const;
    void UpdateDiversityControl(const qint32 &idPipeLine,const qint32 &idSaida,const qreal &bestMetric,const qreal &meanMetric) const;
    static volatile qint16 DES_TH_size,DES_countSR;//,DES_countPipe;
    qint32 DES_TH_id;
    bool DES_idParada_Th[TAMPIPELINE];
};
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#endif // AGSTRUCT_H
