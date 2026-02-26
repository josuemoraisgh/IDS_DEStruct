#include "DifferentialEvolutionService.h"
#include <QtMath>
#include <cmath>
#include <QDebug>
#include <QMetaType>
#include <QElapsedTimer>
#include <QTime>
#include <algorithm>
#include <limits>

namespace Services {

////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
////////////////////////////////////////////////////////////////////////////////

DifferentialEvolutionService::DifferentialEvolutionService(MathematicalService* mathService,
                                                         QObject* parent)
    : IAlgorithmService(parent)
    , m_mathService(mathService)
    , m_initialized(false)
    , m_running(false)
    , m_paused(false)
    , m_stopRequested(false)
    , m_currentIteration(0)
    , m_maxIterations(10000)
    , m_previousIterations(0)
    , m_workerThread(nullptr)
{
    qRegisterMetaType<QVector<qreal>>("QVector<qreal>");
    qRegisterMetaType<QVector<Domain::Chromosome>>("QVector<Domain::Chromosome>");
}

DifferentialEvolutionService::~DifferentialEvolutionService()
{
    m_stopRequested.store(true);
    m_paused.store(false);
    m_pauseCondition.wakeAll();
    if (m_workerThread) {
        m_workerThread->wait(10000);
        delete m_workerThread;
        m_workerThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Public interface
////////////////////////////////////////////////////////////////////////////////

bool DifferentialEvolutionService::initialize(const Domain::Configuration& config)
{
    if (m_running.load()) {
        emit errorOccurred(QString::fromUtf8("Algoritmo em execução — pare antes de reinicializar"));
        return false;
    }
    if (!config.isCreated()) {
        emit errorOccurred(QString::fromUtf8("Configuração não foi criada"));
        return false;
    }
    if (config.getAlgorithmData().populationSize <= 0) {
        emit errorOccurred(QString::fromUtf8("Tamanho da população inválido"));
        return false;
    }

    m_config = config;
    m_initialized = true;
    m_currentIteration.store(0);
    m_previousIterations = 0;

    if (m_config.getCycleCount() > 0)
        m_maxIterations = m_config.getCycleCount();

    const auto& algorithmData = m_config.getAlgorithmData();
    qint32 outputCount = algorithmData.variables.getOutputCount();
    if (outputCount <= 0) outputCount = 1;

    m_population.clear();
    m_elitismIndices.clear();
    m_crMut.clear();
    m_somaSSE.clear();
    m_bestPreviousFitness.clear();

    {
        QMutexLocker locker(&m_resultMutex);
        m_bestChromosomes.clear();
        m_bestErrors.clear();
        for (qint32 i = 0; i < outputCount; ++i) {
            m_bestChromosomes.append(Domain::Chromosome());
            m_bestErrors.append(std::numeric_limits<qreal>::max());
        }
    }

    for (qint32 i = 0; i < outputCount; ++i) {
        m_population.append(QVector<Domain::Chromosome>());
        m_elitismIndices.append(QVector<qint32>());
        m_somaSSE.append(0.0);
        m_bestPreviousFitness.append(std::numeric_limits<qreal>::max());
    }

    qDebug() << "[DE] initialize: outputCount=" << outputCount
             << "popSize=" << algorithmData.populationSize
             << "maxIter=" << m_maxIterations;
    return true;
}

void DifferentialEvolutionService::start()
{
    if (!m_initialized) {
        emit errorOccurred(QString::fromUtf8("Algoritmo não inicializado"));
        return;
    }
    if (m_running.load()) return;

    m_running.store(true);
    m_stopRequested.store(false);
    m_paused.store(false);

    if (m_workerThread) {
        m_workerThread->wait(1000);
        delete m_workerThread;
        m_workerThread = nullptr;
    }

    m_workerThread = new DEWorkerThread(this);
    connect(m_workerThread, &QThread::finished,
            this, &DifferentialEvolutionService::onWorkerFinished,
            Qt::QueuedConnection);
    m_workerThread->start();
}

void DifferentialEvolutionService::stop()
{
    // Never blocks the main thread — just sets flag
    m_stopRequested.store(true);
    m_paused.store(false);
    m_pauseCondition.wakeAll();
}

void DifferentialEvolutionService::onWorkerFinished()
{
    m_running.store(false);
    if (m_workerThread) {
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }
    emit finished();
}

void DifferentialEvolutionService::pause()
{
    if (m_running.load()) {
        m_paused.store(true);
        emit paused();
    }
}

void DifferentialEvolutionService::resume()
{
    if (m_paused.load()) {
        m_paused.store(false);
        m_pauseCondition.wakeAll();
    }
}

bool DifferentialEvolutionService::isRunning() const { return m_running.load(); }

Domain::Chromosome DifferentialEvolutionService::getBestChromosome(qint32 outputId) const
{
    QMutexLocker locker(&m_resultMutex);
    if (outputId >= 0 && outputId < m_bestChromosomes.size())
        return m_bestChromosomes[outputId];
    return Domain::Chromosome();
}

////////////////////////////////////////////////////////////////////////////////
// Worker thread entry point
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::runAlgorithm()
{
    QElapsedTimer timer;
    timer.start();

    qDebug() << "[DE] runAlgorithm START — popSize="
             << m_config.getAlgorithmData().populationSize
             << "outputs=" << m_config.getAlgorithmData().variables.getOutputCount()
             << "rows=" << m_config.getAlgorithmData().variables.getValues().numRows()
             << "cols=" << m_config.getAlgorithmData().variables.getValues().numCols()
             << "NCy=" << m_config.getCycleCount()
             << "JNRR=" << m_config.getJNRR()
             << "SSE=" << m_config.getSSE();

    try {
        initializePopulation();
        if (shouldStop()) {
            qDebug() << "[DE] Stopped during initialization";
            return;
        }

        qDebug() << "[DE] Population initialized in" << timer.elapsed() << "ms";

        // Emit initial progress immediately (so UI shows something)
        for (qint32 idSaida = 0; idSaida < m_population.size(); ++idSaida) {
            qint32 bestIdx = m_elitismIndices[idSaida].isEmpty() ? 0 : m_elitismIndices[idSaida][0];
            if (bestIdx < m_population[idSaida].size()) {
                QMutexLocker locker(&m_resultMutex);
                m_bestChromosomes[idSaida] = m_population[idSaida][bestIdx];
                m_bestErrors[idSaida] = m_population[idSaida][bestIdx].getFitness();
            }
        }
        emitProgress();

        evolutionLoop();

        qDebug() << "[DE] evolutionLoop finished, iterations=" << m_currentIteration.load()
                 << "elapsed=" << timer.elapsed() << "ms";
        emitProgress();

    } catch (const std::exception& ex) {
        qDebug() << "[DE] EXCEPTION:" << ex.what();
        emit errorOccurred(QString("Exceção: %1").arg(ex.what()));
    } catch (...) {
        qDebug() << "[DE] UNKNOWN EXCEPTION";
        emit errorOccurred("Exceção desconhecida");
    }

    m_running.store(false);
}

////////////////////////////////////////////////////////////////////////////////
// Population initialization — faithful to DES_AlgDiffEvol init phase
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::initializePopulation()
{
    const auto& algorithmData = m_config.getAlgorithmData();
    const qint32 populationSize = algorithmData.populationSize;
    qint32 outputCount = algorithmData.variables.getOutputCount();
    if (outputCount <= 0) outputCount = 1;

    // Create initial population and evaluate
    for (qint32 output = 0; output < outputCount; ++output) {
        m_population[output].clear();
        m_population[output].reserve(populationSize);
        for (qint32 i = 0; i < populationSize; ++i) {
            if (shouldStop()) return;
            Domain::Chromosome cr = createRandomChromosome(output);
            m_population[output].append(cr);
        }
    }

    // Initialize elitism indices (like original: 1, 0, 2, 3, ..., n-1)
    for (qint32 output = 0; output < outputCount; ++output) {
        m_elitismIndices[output].clear();
        if (populationSize > 1) {
            m_elitismIndices[output].append(1);
            m_elitismIndices[output].append(0);
            for (qint32 i = 2; i < populationSize; ++i)
                m_elitismIndices[output].append(i);
        } else {
            m_elitismIndices[output].append(0);
        }
    }

    // Initialize crMut per output (fresh random chromosomes, like original)
    m_crMut.resize(outputCount);
    for (qint32 output = 0; output < outputCount; ++output) {
        m_crMut[output] = createRandomChromosome(output);
        m_bestPreviousFitness[output] = m_crMut[output].getFitness();
    }

    // Reset SSE
    for (qint32 i = 0; i < m_somaSSE.size(); ++i)
        m_somaSSE[i] = 0.0;
}

////////////////////////////////////////////////////////////////////////////////
// Main evolution loop — faithful to DES_AlgDiffEvol forever loop
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::evolutionLoop()
{
    const qint32 outputCount = m_population.size();
    const qint32 tamPop = m_config.getAlgorithmData().populationSize;

    QElapsedTimer printTimer;
    printTimer.start();
    bool isPrint = true;

    QVector<Domain::Chromosome> crBest(outputCount);

    while (!shouldStop()) {
        // Check pause
        if (m_paused.load()) {
            QMutexLocker locker(&m_pauseMutex);
            while (m_paused.load() && !m_stopRequested.load())
                m_pauseCondition.wait(&m_pauseMutex, 200);
        }
        if (shouldStop()) break;

        // ── Process each individual in the population ─────────────────
        for (qint32 tokenPop = 0; tokenPop < tamPop; ++tokenPop) {
            if (shouldStop()) return;

            for (qint32 idSaida = 0; idSaida < outputCount; ++idSaida) {
                // Select 3 random individuals from elitism (like original)
                qint32 count0 = m_randomGen.randInt(0, tamPop - 1);
                qint32 count1;
                do { count1 = m_randomGen.randInt(0, tamPop - 1); } while (count1 == count0);
                qint32 count2;
                do { count2 = m_randomGen.randInt(0, tamPop - 1); } while (count2 == count0 || count2 == count1);

                // Get chromosomes via elitism indices (like original)
                qint32 cr0Point = m_elitismIndices[idSaida][count0];
                qint32 cr1Point = m_elitismIndices[idSaida][count1];
                qint32 cr2Point = m_elitismIndices[idSaida][count2];

                Domain::Chromosome cr0 = m_population[idSaida][cr0Point];
                Domain::Chromosome cr1 = m_population[idSaida][cr1Point];
                Domain::Chromosome cr2 = m_population[idSaida][cr2Point];

                // Faithful: DES_CruzMut(Pop[idSaida][tokenPop], cr0, DES_crMut[idSaida], cr1, cr2)
                cruzMut(m_population[idSaida][tokenPop], cr0, m_crMut[idSaida], cr1, cr2);

                // Accumulate SSE
                m_somaSSE[idSaida] += m_population[idSaida][tokenPop].getError();
            }
        }

        // ── After processing all individuals ──────────────────────────
        // isOk = true means ALL outputs improved by at least JNRR (faithful to original)
        bool isOk = true;
        for (qint32 idSaida = 0; idSaida < outputCount; ++idSaida) {
            // Previous best
            qint32 prevBestIdx = m_elitismIndices[idSaida].isEmpty() ? 0 : m_elitismIndices[idSaida][0];

            // Sort elitism (like qSortPop in original)
            sortElitism(m_elitismIndices[idSaida], idSaida);

            qint32 bestIdx = m_elitismIndices[idSaida][0];
            crBest[idSaida] = m_population[idSaida][bestIdx];

            // JNRR convergence check (faithful to original)
            // Original: if improvement >= jnrr → update; else isOk = false
            if ((m_bestPreviousFitness[idSaida] - crBest[idSaida].getFitness()) >= m_config.getJNRR()) {
                m_bestPreviousFitness[idSaida] = crBest[idSaida].getFitness();
            } else {
                isOk = false; // this output did NOT improve enough
            }

            // Check if plot should update
            if (prevBestIdx < m_population[idSaida].size()) {
                const auto& prevBest = m_population[idSaida][prevBestIdx];
                isPrint = isPrint || (crBest[idSaida].getFitness() < prevBest.getFitness());
            }

            // Create fresh random chromosome (like DES_crMut = DES_criaCromossomo(idSaida))
            m_crMut[idSaida] = createRandomChromosome(idSaida);
        }

        // Increment iteration
        qint64 iter = m_currentIteration.fetch_add(1) + 1;

        // JNRR stop condition (faithful to original):
        // if ALL outputs improved → reset convergence counter
        // else if stagnant for numeroCiclos iterations → stop
        if (isOk) {
            m_previousIterations = iter;
        } else {
            if (m_config.getCycleCount() > 0 &&
                iter >= m_previousIterations + (qint64)m_config.getCycleCount()) {
                qDebug() << "[DE] JNRR convergence: stopping after" << iter << "iterations";
                break; // Stop after numeroCiclos iterations without ALL outputs improving
            }
        }

        // Emit progress periodically (like original: 6s if improved, 60s always)
        // Reduced from 6s to 2s for faster UI feedback
        bool shouldEmit = (printTimer.elapsed() >= 2000 && isPrint) ||
                          (printTimer.elapsed() >= 30000);
        if (shouldEmit) {
            printTimer.restart();
            for (qint32 idSaida = 0; idSaida < outputCount; ++idSaida) {
                // Recalc with tamErro=15 for display (like original)
                calcAptidao(crBest[idSaida], 15);
                {
                    QMutexLocker locker(&m_resultMutex);
                    m_bestChromosomes[idSaida] = crBest[idSaida];
                    m_bestErrors[idSaida] = crBest[idSaida].getFitness();
                }
            }
            qDebug() << "[DE] Progress: iter=" << iter
                     << "bestFitness=" << crBest[0].getFitness();
            emitProgress();
            isPrint = false;
        }

        // Reset SSE
        for (qint32 i = 0; i < m_somaSSE.size(); ++i)
            m_somaSSE[i] = 0.0;
    }

    // Final progress update
    for (qint32 idSaida = 0; idSaida < outputCount; ++idSaida) {
        qint32 bestIdx = m_elitismIndices[idSaida].isEmpty() ? 0 : m_elitismIndices[idSaida][0];
        if (bestIdx < m_population[idSaida].size()) {
            Domain::Chromosome best = m_population[idSaida][bestIdx];
            calcAptidao(best, 15);
            QMutexLocker locker(&m_resultMutex);
            m_bestChromosomes[idSaida] = best;
            m_bestErrors[idSaida] = best.getFitness();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// createRandomChromosome — faithful to DES_criaCromossomo
// Key fixes: random reg IDs (not sequential), err.fill(-1)
////////////////////////////////////////////////////////////////////////////////

Domain::Chromosome DifferentialEvolutionService::createRandomChromosome(qint32 outputId)
{
    Domain::Chromosome cr;
    cr.setOutputId(outputId);

    // Local RNG seeded from current time (like original MTRand)
    Utils::RandomGenerator RG(QTime::currentTime().msec());

    const qint32 numVariaveis = m_config.getAlgorithmData().variables.getValues().numRows();
    const qint32 numAmostras  = m_config.getAlgorithmData().variables.getValues().numCols();
    const quint32 vlrMaxAtras = numAmostras < 60 ? numAmostras / 2 : 30;
    const bool isRacional = m_config.isRational();

    QVector<QVector<Domain::CompositeTerm>> regressions;

    // Regressor 0 — constant coefficient (like original)
    {
        QVector<Domain::CompositeTerm> constReg;
        Domain::CompositeTerm t;
        t.setVariable(1);
        t.setDelay(0);
        t.setRegression(0); // reg=0 means constant
        t.setNumerator(true);
        t.setExponent(1.0);
        constReg.append(t);
        regressions.append(constReg);
    }

    // Normal regressors (like original: 0..3 terms, each with 0..3 composite terms)
    qint32 tamCrom = RG.randInt(0, 3);
    for (qint32 c = 0; c <= tamCrom; ++c) {
        QVector<Domain::CompositeTerm> regression;

        // FIXED: random reg ID, faithful to RG.randInt(1,(MASKREG/2)-1)
        // MASKREG = (1<<9)-1 = 511, so (MASKREG/2)-1 = 254
        bool nd = isRacional ? (RG.randInt(0, 1) == 1) : true;
        quint32 regId = RG.randUInt(1, (MASKREG / 2) - 1);

        qint32 tamRegress = RG.randInt(0, 3);
        for (qint32 r = 0; r <= tamRegress; ++r) {
            Domain::CompositeTerm t;
            t.setNumerator(nd);
            t.setRegression(regId);
            t.setVariable(RG.randUInt(1, numVariaveis > 0 ? numVariaveis : 1));
            quint32 atraso = RG.randUInt(1, vlrMaxAtras > 0 ? vlrMaxAtras : 1);
            t.setDelay(atraso);
            qreal expo = (qreal)RG.randInt(1, 10);
            if (expo == 0.0) expo = 1.0;
            t.setExponent(expo);

            // Randomly assign basis function type
            // ~60% BasisPow (original), ~10% each of the others
            quint32 basisRoll = RG.randUInt(0, 9);
            if (basisRoll < 6)      t.setBasisType(Domain::BasisPow);
            else if (basisRoll < 7) t.setBasisType(Domain::BasisAbsPow);
            else if (basisRoll < 8) t.setBasisType(Domain::BasisLogPow);
            else if (basisRoll < 9) t.setBasisType(Domain::BasisExp);
            else                    t.setBasisType(Domain::BasisTanhPow);

            // For BasisExp, alpha (stored in exponent) should be small
            if (t.getBasisType() == Domain::BasisExp)
                t.setExponent((qreal)RG.randInt(-3, 3));

            regression.append(t);

            if ((qint32)atraso > cr.getMaxDelay())
                cr.setMaxDelay(atraso);
        }

        // Sort descending and remove duplicates (like original)
        std::sort(regression.begin(), regression.end(),
                  [](const Domain::CompositeTerm& a, const Domain::CompositeTerm& b) {
                      return a.getCompact() > b.getCompact();
                  });
        for (qint32 i = 1; i < regression.size(); ++i) {
            if (regression[i].getCompact() == regression[i - 1].getCompact())
                regression.remove(i--);
        }
        regressions.append(regression);
    }

    cr.setRegressions(regressions);

    // FIXED: cr.err.fill(-1, cr.regress.size()) — faithful to original
    Utils::MathVector<qreal> initErrors(regressions.size(), -1.0);
    cr.setErrors(initErrors);

    // Calculate fitness (like DES_criaCromossomo calls DES_calAptidao)
    calcAptidao(cr);
    return cr;
}

////////////////////////////////////////////////////////////////////////////////
// cruzMut — FAITHFUL to DES_CruzMut
// Takes 5 chromosomes, aggregates all terms, applies mutation via bit rotation,
// selects regressors, calls calcERR + calcAptidao, renumbers reg IDs.
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::cruzMut(Domain::Chromosome& crAvali,
                                           const Domain::Chromosome& cr0,
                                           const Domain::Chromosome& crNew,
                                           const Domain::Chromosome& cr1,
                                           const Domain::Chromosome& cr2)
{
    // Save crAvali before modification (like original: crAvali0 = crAvali)
    Domain::Chromosome crAvali0 = crAvali;
    Domain::Chromosome crA1;
    crA1.setOutputId(crAvali.getOutputId());

    // Local RNG (like original MTRand)
    Utils::RandomGenerator RG(QTime::currentTime().msec());
    const qreal multBase = RG.randReal(-2.0, 2.0);

    QVector<Domain::CompositeTerm> termosAnalisados;
    QVector<qint32> posTermosAnalisados;

    // Generate bit rotation mask with exactly 5 set bits (faithful)
    qint32 count = 0, testeSize = 0, teste;
    teste = RG.randInt(0, 1);
    if (teste & 1) count++;
    for (testeSize = 1; count < 5; testeSize++) {
        teste = (teste << 1) + RG.randInt(0, 1);
        if (teste & 1) count++;
    }

    // Aggregate ALL terms from all 5 chromosomes with source labels
    // Label 0=cr0, 1=crNew, 2=cr1, 3=cr2, 4=crAvali0
    auto aggregate = [&](const Domain::Chromosome& chromosome, qint32 label) {
        const auto& regs = chromosome.getRegressions();
        for (qint32 i = 0; i < regs.size(); ++i) {
            for (qint32 j = 0; j < regs[i].size(); ++j) {
                termosAnalisados.append(regs[i][j]);
                posTermosAnalisados.append(label);
            }
        }
    };

    aggregate(cr0, 0);
    aggregate(crNew, 1);
    aggregate(cr1, 2);
    aggregate(cr2, 3);
    aggregate(crAvali0, 4);

    if (termosAnalisados.isEmpty()) return;

    // Sort all terms by compact value descending, keeping labels in sync
    dualSort(termosAnalisados, posTermosAnalisados);

    // Process terms, grouping by compact value and regressor group ID
    QVector<Domain::CompositeTerm> termoAv(6);  // Slots: 0-4=sources, 5=current
    QVector<Domain::CompositeTerm> vetTermo1, vetTermo2, vetTermo3;

    termoAv[5] = termosAnalisados[0];
    termoAv[posTermosAnalisados[0]] = termosAnalisados[0];

    const qint32 totalTerms = termosAnalisados.size();

    for (qint32 idx = 1; idx <= totalTerms; ++idx) {
        const bool isEnd = (idx >= totalTerms);

        Domain::CompositeTerm currentTerm;
        qint32 currentLabel = 0;
        if (!isEnd) {
            currentTerm = termosAnalisados[idx];
            currentLabel = posTermosAnalisados[idx];
        }

        // Check if compact value changed (new term group)
        const bool compactChanged = isEnd ||
            (termoAv[5].getCompact() != currentTerm.getCompact());

        if (compactChanged) {
            // Process completed group: apply DE mutation to terms from labels 0-3
            if (termoAv[0].getCompact() || termoAv[1].getCompact() ||
                termoAv[2].getCompact() || termoAv[3].getCompact())
            {
                Domain::CompositeTerm auxTermo;
                if (termoAv[0].getCompact()) auxTermo.setCompact(termoAv[0].getCompact());
                if (termoAv[1].getCompact()) auxTermo.setCompact(termoAv[1].getCompact());
                if (termoAv[2].getCompact()) auxTermo.setCompact(termoAv[2].getCompact());
                if (termoAv[3].getCompact()) auxTermo.setCompact(termoAv[3].getCompact());

                // Inherit basisType from the first non-empty source
                for (qint32 s = 0; s < 4; ++s) {
                    if (termoAv[s].getCompact()) {
                        auxTermo.setBasisType(termoAv[s].getBasisType());
                        break;
                    }
                }

                // DE mutation:  exp = exp[0] + mult*(exp[1]-exp[0]) + mult*(exp[2]-exp[3])
                qreal newExp = termoAv[0].getExponent()
                    + multBase * (termoAv[1].getExponent() - termoAv[0].getExponent())
                    + multBase * (termoAv[2].getExponent() - termoAv[3].getExponent());

                // Rotate bits
                teste = (teste >> 1) | ((teste & 1) << testeSize);

                if (newExp != 0.0) {
                    auxTermo.setExponent(newExp);
                    vetTermo1.append(auxTermo);
                }

                // Reset slots 0-3
                for (qint32 s = 0; s < 4; ++s) {
                    termoAv[s].setCompact(0);
                    termoAv[s].setExponent(0.0);
                }
            }

            // Process crAvali0 terms (slot 4)
            if (termoAv[4].getCompact()) {
                vetTermo2.append(termoAv[4]);
                termoAv[4].setCompact(0);
                termoAv[4].setExponent(0.0);
            }
        }

        // Check if regressor group ID changed
        const bool regGroupChanged = isEnd ||
            (termoAv[5].getRegGroupId() != currentTerm.getRegGroupId());

        if (regGroupChanged) {
            const qint32 size1 = vetTermo1.size();
            const qint32 size2 = vetTermo2.size();
            const qint32 size3 = size1 + size2;

            if (size1 || size2) {
                // Select from mutated terms using bit rotation
                vetTermo3.clear();
                teste = (teste >> 1) | ((teste & 1) << testeSize);
                for (qint32 i = 0; i < size1; ++i)
                    if ((teste >> i) & 1) vetTermo3.append(vetTermo1[i]);
                crA1.addRegression(vetTermo3);

                // If first term has reg != 0 (not constant), also consider crAvali0 terms
                bool hasNonConst = vetTermo3.isEmpty() ? true : (vetTermo3[0].getRegression() != 0);
                if (hasNonConst) {
                    // Select from crAvali0 terms
                    vetTermo3.clear();
                    teste = (teste >> 1) | ((teste & 1) << testeSize);
                    for (qint32 i = 0; i < size2; ++i)
                        if ((teste >> i) & 1) vetTermo3.append(vetTermo2[i]);
                    crA1.addRegression(vetTermo3);

                    bool hasNonConst2 = vetTermo3.isEmpty() ? true : (vetTermo3[0].getRegression() != 0);
                    if (hasNonConst2) {
                        // Merge and select from combined
                        vetTermo3.clear();
                        teste = (teste >> 1) | ((teste & 1) << testeSize);
                        QVector<Domain::CompositeTerm> combined = vetTermo1;
                        combined += vetTermo2;
                        for (qint32 i = 0; i < size3; ++i)
                            if ((teste >> i) & 1) vetTermo3.append(combined[i]);
                        // Sort and remove duplicates
                        std::sort(vetTermo3.begin(), vetTermo3.end(),
                                  [](const Domain::CompositeTerm& a, const Domain::CompositeTerm& b) {
                                      return a.getCompact() > b.getCompact();
                                  });
                        for (qint32 i = 1; i < vetTermo3.size(); ++i)
                            if (vetTermo3[i].getCompact() == vetTermo3[i - 1].getCompact())
                                vetTermo3.remove(i--);
                        crA1.addRegression(vetTermo3);
                    }
                }
                vetTermo1.clear();
                vetTermo2.clear();
            }
        }

        // Update tracking for next iteration
        if (!isEnd) {
            termoAv[5] = currentTerm;
            if (termoAv[currentLabel].getCompact() == currentTerm.getCompact())
                termoAv[currentLabel].setExponent(
                    termoAv[currentLabel].getExponent() + currentTerm.getExponent());
            else
                termoAv[currentLabel] = currentTerm;
        }
    }

    // Limit regressors by available data (faithful: qtdeAtrasos - 27)
    qint32 dec = 1;
    const auto& decVec = m_config.getDecimation();
    if (crA1.getOutputId() < decVec.size() && decVec[crA1.getOutputId()] > 0)
        dec = decVec[crA1.getOutputId()];
    const qint32 totalCols = m_config.getAlgorithmData().variables.getValues().numCols();
    const qint32 qtdeAtrasos = (totalCols / (2 * dec)) - 27;

    auto regsA1 = crA1.getRegressions();
    const qint32 regsSize = regsA1.size();
    QVector<QVector<Domain::CompositeTerm>> matTermo = regsA1;
    regsA1.clear();

    // Bit selection for first testeSize regressors
    count = 0;
    for (qint32 i = 0; i < regsSize && i <= testeSize && count < qtdeAtrasos; ++i) {
        if ((teste >> i) & 1) {
            regsA1.append(matTermo[i]);
            count++;
        }
    }
    // Random selection for remaining
    for (qint32 i = testeSize + 1; i < regsSize && count < qtdeAtrasos; ++i) {
        if (RG.randInt(0, 1)) {
            regsA1.append(matTermo[i]);
            count++;
        }
    }

    crA1.setRegressions(regsA1);

    // Apply ERR and fitness (like original)
    calcERR(crA1, m_config.getSSE());
    calcAptidao(crA1);

    // Renumber regressor IDs in reverse (like original: size-i)
    regsA1 = crA1.getRegressions();
    const qint32 finalSize = regsA1.size();
    if (finalSize) {
        for (qint32 i = 0; i < finalSize; ++i) {
            for (qint32 j = 0; j < regsA1[i].size(); ++j) {
                if (regsA1[i][j].getRegression() != 0)
                    regsA1[i][j].setRegression(finalSize - i);
            }
        }
        crA1.setRegressions(regsA1);

        // Replace crAvali if improved (or if crAvali is NaN)
        if (crA1.getFitness() <= crAvali.getFitness() ||
            crAvali.getFitness() != crAvali.getFitness()) {
            crAvali = crA1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// dualSort — quicksort keeping terms and labels in sync (like qSortDuplo)
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::dualSort(QVector<Domain::CompositeTerm>& terms,
                                            QVector<qint32>& labels)
{
    const qint32 n = terms.size();
    if (n < 2) return;

    QVector<qint32> indices(n);
    for (qint32 i = 0; i < n; ++i) indices[i] = i;

    // Sort indices descending by compact value (like CmpMaiorTerm)
    std::sort(indices.begin(), indices.end(), [&terms](qint32 a, qint32 b) {
        return terms[a].getCompact() > terms[b].getCompact();
    });

    QVector<Domain::CompositeTerm> sortedTerms(n);
    QVector<qint32> sortedLabels(n);
    for (qint32 i = 0; i < n; ++i) {
        sortedTerms[i] = terms[indices[i]];
        sortedLabels[i] = labels[indices[i]];
    }
    terms = sortedTerms;
    labels = sortedLabels;
}

////////////////////////////////////////////////////////////////////////////////
// sortElitism — sort indices by fitness (like qSortPop)
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::sortElitism(QVector<qint32>& indices,
                                               qint32 outputId) const
{
    const auto& pop = m_population[outputId];
    std::sort(indices.begin(), indices.end(), [&pop](qint32 a, qint32 b) {
        const auto& cr1 = pop[a];
        const auto& cr2 = pop[b];
        if (qAbs(cr1.getFitness() - cr2.getFitness()) < 1e-10)
            return cr1.getError() < cr2.getError();
        return cr1.getFitness() < cr2.getFitness();
    });
}

////////////////////////////////////////////////////////////////////////////////
// applyBasis — compute the basis function value for a data point
//   BasisPow:     pow(x, e)
//   BasisAbsPow:  |x|^e
//   BasisLogPow:  log(1 + |x|)^e
//   BasisExp:     exp(alpha * x)    (exponent stores alpha)
//   BasisTanhPow: tanh(x)^e
////////////////////////////////////////////////////////////////////////////////

static inline qreal applyBasis(Domain::BasisType bt, qreal x, qreal expo)
{
    switch (bt) {
    default:
    case Domain::BasisPow:
        return qPow(x, expo);
    case Domain::BasisAbsPow:
        return qPow(qAbs(x), expo);
    case Domain::BasisLogPow:
        return qPow(qLn(1.0 + qAbs(x)), expo);
    case Domain::BasisExp:
        return qExp(expo * x);            // alpha = expo
    case Domain::BasisTanhPow:
        return qPow(std::tanh(x), expo);
    }
}

////////////////////////////////////////////////////////////////////////////////
// montaRegressores — FAITHFUL to DES_MontaVlrs
// Key fixes: row 0 for vlrsMedido, 1e9 overflow threshold
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::montaRegressores(Domain::Chromosome& cr,
                                                    Utils::MathMatrix<qreal>& vlrsRegress,
                                                    Utils::MathVector<qreal>& vlrsMedido,
                                                    bool isValidacao,
                                                    bool isLinearCoef)
{
    auto regs = cr.getRegressions();
    const auto& dataMatrix = m_config.getAlgorithmData().variables.getValues();
    const qint32 numVars = dataMatrix.numRows();
    const qint32 totalCols = dataMatrix.numCols();
    const quint8 tipoExpo = m_config.getExponentType();

    // Decimation
    qint32 dec = 1;
    const auto& decVec = m_config.getDecimation();
    if (cr.getOutputId() < decVec.size() && decVec[cr.getOutputId()] > 0)
        dec = decVec[cr.getOutputId()];

    // Remove empty regressors and find maxDelay
    for (qint32 i = 0; i < regs.size(); ++i) {
        if (regs[i].isEmpty()) { regs.remove(i); --i; }
    }
    qint32 maxDelay = 0;
    for (qint32 i = 0; i < regs.size(); ++i)
        for (qint32 j = 0; j < regs[i].size(); ++j)
            if ((qint32)regs[i][j].getDelay() > maxDelay)
                maxDelay = regs[i][j].getDelay();
    cr.setMaxDelay(maxDelay);

    const qint32 qtdeAtrasos = totalCols / ((isValidacao ? 1 : 2) * dec);
    const qint32 posIniAtrasos = maxDelay * dec;
    const qint32 tam = qtdeAtrasos - maxDelay;

    if (tam <= 0 || regs.isEmpty()) {
        vlrsRegress.clear();
        vlrsMedido.clear();
        cr.setRegressions(regs);
        return;
    }

    // FIXED: vlrsMedido always uses row 0 (faithful to original DES_MontaVlrs)
    vlrsMedido.resize(tam);
    for (qint32 k = 0; k < tam; ++k) {
        qint32 col = posIniAtrasos + k * dec;
        vlrsMedido[k] = (col < totalCols && numVars > 0) ? dataMatrix.at(0, col) : 0.0;
    }

    // Build regressor columns
    QVector<Utils::MathVector<qreal>> columns;
    QVector<QVector<Domain::CompositeTerm>> validRegs;

    for (qint32 countRegress = 0; countRegress < regs.size() && countRegress < (tam - 2); ++countRegress) {
        Utils::MathVector<qreal> col(tam, 1.0);
        bool colValid = true;

        for (qint32 i = 0; i < regs[countRegress].size(); ++i) {
            const quint32 var = regs[countRegress][i].getVariable();
            const quint32 atraso = regs[countRegress][i].getDelay();
            const quint32 regIdx = regs[countRegress][i].getRegression();
            const bool isNum = regs[countRegress][i].isNumerator();
            qreal expo = regs[countRegress][i].getExponent();

            // Adjust exponent type (faithful to original)
            if (!tipoExpo) {
                // tipoExpo == 0: real exponent, use as-is
            } else {
                qreal ip = (qint32)expo;
                qreal fp = expo - ip;
                ip += (fp >= 0.5) ? 1 : ((fp <= -0.5) ? -1 : 0);
                if (tipoExpo == 2) ip = qAbs(ip); // Natural exponent
                expo = ip;
            }

            // Discard near-zero exponent (except constant: reg=0)
            if ((expo > -1e-5 && expo < 1e-5) && regIdx != 0) {
                regs[countRegress].remove(i); --i; continue;
            }

            // Compute term values
            Utils::MathVector<qreal> termVals(tam);
            bool termValid = true;
            const Domain::BasisType basisType = regs[countRegress][i].getBasisType();

            if (regIdx == 0) {
                termVals.fill(1.0);
            } else {
                const qint32 varIdx = (qint32)var - 1;
                for (qint32 k = 0; k < tam; ++k) {
                    qint32 sampleIdx = posIniAtrasos + k * dec - (qint32)atraso * dec;
                    if (varIdx >= 0 && varIdx < numVars && sampleIdx >= 0 && sampleIdx < totalCols) {
                        qreal dataVal = dataMatrix.at(varIdx, sampleIdx);
                        termVals[k] = applyBasis(basisType, dataVal, expo);
                        // FIXED: 1e9 threshold (faithful to original AoQuadTrans/AoQuad)
                        if (termVals[k] != termVals[k] || qAbs(termVals[k]) > 1e9)
                            { termValid = false; break; }
                    } else { termValid = false; break; }
                }
            }

            if (!termValid) {
                regs[countRegress].remove(i); --i; continue;
            }

            // Pseudo-linearization for denominator (1st term, isLinearCoef)
            if (i == 0 && !isNum && isLinearCoef) {
                for (qint32 k = 0; k < tam; ++k)
                    termVals[k] *= -1.0 * vlrsMedido[k];
            }

            // i==0 → assign, i>0 → multiply
            if (i == 0) col = termVals;
            else for (qint32 k = 0; k < tam; ++k) col[k] *= termVals[k];
        }

        if (regs[countRegress].isEmpty()) continue;

        // Validate column: FIXED 1e9 threshold
        for (qint32 k = 0; k < tam; ++k) {
            if (col[k] != col[k] || qAbs(col[k]) > 1e9) { colValid = false; break; }
        }
        if (!colValid) continue;

        columns.append(col);
        validRegs.append(regs[countRegress]);
    }

    // Build final matrix
    const qint32 nCols = columns.size();
    vlrsRegress.resize(tam, nCols);
    for (qint32 c = 0; c < nCols; ++c)
        for (qint32 r = 0; r < tam; ++r)
            vlrsRegress.at(r, c) = columns[c][r];

    cr.setRegressions(validRegs);
}

////////////////////////////////////////////////////////////////////////////////
// calcERR — FAITHFUL to DES_CalcERR
// Key fix: added second reordering pass after Householder
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::calcERR(Domain::Chromosome& cr, qreal serr)
{
    Utils::MathMatrix<qreal> vlrsRegress;
    Utils::MathVector<qreal> vlrsMedido;
    montaRegressores(cr, vlrsRegress, vlrsMedido, true, true);

    auto regs = cr.getRegressions();
    if (regs.isEmpty() || vlrsRegress.isEmpty()) return;

    const qint32 n = vlrsRegress.numCols();
    const qint32 m = vlrsRegress.numRows();

    // Build [A | y]
    Utils::MathMatrix<qreal> A(m, n + 1);
    for (qint32 r = 0; r < m; ++r) {
        for (qint32 c = 0; c < n; ++c)
            A.at(r, c) = vlrsRegress.at(r, c);
        A.at(r, n) = vlrsMedido[r];
    }

    // y'y
    qreal ySquared = 0.0;
    for (qint32 r = 0; r < m; ++r)
        ySquared += vlrsMedido[r] * vlrsMedido[r];
    if (ySquared < 1e-30) return;

    Utils::MathVector<qreal> errVec(n, 0.0);

    // First pass: greedy ERR with Householder orthogonalization
    for (qint32 j = 0; j < n; ++j) {
        if (m_stopRequested.load()) return;

        // Find column with maximum ERR among j..n-1
        qint32 bestCol = j;
        qreal bestERR = -1.0;

        for (qint32 k = j; k < n; ++k) {
            qreal dotKY = 0.0, dotKK = 0.0;
            for (qint32 r = j; r < m; ++r) {
                dotKY += A.at(r, k) * A.at(r, n);
                dotKK += A.at(r, k) * A.at(r, k);
            }
            qreal errK = (qAbs(ySquared * dotKK) > 1e-30)
                         ? (dotKY * dotKY) / (ySquared * dotKK) : 0.0;
            if (errK > bestERR) { bestERR = errK; bestCol = k; }
        }

        errVec[j] = bestERR;

        // Swap columns j and bestCol
        if (bestCol != j) {
            for (qint32 r = 0; r < m; ++r)
                qSwap(A.at(r, j), A.at(r, bestCol));
            for (qint32 r = 0; r < m; ++r)
                qSwap(vlrsRegress.at(r, j), vlrsRegress.at(r, bestCol));
            qSwap(regs[j], regs[bestCol]);
        }

        // Householder reflection
        qreal vNorm = 0.0;
        for (qint32 r = j; r < m; ++r)
            vNorm += A.at(r, j) * A.at(r, j);
        vNorm = qSqrt(vNorm);

        if (vNorm > 1e-10) {
            qreal v0 = A.at(j, j);
            qreal u = (qAbs(v0) > 1e-10 ? v0 : (v0 >= 0 ? 1e-10 : -1e-10))
                      + (v0 >= 0 ? 1.0 : -1.0) * vNorm;

            QVector<qreal> v(m - j);
            v[0] = 1.0;
            for (qint32 i = 1; i < v.size(); ++i)
                v[i] = A.at(j + i, j) / u;

            qreal vDotV = 0.0;
            for (qint32 i = 0; i < v.size(); ++i)
                vDotV += v[i] * v[i];
            qreal factor = -2.0 / vDotV;

            for (qint32 c = j; c <= n; ++c) {
                qreal vdotA = 0.0;
                for (qint32 i = 0; i < v.size(); ++i)
                    vdotA += v[i] * A.at(j + i, c);
                for (qint32 i = 0; i < v.size(); ++i)
                    A.at(j + i, c) += factor * v[i] * vdotA;
            }
        }
    }

    // FIXED: Second reordering pass after Householder (faithful to original)
    for (qint32 j = 0; j < n - 1; ++j) {
        qint32 bestIdx = j;
        qreal bestVal = errVec[j];
        for (qint32 k = j + 1; k < n; ++k) {
            if (errVec[k] > bestVal) {
                bestVal = errVec[k];
                bestIdx = k;
            }
        }
        if (bestIdx != j) {
            qSwap(errVec[j], errVec[bestIdx]);
            for (qint32 r = 0; r < m; ++r) {
                qSwap(A.at(r, j), A.at(r, bestIdx));
                qSwap(vlrsRegress.at(r, j), vlrsRegress.at(r, bestIdx));
            }
            qSwap(regs[j], regs[bestIdx]);
        }
    }

    // Select regressors by cumulative ERR vs threshold (faithful to original)
    qreal cumERR = 0.0;
    qint32 selectedCount = 0;
    for (qint32 j = 0; j < n; ++j) {
        if ((cumERR < serr || errVec[j] > 0.001) &&
            errVec[j] > 0.0009 && errVec[j] == errVec[j]) {
            cumERR += errVec[j];
            selectedCount++;
        } else break;
    }

    // Keep only selected
    while (regs.size() > selectedCount)
        regs.removeLast();

    // Update maxDelay
    qint32 maxDelay = 0;
    for (qint32 i = 0; i < regs.size(); ++i)
        for (qint32 j = 0; j < regs[i].size(); ++j)
            if ((qint32)regs[i][j].getDelay() > maxDelay)
                maxDelay = regs[i][j].getDelay();
    cr.setMaxDelay(maxDelay);

    Utils::MathVector<qreal> finalErr(selectedCount);
    for (qint32 i = 0; i < selectedCount; ++i)
        finalErr[i] = errVec[i];
    cr.setErrors(finalErr);
    cr.setRegressions(regs);
}

////////////////////////////////////////////////////////////////////////////////
// calcAptidao — FAITHFUL to DES_calAptidao (ELS Estendido)
// Resolve: [A'A - COV(e)*sigma1]*x = [A'b - COV(e)*sigma2]
// Modelo racional: y = (Rnum*Cnum + E*Ce) / (1 + Rden*Cden)
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::calcAptidao(Domain::Chromosome& cr, quint32 tamErro)
{
    cr.setError(9e99);
    cr.setFitness(9e99);

    // montaRegressores with isLinearCoef=false (like original)
    Utils::MathMatrix<qreal> vlrsRegress;
    Utils::MathVector<qreal> vlrsMedido;
    montaRegressores(cr, vlrsRegress, vlrsMedido, true, false);

    const qint32 qtdeAtrasos = vlrsMedido.size();
    auto regs = cr.getRegressions();

    if (qtdeAtrasos <= 0 || regs.isEmpty() || vlrsRegress.isEmpty()) return;

    // Remove excess regressors
    while (regs.size() > vlrsRegress.numCols()) {
        regs.removeLast();
    }

    // Count total terms
    qint32 size = 0;
    for (qint32 i = 0; i < regs.size(); ++i)
        size += regs[i].size();

    // Separate into numerator and denominator
    QVector<QVector<Domain::CompositeTerm>> regressNum, regressDen;
    QVector<Utils::MathVector<qreal>> numCols, denCols, denColsAux;
    Utils::MathVector<qreal> errNum, errDen;
    const auto& crErrors = cr.getErrors();

    qint32 tamNum = 0, tamDen = 0;
    for (qint32 i = 0; i < regs.size(); ++i) {
        Utils::MathVector<qreal> col(qtdeAtrasos);
        for (qint32 k = 0; k < qtdeAtrasos; ++k)
            col[k] = vlrsRegress.at(k, i);

        if (regs[i].at(0).isNumerator()) {
            regressNum.append(regs[i]);
            if (i < crErrors.size()) errNum.append(crErrors[i]);
            numCols.append(col);
            tamNum++;
        } else {
            regressDen.append(regs[i]);
            if (i < crErrors.size()) errDen.append(crErrors[i]);
            denCols.append(col);
            // auxDen = col * (-y) for pseudo-linearization
            Utils::MathVector<qreal> auxCol(qtdeAtrasos);
            for (qint32 k = 0; k < qtdeAtrasos; ++k)
                auxCol[k] = col[k] * (-1.0) * vlrsMedido[k];
            denColsAux.append(auxCol);
            tamDen++;
        }
    }

    // If no numerator, insert constant (like original)
    if (tamNum == 0) {
        tamNum = 1;
        Domain::CompositeTerm t;
        t.setVariable(1);
        t.setDelay(0);
        t.setRegression(0);
        t.setNumerator(true);
        t.setExponent(1.0);
        QVector<Domain::CompositeTerm> constReg;
        constReg.append(t);
        regressNum.prepend(constReg);
        errNum.append(-1.0);
        Utils::MathVector<qreal> onesCol(qtdeAtrasos, 1.0);
        numCols.prepend(onesCol);
    }

    // Reassemble: numerator first, then denominator
    regs.clear();
    regs += regressNum;
    regs += regressDen;
    cr.setRegressions(regs);

    // Build err vector
    Utils::MathVector<qreal> errAll(tamNum + tamDen);
    for (qint32 i = 0; i < errNum.size() && i < tamNum; ++i) errAll[i] = errNum[i];
    for (qint32 i = 0; i < errDen.size() && i < tamDen; ++i) errAll[tamNum + i] = errDen[i];
    cr.setErrors(errAll);

    // Initialize coefficients
    Utils::MathVector<qreal> crCoefic(tamNum + tamDen, 0.0);
    cr.setCoefficients(crCoefic);

    // Build A  = [Rnum | auxDen]  (for solving ELS — pseudo-linearized)
    // Build A1 = [Rnum | Rden]    (for estimating values — raw)
    qint32 totalRegressors = tamNum + tamDen;
    Utils::MathMatrix<qreal> A(qtdeAtrasos, totalRegressors);
    Utils::MathMatrix<qreal> A1(qtdeAtrasos, totalRegressors);
    for (qint32 k = 0; k < qtdeAtrasos; ++k) {
        for (qint32 c = 0; c < tamNum; ++c) {
            A.at(k, c)  = numCols[c][k];
            A1.at(k, c) = numCols[c][k];
        }
        for (qint32 c = 0; c < tamDen; ++c) {
            A.at(k, tamNum + c)  = denColsAux[c][k]; // pseudo-linearized
            A1.at(k, tamNum + c) = denCols[c][k];     // raw
        }
    }

    // sigma1 and sigma2 for ELS correction (rational model)
    Utils::MathMatrix<qreal> sigma1(totalRegressors, totalRegressors, 0.0);
    Utils::MathVector<qreal> sigma2(totalRegressors, 0.0);

    if (tamDen > 0) {
        // vlrsRegressDenAux = denCols * vlrsMedido (without -1)
        for (qint32 i = 0; i < tamDen; ++i) {
            for (qint32 j = 0; j < tamDen; ++j) {
                qreal dot = 0.0;
                for (qint32 k = 0; k < qtdeAtrasos; ++k) {
                    qreal ri = denCols[i][k] * vlrsMedido[k];
                    qreal rj = denCols[j][k] * vlrsMedido[k];
                    dot += ri * rj;
                }
                sigma1.at(tamNum + i, tamNum + j) = dot;
            }
            qreal colSum = 0.0;
            for (qint32 k = 0; k < qtdeAtrasos; ++k)
                colSum += denCols[i][k] * vlrsMedido[k];
            sigma2[tamNum + i] = -colSum;
        }
    }

    // Iterative ELS Estendido loop
    qreal var = 0.0, var1 = 0.0, erroDepois = 9e99;
    bool isOk = false, isOk1 = false, isOk2 = false;
    Utils::MathVector<qreal> vlrsCoefic, vlrsCoefic1;
    Utils::MathVector<qreal> vlrsResiduo, vlrsEstimado;

    const qint32 totalDataCols = m_config.getAlgorithmData().variables.getValues().numCols();

    qint32 count1 = 0;
    do {
        if (m_stopRequested.load()) return;

        // Update best result (from 2nd iteration)
        if (count1 > 0 && isOk && erroDepois < cr.getError()) {
            cr.setCoefficients(vlrsCoefic);
            cr.setError(erroDepois);
            cr.setFitness(totalDataCols * qLn(cr.getError()) +
                          (2 * size + regs.size()) * qLn((qreal)totalDataCols));
        }

        // Insert residual terms into the system
        for (qint32 i = 0; i < count1 && vlrsResiduo.size() > 1; ++i) {
            // Shift residual: prepend 0, remove last
            Utils::MathVector<qreal> shifted(vlrsResiduo.size());
            shifted[0] = 0.0;
            for (qint32 k = 1; k < vlrsResiduo.size(); ++k)
                shifted[k] = vlrsResiduo[k - 1];
            vlrsResiduo = shifted;

            qint32 errColIdx = tamNum + tamDen + i;
            // Expand matrices if needed
            if (A.numCols() <= errColIdx) {
                Utils::MathMatrix<qreal> newA(qtdeAtrasos, errColIdx + 1, 0.0);
                Utils::MathMatrix<qreal> newA1(qtdeAtrasos, errColIdx + 1, 0.0);
                for (qint32 r = 0; r < qtdeAtrasos; ++r) {
                    for (qint32 c = 0; c < A.numCols(); ++c) {
                        newA.at(r, c) = A.at(r, c);
                        newA1.at(r, c) = A1.at(r, c);
                    }
                }
                A = newA; A1 = newA1;

                Utils::MathMatrix<qreal> newS1(errColIdx + 1, errColIdx + 1, 0.0);
                Utils::MathVector<qreal> newS2(errColIdx + 1, 0.0);
                for (qint32 r = 0; r < sigma1.numRows(); ++r) {
                    for (qint32 c = 0; c < sigma1.numCols(); ++c)
                        newS1.at(r, c) = sigma1.at(r, c);
                    newS2[r] = sigma2[r];
                }
                sigma1 = newS1; sigma2 = newS2;
            }
            for (qint32 k = 0; k < qtdeAtrasos; ++k) {
                A.at(k, errColIdx) = vlrsResiduo[k];
                A1.at(k, errColIdx) = vlrsResiduo[k];
            }
        }

        // Inner ELS convergence loop
        qint32 count2 = 0;
        qint32 currentCols = A.numCols();
        do {
            if (m_stopRequested.load()) return;

            if (count2 > 0) {
                var = var1;
                vlrsCoefic = vlrsCoefic1;
            }

            // Solve: [A'A - var*sigma1]*x = [A'b - var*sigma2]
            Utils::MathMatrix<qreal> V(currentCols, currentCols, 0.0);
            Utils::MathVector<qreal> X(currentCols, 0.0);

            // V = A'A
            for (qint32 i = 0; i < currentCols; ++i) {
                for (qint32 j = i; j < currentCols; ++j) {
                    qreal dot = 0.0;
                    for (qint32 k = 0; k < qtdeAtrasos; ++k)
                        dot += A.at(k, i) * A.at(k, j);
                    V.at(i, j) = dot;
                    V.at(j, i) = dot;
                }
            }
            // V -= var * sigma1
            for (qint32 i = 0; i < currentCols && i < sigma1.numRows(); ++i)
                for (qint32 j = 0; j < currentCols && j < sigma1.numCols(); ++j)
                    V.at(i, j) -= var * sigma1.at(i, j);

            // X = A'y
            for (qint32 i = 0; i < currentCols; ++i) {
                qreal dot = 0.0;
                for (qint32 k = 0; k < qtdeAtrasos; ++k)
                    dot += A.at(k, i) * vlrsMedido[k];
                X[i] = dot;
            }
            // X -= var * sigma2
            for (qint32 i = 0; i < currentCols && i < sigma2.size(); ++i)
                X[i] -= var * sigma2[i];

            // Solve V * coefic = X
            isOk = m_mathService->solveLinearSystem(V, X, vlrsCoefic1);

            if (isOk) {
                // Compute estimated/residual (rational model)
                calcVlrsEstRes(cr, A1, vlrsCoefic1, vlrsMedido, vlrsResiduo, vlrsEstimado);

                // Variance of residuals
                var1 = computeVariance(vlrsResiduo);

                // Check convergence
                isOk1 = compareCoefficients(vlrsCoefic, vlrsCoefic1, 1e-3);
                isOk2 = (var - var1) == 0.0 ? true :
                         ((var - var1) > 0 ? (var - var1) < 1e-3 : (var - var1) > -1e-3);
                count2++;
            }
        } while (isOk && !(isOk1 && isOk2) && count2 <= 20);

        // Mean squared error: r'r / n
        if (!vlrsResiduo.isEmpty()) {
            qreal rDotR = vlrsResiduo.dotProduct(vlrsResiduo);
            erroDepois = rDotR / qtdeAtrasos;
        }

        count1++;
    } while ((quint32)count1 <= tamErro);

    // Eliminate spurious coefficients (|c| < 1e-3 or |c| > 1e+3, like original)
    auto finalRegs = cr.getRegressions();
    auto finalCoefic = cr.getCoefficients();
    auto finalErrs = cr.getErrors();
    bool needRecalc = false;
    for (qint32 i = 0; i < finalRegs.size(); ++i) {
        if (i < finalCoefic.size() && finalCoefic[i] != 0.0) {
            if ((finalCoefic[i] <= 1e-3 && finalCoefic[i] >= -1e-3) ||
                finalCoefic[i] >= 1e+3 || finalCoefic[i] <= -1e+3) {
                finalRegs.remove(i);
                if (i < finalErrs.size()) finalErrs.remove(i);
                needRecalc = true;
            }
        }
    }
    if (needRecalc) {
        cr.setRegressions(finalRegs);
        cr.setErrors(finalErrs);
        calcAptidao(cr);
    }
}

////////////////////////////////////////////////////////////////////////////////
// calcVlrsEstRes — FAITHFUL to DES_CalcVlrsEstRes
// Model: y = (Rnum*Cnum + E*Ce) / (1 + Rden*Cden)
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::calcVlrsEstRes(
    const Domain::Chromosome& cr,
    const Utils::MathMatrix<qreal>& vlrsRegress,
    const Utils::MathVector<qreal>& vlrsCoefic,
    const Utils::MathVector<qreal>& vlrsMedido,
    Utils::MathVector<qreal>& vlrsResiduo,
    Utils::MathVector<qreal>& vlrsEstimado) const
{
    const qint32 n = vlrsMedido.size();
    const auto& regs = cr.getRegressions();

    // Separate into numerator and denominator (like original)
    qint32 tamNum = 0, tamDen = 0;
    for (qint32 i = 0; i < regs.size(); ++i) {
        if (regs[i].at(0).isNumerator()) tamNum++;
        else tamDen++;
    }

    const qint32 totalErro = vlrsCoefic.size() - (tamNum + tamDen);

    // Build numerator indices and denominator indices
    QVector<qint32> numIdx, denIdx;
    {
        qint32 numI = 0, denI = 0;
        for (qint32 i = 0; i < regs.size(); ++i) {
            if (regs[i].at(0).isNumerator()) { numIdx.append(i); numI++; }
            else { denIdx.append(i); denI++; }
        }
    }

    // Numerator: Rnum * Cnum
    vlrsEstimado.resize(n);
    for (qint32 k = 0; k < n; ++k) {
        qreal numVal = 0.0;
        for (qint32 j = 0; j < numIdx.size(); ++j) {
            qint32 col = numIdx[j];
            if (col < vlrsRegress.numCols() && col < vlrsCoefic.size())
                numVal += vlrsRegress.at(k, col) * vlrsCoefic[col];
        }
        vlrsEstimado[k] = numVal;
    }

    // Denominator: 1 + Rden * Cden
    Utils::MathVector<qreal> denominator(n, 1.0);
    for (qint32 k = 0; k < n; ++k) {
        for (qint32 j = 0; j < denIdx.size(); ++j) {
            qint32 col = denIdx[j];
            if (col < vlrsRegress.numCols() && col < vlrsCoefic.size())
                denominator[k] += vlrsRegress.at(k, col) * vlrsCoefic[col];
        }
    }

    if (totalErro > 0) {
        // With error terms — recursive calculation
        vlrsResiduo.resize(n);
        vlrsResiduo.fill(0.0);
        const qint32 tamvlrsRegress = tamNum + tamDen;

        for (qint32 k = 0; k < n; ++k) {
            for (qint32 i = 0; i < totalErro; ++i) {
                qreal errVal = (k - i >= 0) ? vlrsResiduo[k - i] : 0.0;
                vlrsEstimado[k] += vlrsCoefic[tamvlrsRegress + i] * errVal;
            }
            if (qAbs(denominator[k]) > 1e-30)
                vlrsEstimado[k] /= denominator[k];
            vlrsResiduo[k] = vlrsMedido[k] - vlrsEstimado[k];
        }
    } else {
        // No error terms
        vlrsResiduo.resize(n);
        for (qint32 k = 0; k < n; ++k) {
            if (qAbs(denominator[k]) > 1e-30)
                vlrsEstimado[k] /= denominator[k];
            vlrsResiduo[k] = vlrsMedido[k] - vlrsEstimado[k];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// montaSaida — FAITHFUL to DES_MontaSaida
////////////////////////////////////////////////////////////////////////////////

void DifferentialEvolutionService::montaSaida(Domain::Chromosome& cr,
                                              QVector<qreal>& vplotar,
                                              QVector<qreal>& resid)
{
    Utils::MathMatrix<qreal> vlrsRegress;
    Utils::MathVector<qreal> vlrsMedido;
    Utils::MathVector<qreal> vlrsEstimado, vlrsResiduo;

    montaRegressores(cr, vlrsRegress, vlrsMedido, true, false);
    calcVlrsEstRes(cr, vlrsRegress, cr.getCoefficients(), vlrsMedido, vlrsResiduo, vlrsEstimado);

    vplotar.clear();
    for (qint32 i = 0; i < vlrsEstimado.size(); ++i)
        vplotar.append(vlrsEstimado[i]);

    resid.clear();
    for (qint32 i = 0; i < vlrsResiduo.size(); ++i)
        resid.append(vlrsResiduo[i]);
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

bool DifferentialEvolutionService::compareCoefficients(
    const Utils::MathVector<qreal>& a,
    const Utils::MathVector<qreal>& b,
    qreal tolerance)
{
    if (a.size() != b.size() || a.isEmpty()) return false;
    for (qint32 i = 0; i < a.size(); ++i)
        if (qAbs(a[i] - b[i]) > tolerance) return false;
    return true;
}

qreal DifferentialEvolutionService::computeVariance(const Utils::MathVector<qreal>& vec)
{
    if (vec.size() <= 1) return 0.0;
    qreal mean = vec.mean();
    qreal sum = 0.0;
    for (qint32 i = 0; i < vec.size(); ++i) {
        qreal diff = vec[i] - mean;
        sum += diff * diff;
    }
    return sum / (vec.size() - 1);
}

void DifferentialEvolutionService::emitProgress()
{
    QVector<qreal> errors;
    QVector<Domain::Chromosome> best;
    {
        QMutexLocker locker(&m_resultMutex);
        for (qint32 i = 0; i < m_bestChromosomes.size(); ++i) {
            errors.append(m_bestErrors[i]);
            best.append(m_bestChromosomes[i]);
        }
    }
    emit statusUpdated(m_currentIteration.load(), errors, best);
}

bool DifferentialEvolutionService::shouldStop() const
{
    // Faithful to original: algorithm runs forever until user stops
    // or JNRR convergence triggers 'break' in evolutionLoop().
    // numeroCiclos (cycleCount) is NOT a total iteration limit.
    return m_stopRequested.load();
}

} // namespace Services
