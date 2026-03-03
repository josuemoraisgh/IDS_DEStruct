#ifndef I_CHROMOSOME_SERVICE_H
#define I_CHROMOSOME_SERVICE_H

#include "xtipodados.h"

///////////////////////////////////////////////////////////////////////////////
/// Interface Segregation Principle (ISP):
///   Cada consumidor depende apenas da interface que realmente usa.
///
/// Dependency Inversion Principle (DIP):
///   Camadas superiores dependem desta abstração, não da impl concreta.
///////////////////////////////////////////////////////////////////////////////

// ─── Interface para criação de cromossomos ─────────────────────────────────
class IChromosomeFactory
{
public:
    virtual ~IChromosomeFactory() = default;
    virtual Cromossomo createRandom(qint32 idSaida) const = 0;
};

// ─── Interface para avaliação de aptidão ───────────────────────────────────
class IFitnessEvaluator
{
public:
    virtual ~IFitnessEvaluator() = default;
    virtual void evaluate(Cromossomo &cr, quint32 tamErro = 1) const = 0;
    virtual void calcERR(Cromossomo &cr, qreal metodoSerr) const = 0;
};

// ─── Interface para geração de trial (mutação + crossover) ─────────────────
class ITrialGenerator
{
public:
    virtual ~ITrialGenerator() = default;
    virtual Cromossomo generateTrial(const Cromossomo &target,
                                      const Cromossomo &pbest,
                                      const Cromossomo &r1,
                                      const Cromossomo &r2,
                                      double F, double CR) const = 0;
};

// ─── Interface para montagem de valores e saída ────────────────────────────
class IOutputBuilder
{
public:
    virtual ~IOutputBuilder() = default;
    virtual void buildOutput(Cromossomo &cr,
                             QVector<qreal> &vplotar,
                             QVector<qreal> &resid) const = 0;
};

#endif // I_CHROMOSOME_SERVICE_H
