#ifndef CHROMOSOME_SERVICE_H
#define CHROMOSOME_SERVICE_H

#include "../interfaces/i_chromosome_service.h"
#include "mtrand.h"
#include "model_pruning_config.h"
#include "xmatriz.h"
#include "xvetor.h"

class SharedState;

///////////////////////////////////////////////////////////////////////////////
/// Funções auxiliares de termos (extraídas de destruct.cpp, inalteradas)
///////////////////////////////////////////////////////////////////////////////
inline qreal sign(const qreal &x)
{
    if (x > 0) return 1;
    else if (x == 0) return 0;
    else return -1;
}

inline const compTermo XInv(compTermo var1)
{
    compTermo var;
    var.vTermo.tTermo0 = var1.vTermo.tTermo0;
    var.expoente = (-1) * var1.expoente;
    return var;
}

inline const compTermo XSum(const compTermo &var1, const compTermo &var2)
{
    compTermo var;
    if ((var1.vTermo.tTermo0 && var2.vTermo.tTermo0) ?
        (var1.vTermo.tTermo2.idVar != var2.vTermo.tTermo2.idVar) : false)
        qDebug() << "Error:XSum Termos var1 != var2";
    else {
        var.vTermo.tTermo0 = var1.vTermo.tTermo0;
        var.expoente = var1.expoente + var2.expoente;
    }
    return var;
}

inline const compTermo XMult(const quint32 &var1, const compTermo &var2)
{
    compTermo var;
    var.vTermo.tTermo0 = var2.vTermo.tTermo0;
    var.expoente = var1 * var2.expoente;
    return var;
}

inline bool CmpMaiorTerm(const compTermo &vlr1, const compTermo &vlr2)
{
    return (vlr1.vTermo.tTermo0 > vlr2.vTermo.tTermo0);
}

inline bool CmpMaiorAptShared(const Cromossomo &cr1, const Cromossomo &cr2)
{
    return (cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}

///////////////////////////////////////////////////////////////////////////////
/// qSortDuplo — QuickSort duplo (idêntico ao original)
///////////////////////////////////////////////////////////////////////////////
void qSortDuplo(compTermo *start, compTermo *end,
                qint32 *posStart, qint32 *posEnd,
                bool (*compare)(const compTermo &, const compTermo &));

///////////////////////////////////////////////////////////////////////////////
/// ChromosomeService — Implementa IChromosomeFactory + IFitnessEvaluator
///                     + ITrialGenerator + IOutputBuilder
///
/// SRP: Responsabilidade = operações sobre cromossomos.
/// Depende de SharedState (injetado via construtor) — DIP.
/// Algoritmos EXATAMENTE iguais ao original.
///////////////////////////////////////////////////////////////////////////////
class ChromosomeService
    : public IChromosomeFactory
    , public IFitnessEvaluator
    , public ITrialGenerator
    , public IOutputBuilder
{
public:
    explicit ChromosomeService(SharedState *state);

    // ─── IChromosomeFactory ────────────────────────────────────────────
    Cromossomo createRandom(qint32 idSaida) const override;

    // ─── IFitnessEvaluator ─────────────────────────────────────────────
    void evaluate(Cromossomo &cr, quint32 tamErro = 1) const override;
    void calcERR(Cromossomo &cr, qreal metodoSerr) const override;

    // ─── ITrialGenerator ───────────────────────────────────────────────
    Cromossomo generateTrial(const Cromossomo &target,
                              const Cromossomo &pbest,
                              const Cromossomo &r1,
                              const Cromossomo &r2,
                              double F, double CR) const override;

    // ─── IOutputBuilder ────────────────────────────────────────────────
    void buildOutput(Cromossomo &cr,
                     QVector<qreal> &vplotar,
                     QVector<qreal> &resid) const override;

    // ─── Métodos internos usados pelo engine ───────────────────────────
    void montaVlrs(Cromossomo &cr, JMathVar<qreal> &vlrsRegress,
                   JMathVar<qreal> &vlrsMedido,
                   bool isValidacao = false,
                   bool isLinearCoef = true) const;

    void calcVlrsEstRes(const Cromossomo &cr,
                        const JMathVar<qreal> &vlrsRegress,
                        const JMathVar<qreal> &vlrsCoefic,
                        const JMathVar<qreal> &vlrsMedido,
                        JMathVar<qreal> &vlrsResiduo,
                        JMathVar<qreal> &vlrsEstimado) const;


    void qSortPop(qint32 *start, qint32 *end, qint32 idSaida) const;

    // ─── Análise de importância de termos e poda probabilística ────────
    /// @brief Calcula importância relativa de cada termo em cada regressor
    ///        baseada em sensibilidade (quanto piora sem o termo)
    /// @param cr cromossomo a analisar
    /// @param termImportance vetor de vetores: termImportance[regIdx][termIdx] = importância [0,1]
    /// @return true se análise bem-sucedida
    bool calculateTermImportance(const Cromossomo &cr,
                                  QVector<QVector<qreal>> &termImportance) const;

    /// @brief Avalia MSE removendo temporariamente um termo específico
    /// @param cr cromossomo (com todos os termos)
    /// @param regIdx índice do regressor
    /// @param termIdx índice do termo dentro do regressor
    /// @return MSE com o termo removido (ou -1 se erro)
    qreal evaluateWithoutTerm(const Cromossomo &cr,
                               qint32 regIdx,
                               qint32 termIdx) const;

    /// @brief Poda probabilística: remove termos com baixa importância usando roleta
    /// @param cr cromossomo (será modificado)
    /// @param importanceThreshold [0,1] - termos com importância < limiar tem chance de remoção
    /// @param removalRate [0,1] - fração máxima de termos a remover por regressor
    /// @return número de termos removidos
    qint32 probabilisticTermPruning(Cromossomo &cr,
                                     qreal importanceThreshold = ModelPruningConfig::kDefaultImportanceThreshold,
                                     qreal removalRate = ModelPruningConfig::kDefaultRemovalRate) const;

private:
    SharedState *m_state;
    mutable MTRand m_rng;
};

#endif // CHROMOSOME_SERVICE_H
