#include "evolution_engine.h"
#include "chromosome_service.h"
#include "model_pruning_config.h"
#include "adaptive_tuning_engine.h"
#include "../threading/shared_state.h"
#include "../threading/thread_worker.h"
#include <QTime>
#include <QDebug>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
EvolutionEngine::EvolutionEngine(SharedState *state, ChromosomeService *chromoSvc, ThreadWorker *worker)
    : m_state(state)
    , m_chromoSvc(chromoSvc)
    , m_worker(worker)
{
    m_rng.seed(QTime::currentTime().msec());
    for (int i = 0; i < TAMPIPELINE; ++i)
        m_idParada_Th[i] = false;
}

///////////////////////////////////////////////////////////////////////////////
/// run — DES_AlgDiffEvol (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void EvolutionEngine::run()
{
    ////////////////////////////////////////////////////////////////////////////
    const qint32 qtSaidas = m_state->Adj.Dados.variaveis.qtSaidas;
    const qint32 tamPop   = m_state->Adj.Dados.tamPop;
    qint32 tokenPop;
    JMathVar<qreal> m1(10, 10, 5.0), m2(10, 10, 5.0);
    QVector<Cromossomo> crBest(qtSaidas);
    ////////////////////////////////////////////////////////////////////////////
    bool isOk = false, isPrint = true;
    qint32 count0 = 0, count2 = 0, idSaida = 0, idPipeLine = 0;
    qint32 pbestPoint, r1Point, r2Point, cr2Point;
    ////////////////////////////////////////////////////////////////////////////
    qDebug() << "[DE] EvolutionEngine::run START  tamPop=" << tamPop
             << " qtSaidas=" << qtSaidas
             << " F=" << m_state->Adj.deParams.F
             << " CR=" << m_state->Adj.deParams.CR
             << " pbest_rate=" << m_state->Adj.deParams.pbest_rate;
    ////////////////////////////////////////////////////////////////////////////
    for (idPipeLine = 0; idPipeLine < TAMPIPELINE; idPipeLine++)
        m_idParada_Th[idPipeLine] = !m_state->idParadaJust[count0];
    ////////////////////////////////////////////////////////////////////////////
    // Apenas um thread qualquer inicializa as variaveis e os ponteiros deste metodo.
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        for (count2 = 0; count2 < m1.size(); count2++) { m1[count2] = count2; m2[count2] = count2; }
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->Adj.tp = QTime::currentTime();
        for (idPipeLine = 0; idPipeLine < TAMPIPELINE; idPipeLine++) {
            if (idPipeLine) m_state->Adj.vetPop[idPipeLine].clear();
            m_state->Adj.melhorAptidaoAnt.clear();
            for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                m_state->idChange[idPipeLine][idSaida] = tamPop + 1;
                m_state->Adj.vetElitismo[idPipeLine][idSaida].append(1);
                m_state->Adj.vetElitismo[idPipeLine][idSaida].append(0);
                for (count0 = 2; count0 < tamPop; count0++)
                    m_state->Adj.vetElitismo[idPipeLine][idSaida].append(count0);
                m_state->crMut[idPipeLine][idSaida] = m_chromoSvc->createRandom(idSaida);
                m_state->Adj.melhorAptidaoAnt.append(m_state->crMut.at(idPipeLine).at(idSaida).aptidao);
            }
        }
        for (count0 = 0; count0 < tamPop; count0++) {
            if (m_state->Adj.Dados.isElitismo == 1 ? count0 >= m_state->Adj.Dados.iElitismo : true)
                m_state->Adj.vetPop[0].append(count0);
            else
                m_state->Adj.vetPop[TAMPIPELINE - 1].append(count0);
        }
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    // Calcula a aptidao ou cria os cromossomos iniciais
    ////////////////////////////////////////////////////////////////////////////
    idPipeLine = 0;
    isOk = false;
    m_state->lock_index[0].lockForWrite();
    if (m_state->Adj.vetPop.at(0).size() && !m_state->index[0]) {
        isOk = true;
        tokenPop = m_state->Adj.vetPop[0].takeFirst();
    }
    m_state->lock_index[0].unlock();
    ////////////////////////////////////////////////////////////////////////////
    while (isOk) {
        for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
            if (!m_state->Adj.isCriado)
                m_state->Adj.Pop[idSaida][tokenPop] = m_chromoSvc->createRandom(idSaida);
            else
                m_chromoSvc->evaluate(m_state->Adj.Pop[idSaida][tokenPop]);
        }
        ////////////////////////////////////////////////////////////////////////////
        m_state->lock_index[TAMPIPELINE - 1].lockForWrite();
        m_state->Adj.vetPop[TAMPIPELINE - 1].append(tokenPop);
        m_state->lock_index[TAMPIPELINE - 1].unlock();
        ////////////////////////////////////////////////////////////////////////////
        isOk = false;
        m_state->lock_index[0].lockForWrite();
        if (m_state->Adj.vetPop.at(0).size() && !m_state->index[0]) {
            isOk = true;
            tokenPop = m_state->Adj.vetPop[0].takeFirst();
        }
        m_state->lock_index[0].unlock();
        ////////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////
    idPipeLine = 2;
    m_state->mutex.lock();
    if (m_state->justThread[0].tryAcquire()) m_state->justSync.wait(&m_state->mutex);
    else {
        m_state->Adj.isCriado = true;
        m_state->justThread[0].release(m_state->TH_size - 1);
        m_state->justSync.wakeAll();
    }
    m_state->mutex.unlock();
    ////////////////////////////////////////////////////////////////////////////
    // Inicia o processo do DE
    ////////////////////////////////////////////////////////////////////////////
    forever {
        isOk = false;
        m_state->lock_index[idPipeLine].lockForWrite();
        if (m_state->Adj.vetPop.at(idPipeLine).size() && !m_state->index[idPipeLine]) {
            isOk = true;
            tokenPop = m_state->Adj.vetPop[idPipeLine].takeFirst();
        }
        m_state->lock_index[idPipeLine].unlock();
        while (isOk) {
            for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                ////////////////////////////////////////////////////////////////////////////
                // DE Canonico: Selecao de doadores (pbest, r1, r2)
                ////////////////////////////////////////////////////////////////////////////
                const qreal effectivePbestRate =
                    (m_state->Adj.iteracoes < DEDiversityConfig::kWarmupCycles)
                    ? qMax(m_state->Adj.deParams.pbest_rate, (double)DEDiversityConfig::kWarmupPbestRate)
                    : m_state->Adj.deParams.pbest_rate;
                const qint32 topP = qMax((qint32)1, (qint32)(effectivePbestRate * tamPop));
                qint32 pbest_rank, r1_rank, r2_rank;
                qint32 safetyCounter;
                const qint32 MAX_RETRIES = tamPop * 3 + 100;

                m_state->lock_Elitismo[idPipeLine].lockForRead();
                // Seleciona pbest do top-p%
                safetyCounter = 0;
                do {
                    pbest_rank = m_rng.randInt(0, topP - 1);
                    pbestPoint = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(pbest_rank);
                    if (++safetyCounter > MAX_RETRIES) break;
                } while (pbestPoint == tokenPop && topP > 1);
                // Seleciona r1 distinto de target e pbest
                safetyCounter = 0;
                do {
                    r1_rank = m_rng.randInt(0, tamPop - 1);
                    r1Point = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(r1_rank);
                    if (++safetyCounter > MAX_RETRIES) break;
                } while (r1Point == tokenPop || r1Point == pbestPoint);
                // Seleciona r2 distinto de target, pbest e r1
                safetyCounter = 0;
                do {
                    r2_rank = m_rng.randInt(0, tamPop - 1);
                    r2Point = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(r2_rank);
                    if (++safetyCounter > MAX_RETRIES) break;
                } while (r2Point == tokenPop || r2Point == pbestPoint || r2Point == r1Point);
                m_state->lock_Elitismo[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
                // Leitura thread-safe dos cromossomos doadores e do target
                ////////////////////////////////////////////////////////////////////////////
                Cromossomo target_cr, pbest_cr, r1_cr, r2_cr;
                m_state->lock_BufferSR.lockForRead();
                target_cr = m_state->Adj.Pop.at(idSaida).at(tokenPop);
                pbest_cr  = m_state->Adj.Pop.at(idSaida).at(pbestPoint);
                r1_cr     = m_state->Adj.Pop.at(idSaida).at(r1Point);
                r2_cr     = m_state->Adj.Pop.at(idSaida).at(r2Point);
                m_state->lock_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////
                // Gera trial via mutacao current-to-pbest/1 + crossover binomial
                ////////////////////////////////////////////////////////////////////////////
                Cromossomo trial;
                bool trialOk = false;
                if (target_cr.regress.size() > 0) {
                    qreal trialF = m_state->Adj.deParams.F;
                    qreal trialCR = m_state->Adj.deParams.CR;
                    if (DEDiversityConfig::kUseAdaptiveTrialFCR) {
                        trialF = DEDiversityConfig::kFMin
                               + (DEDiversityConfig::kFMax - DEDiversityConfig::kFMin) * m_rng.rand();
                        trialCR = DEDiversityConfig::kCRMin
                                + (DEDiversityConfig::kCRMax - DEDiversityConfig::kCRMin) * m_rng.rand();
                    }
                    trial = m_chromoSvc->generateTrial(target_cr, pbest_cr, r1_cr, r2_cr,
                                                       trialF,
                                                       trialCR);
                    trialOk = true;
                }
                ////////////////////////////////////////////////////////////////////////////
                // Selecao 1-a-1: trial vs target
                ////////////////////////////////////////////////////////////////////////////
                if (trialOk && trial.regress.size() > 0 &&
                    (trial.aptidao <= target_cr.aptidao || target_cr.aptidao != target_cr.aptidao)) {
                    m_state->lock_BufferSR.lockForWrite();
                    m_state->Adj.Pop[idSaida][tokenPop] = trial;
                    m_state->lock_BufferSR.unlock();
                }
                ////////////////////////////////////////////////////////////////////////////
                // Calcula o SSE medio.
                m_state->lock_index[idPipeLine].lockForWrite();
                m_state->somaSSE[idPipeLine][idSaida] += m_state->Adj.Pop.at(idSaida).at(tokenPop).erro;
                m_state->lock_index[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
            }
            ////////////////////////////////////////////////////////////////////////////
            m_state->lock_index[idPipeLine == 0 ? TAMPIPELINE - 1 : idPipeLine - 1].lockForWrite();
            m_state->Adj.vetPop[idPipeLine == 0 ? TAMPIPELINE - 1 : idPipeLine - 1].append(tokenPop);
            m_state->lock_index[idPipeLine == 0 ? TAMPIPELINE - 1 : idPipeLine - 1].unlock();
            ////////////////////////////////////////////////////////////////////////////
            isOk = false;
            m_state->lock_index[idPipeLine].lockForWrite();
            if (m_state->Adj.vetPop.at(idPipeLine).size() && !m_state->index[idPipeLine]) {
                isOk = true;
                tokenPop = m_state->Adj.vetPop[idPipeLine].takeFirst();
            }
            m_state->lock_index[idPipeLine].unlock();
            ////////////////////////////////////////////////////////////////////////////
        }
        m_state->lock_index[idPipeLine].lockForWrite();
        m_state->index[idPipeLine]++;
        isOk = (m_state->idParadaJust[idPipeLine] != m_idParada_Th[idPipeLine]);
        m_state->lock_index[idPipeLine].unlock();
        if (isOk) {
            m_idParada_Th[idPipeLine] = m_idParada_Th[idPipeLine] ? false : true;
            ////////////////////////////////////////////////////////////////////////////
            // Uma thread fica para fazer algumas ponderacoes
            if (!m_state->justThread[idPipeLine].tryAcquire()) {
                m_state->justThread[idPipeLine].release(m_state->TH_size - 1);
                ////////////////////////////////////////////////////////////////////////////
                // Ranking / elitismo
                isOk = true;
                for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                    cr2Point = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);
                    m_chromoSvc->qSortPop(m_state->Adj.vetElitismo[idPipeLine][idSaida].begin(),
                                          m_state->Adj.vetElitismo[idPipeLine][idSaida].end(), idSaida);
                    qint32 bestPoint = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(0);

                    m_state->lock_BufferSR.lockForRead();
                    crBest[idSaida] = m_state->Adj.Pop.at(idSaida).at(bestPoint);
                    {
                        const qreal delta = m_state->Adj.melhorAptidaoAnt.at(idSaida) - crBest.at(idSaida).aptidao;
                        const qreal base_val = qMax(qAbs(m_state->Adj.melhorAptidaoAnt.at(idSaida)), 1e-12);
                        const bool improved = (delta / base_val >= m_state->Adj.deParams.tol_rel)
                                           || (delta >= m_state->Adj.jnrr);
                        if (improved) m_state->Adj.melhorAptidaoAnt[idSaida] = crBest.at(idSaida).aptidao;
                        else isOk = false;
                    }
                    {
                        Cromossomo crOldBest = m_state->Adj.Pop.at(idSaida).at(cr2Point);
                        isPrint = isPrint || (crBest.at(idSaida).aptidao < crOldBest.aptidao);
                    }
                    m_state->lock_BufferSR.unlock();

                    m_state->crMut[idPipeLine][idSaida] = crBest.at(idSaida);
                }
                ////////////////////////////////////////////////////////////////////////////
                m_state->lock_BufferSR.lockForWrite();
                m_state->Adj.iteracoes++;
                if (isOk) m_state->Adj.iteracoesAnt = m_state->Adj.iteracoes;
                else {
                    const qint64 janela = qMin((qint64)m_state->Adj.numeroCiclos,
                                               (qint64)m_state->Adj.deParams.stagnation_window);
                    if (m_state->Adj.iteracoes >= m_state->Adj.iteracoesAnt + janela)
                        m_state->setModoOperTH(2);
                }

                // ===== AUTO-TUNING ADAPTATIVO (a cada 5 gerações) =====
                if (m_state->Adj.adaptiveState.shouldUpdate(m_state->Adj.iteracoes)) {
                    // Coletar fitness global de todas as saídas
                    QVector<qreal> all_fitness;
                    for (qint32 i = 0; i < qtSaidas; ++i) {
                        for (qint32 j = 0; j < tamPop; ++j) {
                            all_fitness.append(m_state->Adj.Pop[i][j].aptidao);
                        }
                    }

                    // Atualizar parâmetros adaptativos
                    IndicatorSnapshot snapshot;
                    AdaptiveParameters new_params = AdaptiveTuningEngine::updateAndGetParameters(
                        all_fitness,
                        qtSaidas,
                        m_state->Adj.iteracoes,
                        m_state->Adj.adaptiveState.previous_best_fitness,
                        snapshot
                    );

                    // Armazenar em estado compartilhado
                    m_state->Adj.adaptiveState.update(new_params, snapshot, m_state->Adj.iteracoes);
                    m_state->Adj.adaptiveState.previous_best_fitness = crBest.at(0).aptidao;

                    // Log breve
                    qDebug() << "[AUTO-TUNE] Gen" << m_state->Adj.iteracoes
                             << "| Div:" << snapshot.diversity
                             << "| DeltaJn:" << snapshot.delta_jn
                             << "| Stag:" << snapshot.is_stagnant
                             << "| ConvPrem:" << snapshot.is_premature_convergence;
                }

                // Injeção periódica de imigrantes aleatórios (anti-convergência prematura)
                if (DEDiversityConfig::kUseRandomImmigrants
                    && m_state->Adj.iteracoes >= DEDiversityConfig::kImmigrantStartCycle
                    && (m_state->Adj.iteracoes % DEDiversityConfig::kImmigrantPeriodCycles) == 0) {
                    const qint32 qtdImm = qMax((qint32)1, (qint32)(tamPop * DEDiversityConfig::kImmigrantFraction));
                    for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                        m_state->lock_Elitismo[idPipeLine].lockForRead();
                        for (qint32 k = 0; k < qtdImm; ++k) {
                            const qint32 worstRank = tamPop - 1 - k;
                            if (worstRank < 0) break;
                            const qint32 worstIdx = m_state->Adj.vetElitismo.at(idPipeLine).at(idSaida).at(worstRank);
                            m_state->Adj.Pop[idSaida][worstIdx] = m_chromoSvc->createRandom(idSaida);
                        }
                        m_state->lock_Elitismo[idPipeLine].unlock();
                    }
                }
                isOk = (((m_state->Adj.tp.secsTo(QTime::currentTime()) >= 6) && isPrint)
                     || (m_state->Adj.tp.secsTo(QTime::currentTime()) >= 60));
                if (isOk) m_state->Adj.tp = QTime::currentTime();
                m_state->lock_BufferSR.unlock();
                ////////////////////////////////////////////////////////////////////////////
                if (isOk) {
                    for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                        m_chromoSvc->evaluate(crBest[idSaida], 15);
                        m_chromoSvc->buildOutput(crBest[idSaida],
                                                 m_state->vcalc[idPipeLine][idSaida],
                                                 m_state->residuos[idPipeLine][idSaida]);
                    }
                    emit m_worker->signal_SetStatus(m_state->Adj.iteracoes,
                                                    &m_state->somaSSE.at(idPipeLine),
                                                    &m_state->vcalc.at(idPipeLine),
                                                    &m_state->residuos.at(idPipeLine),
                                                    &crBest);
                    // Nota: isEquacaoEscrita e isStatusSetado são controlados pelo ThreadWorker
                    emit m_worker->signal_EscreveEquacao();
                    emit m_worker->signal_Desenha();
                    isPrint = false;
                }
                ////////////////////////////////////////////////////////////////////////////
                m_state->lock_index[idPipeLine].lockForWrite();
                m_state->idParadaJust[idPipeLine] = m_state->idParadaJust[idPipeLine] ? false : true;
                for (idSaida = 0; idSaida < qtSaidas; idSaida++)
                    m_state->somaSSE[idPipeLine][idSaida] = 0.0f;
                m_state->index[idPipeLine] = 0;
                m_state->lock_index[idPipeLine].unlock();
                ////////////////////////////////////////////////////////////////////////////
            }
        }
        idPipeLine = idPipeLine == 0 ? TAMPIPELINE - 1 : idPipeLine - 1;
        ////////////////////////////////////////////////////////////////////////////
        // Checa se tem que parar a thread
        m_state->lock_modeOper_TH.lockForRead();
        isOk = m_state->Adj.modeOper_TH == 2;
        m_state->lock_modeOper_TH.unlock();
        if (isOk) {
            m_state->mutex.lock();
            if (m_state->waitThread.tryAcquire()) m_state->waitSync.wait(&m_state->mutex);
            else {
                m_state->waitThread.release(m_state->TH_size - 1);
                for (idSaida = 0; idSaida < qtSaidas; idSaida++) {
                    m_chromoSvc->evaluate(crBest[idSaida], 15);
                    m_chromoSvc->buildOutput(crBest[idSaida],
                                             m_state->vcalc[idPipeLine][idSaida],
                                             m_state->residuos[idPipeLine][idSaida]);
                }
                emit m_worker->signal_SetStatus(m_state->Adj.iteracoes,
                                                &m_state->somaSSE.at(idPipeLine),
                                                &m_state->vcalc.at(idPipeLine),
                                                &m_state->residuos.at(idPipeLine),
                                                &crBest);
                emit m_worker->signal_Finalizar();
                emit m_worker->signal_Parado();
                m_state->waitSync.wait(&m_state->mutex);
            }
            m_state->mutex.unlock();
        }
        ////////////////////////////////////////////////////////////////////////////
        // Checa se tem que finalizar a tarefa.
        m_state->lock_modeOper_TH.lockForRead();
        isOk = m_state->Adj.modeOper_TH <= 1;
        m_state->lock_modeOper_TH.unlock();
        if (isOk)
            return;
        ////////////////////////////////////////////////////////////////////////////
    }
    ////////////////////////////////////////////////////////////////////////////
}
