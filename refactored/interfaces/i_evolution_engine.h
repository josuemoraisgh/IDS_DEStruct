#ifndef I_EVOLUTION_ENGINE_H
#define I_EVOLUTION_ENGINE_H

///////////////////////////////////////////////////////////////////////////////
/// Interface para o motor de evolução diferencial (SRP + DIP)
/// Responsabilidade única: executar o loop DE.
/// A implementação usa IChromosomeFactory, IFitnessEvaluator, ITrialGenerator.
///////////////////////////////////////////////////////////////////////////////
class IEvolutionEngine
{
public:
    virtual ~IEvolutionEngine() = default;

    /// Executa o algoritmo de evolução diferencial (chamado pelo thread worker)
    virtual void run() = 0;
};

#endif // I_EVOLUTION_ENGINE_H
