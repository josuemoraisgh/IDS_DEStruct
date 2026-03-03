#ifndef EVOLUTION_ENGINE_H
#define EVOLUTION_ENGINE_H

#include "../interfaces/i_evolution_engine.h"
#include "mtrand.h"
#include "xtipodados.h"

#ifndef TAMPIPELINE
#define TAMPIPELINE 4
#endif

class SharedState;
class ChromosomeService;
class ThreadWorker;

///////////////////////////////////////////////////////////////////////////////
/// EvolutionEngine — Implementa IEvolutionEngine
///
/// SRP: Responsabilidade única = executar o loop do DE.
/// Extrai DES_AlgDiffEvol() do DEStruct original.
/// DIP: Depende de SharedState + ChromosomeService (injetados).
///////////////////////////////////////////////////////////////////////////////
class EvolutionEngine : public IEvolutionEngine
{
public:
    EvolutionEngine(SharedState *state, ChromosomeService *chromoSvc, ThreadWorker *worker);

    void run() override;

private:
    SharedState      *m_state;
    ChromosomeService *m_chromoSvc;
    ThreadWorker     *m_worker;
    MTRand            m_rng;
    bool              m_idParada_Th[TAMPIPELINE];
};

#endif // EVOLUTION_ENGINE_H
