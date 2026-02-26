#ifndef REFACTORED_DE_SERVICE_H
#define REFACTORED_DE_SERVICE_H

#include "../interfaces/IAlgorithmService.h"
#include "../domain/Configuration.h"
#include "../domain/Chromosome.h"
#include "../services/MathematicalService.h"
#include "../utils/RandomGenerator.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <QElapsedTimer>
#include <atomic>

namespace Services {

// Forward declaration
class DEWorkerThread;

/**
 * @brief Implementação do algoritmo de Evolução Diferencial — fiel ao original DEStruct.
 *
 * Threading: toda a computação pesada roda em DEWorkerThread (QThread subclass).
 * A UI nunca bloqueia.  Stop é assíncrono (seta flag atômica).
 *
 * Algoritmo:
 *   - cruzMut: mutação/cruzamento com 5 cromossomos, rotação de bits,
 *     agregação de termos e seleção de regressores (fiel a DES_CruzMut).
 *   - calcERR:  taxa de redução de erro com Householder + segunda reordenação.
 *   - calcAptidao:  ELS Estendido (mínimos quadrados estendido).
 *   - JNRR:  parada por convergência.
 *   - Elitismo: ordenação da população por aptidão.
 *   - DES_crMut:  cromossomo aleatório recriado a cada iteração.
 */
class DifferentialEvolutionService : public Interfaces::IAlgorithmService
{
    Q_OBJECT

public:
    explicit DifferentialEvolutionService(MathematicalService* mathService,
                                         QObject* parent = nullptr);
    ~DifferentialEvolutionService() override;

    bool initialize(const Domain::Configuration& config) override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;
    Domain::Chromosome getBestChromosome(qint32 outputId) const override;

    void setMaxIterations(qint64 max) { m_maxIterations = max; }
    qint64 getMaxIterations() const { return m_maxIterations; }

    // Called by DEWorkerThread::run() — runs in worker thread
    void runAlgorithm();

private slots:
    void onWorkerFinished();

private:
    // ── Core algorithm methods (faithful to original DEStruct) ───────────
    void initializePopulation();
    void evolutionLoop();

    Domain::Chromosome createRandomChromosome(qint32 outputId);

    void cruzMut(Domain::Chromosome& crAvali,
                 const Domain::Chromosome& cr0,
                 const Domain::Chromosome& crNew,
                 const Domain::Chromosome& cr1,
                 const Domain::Chromosome& cr2);

    void montaRegressores(Domain::Chromosome& cr,
                          Utils::MathMatrix<qreal>& vlrsRegress,
                          Utils::MathVector<qreal>& vlrsMedido,
                          bool isValidacao = true,
                          bool isLinearCoef = true);

    void calcERR(Domain::Chromosome& cr, qreal serr);
    void calcAptidao(Domain::Chromosome& cr, quint32 tamErro = 1);

    void calcVlrsEstRes(const Domain::Chromosome& cr,
                        const Utils::MathMatrix<qreal>& vlrsRegress,
                        const Utils::MathVector<qreal>& vlrsCoefic,
                        const Utils::MathVector<qreal>& vlrsMedido,
                        Utils::MathVector<qreal>& vlrsResiduo,
                        Utils::MathVector<qreal>& vlrsEstimado) const;

    void montaSaida(Domain::Chromosome& cr,
                    QVector<qreal>& vplotar,
                    QVector<qreal>& resid);

    // ── Helpers ──────────────────────────────────────────────────────────
    void sortElitism(QVector<qint32>& indices, qint32 outputId) const;
    static void dualSort(QVector<Domain::CompositeTerm>& terms,
                         QVector<qint32>& labels);
    static bool compareCoefficients(const Utils::MathVector<qreal>& a,
                                    const Utils::MathVector<qreal>& b,
                                    qreal tolerance);
    static qreal computeVariance(const Utils::MathVector<qreal>& vec);
    void emitProgress();
    bool shouldStop() const;

private:
    MathematicalService* m_mathService;
    Utils::RandomGenerator m_randomGen;

    Domain::Configuration m_config;
    bool m_initialized;

    // Atomic flags — no mutex needed, no deadlock
    std::atomic<bool>   m_running;
    std::atomic<bool>   m_paused;
    std::atomic<bool>   m_stopRequested;
    std::atomic<qint64> m_currentIteration;

    qint64 m_maxIterations;

    // Population per output — accessed exclusively by worker thread
    QList<QVector<Domain::Chromosome>> m_population;

    // Elitism indices per output (sorted by fitness, like vetElitismo)
    QList<QVector<qint32>> m_elitismIndices;

    // Fresh random chromosome per output (recreated each iteration, like DES_crMut)
    QVector<Domain::Chromosome> m_crMut;

    // JNRR convergence tracking
    QVector<qreal> m_bestPreviousFitness;
    qint64 m_previousIterations;

    // SSE accumulator per output
    QVector<qreal> m_somaSSE;

    // Best results — mutex-protected for cross-thread UI access
    mutable QMutex m_resultMutex;
    QList<Domain::Chromosome> m_bestChromosomes;
    QVector<qreal> m_bestErrors;

    // Pause mechanism
    QMutex m_pauseMutex;
    QWaitCondition m_pauseCondition;

    // Worker thread
    DEWorkerThread* m_workerThread;
};

////////////////////////////////////////////////////////////////////////////////
/// DEWorkerThread — dedicated QThread subclass
////////////////////////////////////////////////////////////////////////////////
class DEWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit DEWorkerThread(DifferentialEvolutionService* service, QObject* parent = nullptr)
        : QThread(parent), m_service(service) {}

protected:
    void run() override { m_service->runAlgorithm(); }

private:
    DifferentialEvolutionService* m_service;
};

} // namespace Services

#endif // REFACTORED_DE_SERVICE_H
