///////////////////////////////////////////////////////////////////////////////
/// EXEMPLO DE INTEGRAÇÃO — Term Importance Pruning
/// Arquivo: integration_example.cpp
///
/// Este arquivo mostra como integrar as novas funções de análise de 
/// importância e poda probabilística no loop de evolução.
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// OPÇÃO 1: Poda após generateTrial (mais agressiva, a cada trial)
// ============================================================================
// Localização: EvolutionEngine::run(), logo após generateTrial()
// Desvantagem: Muito lento (calcula importância em cada trial)

void integrateAfterGenerateTrial_AGGRESSIVE()
{
    // ... código existing ...
    
    // Gera trial via mutacao current-to-pbest/1 + crossover binomial
    Cromossomo trial;
    bool trialOk = false;
    if (target_cr.regress.size() > 0) {
        trial = m_chromoSvc->generateTrial(target_cr, pbest_cr, r1_cr, r2_cr,
                                          m_state->Adj.deParams.F,
                                          m_state->Adj.deParams.CR);
        trialOk = true;
    }

    // ─────────────────────────────────────────────────────────────────────
    // NOVO: Poda probabilística após geração
    // ─────────────────────────────────────────────────────────────────────
    if (trialOk && trial.regress.size() > 0) {
        qint32 removed = m_chromoSvc->probabilisticTermPruning(
            trial,
            0.01,   // threshold: 1%
            0.25    // removalRate: até 25%
        );
        
        if (removed > 0) {
            // Avaliar após remoções (o probabilisticTermPruning já re-avalia, mas double-check)
            m_chromoSvc->evaluate(trial, 1);
        }
    }

    // Selecao 1-a-1: trial vs target
    if (trialOk && trial.regress.size() > 0 &&
        (trial.aptidao <= target_cr.aptidao || target_cr.aptidao != target_cr.aptidao)) {
        // ... código existing de atualização ...
    }
}

// ============================================================================
// OPÇÃO 2: Poda em cadência baixa (recomendado)
// ============================================================================
// Localização: EvolutionEngine::run(), em ciclos (a cada N gerações)
// Desvantagem: Periódico, pode perder oportunidades
// Vantagem: Eficiente computacionalmente

void integrateWithCadence_BALANCED()
{
    const qint32 PRUNE_CADENCE = 10;  // A cada 10 gerações
    
    // ... loop main ...
    for (numeroCiclos = 0; numeroCiclos < maxGeneracoes; ++numeroCiclos) {
        
        // ... evolução normal ...
        
        // A cada PRUNE_CADENCE ciclos, fazer poda em toda população
        if (numeroCiclos % PRUNE_CADENCE == 0) {
            
            for (qint32 idSaida = 0; idSaida < qtSaidas; ++idSaida) {
                QVector<Cromossomo> &pop = m_state->Adj.Pop[idSaida];
                
                for (qint32 i = 0; i < pop.size(); ++i) {
                    qint32 removed = m_chromoSvc->probabilisticTermPruning(
                        pop[i],
                        0.01,   // threshold 1%
                        0.30    // até 30%
                    );
                    
                    if (removed > 0) {
                        qDebug() << "[Poda @Gen" << numeroCiclos << "]"
                                << "Removidos" << removed << "termos de Ind" << i;
                    }
                }
            }
            
            // Re-rankear população após podas
            for (qint32 idSaida = 0; idSaida < qtSaidas; ++idSaida) {
                qSortPop(0, tamPop - 1, idSaida);
            }
        }
    }
}

// ============================================================================
// OPÇÃO 3: Poda adaptativa (mais inteligente)
// ============================================================================
// Localização: EvolutionEngine::run(), acoplada com critério de estagnação
// Desvantagem: Mais complexa
// Vantagem: Dispara apenas quando necessário

struct PruneState {
    qint32 stalledGenerations = 0;
    qreal lastBestFitness = 1e9;
    qint32 lastPruneGen = 0;
};

void integrateAdaptive_SMART()
{
    PruneState pruneState;
    const qint32 MIN_GEN_BETWEEN_PRUNES = 5;
    const qreal FITNESS_IMPROVEMENT_THRESHOLD = 0.001;  // 0.1% melhoria
    
    // ... loop main ...
    for (numeroCiclos = 0; numeroCiclos < maxGeneracoes; ++numeroCiclos) {
        
        // ... evolução normal ...
        
        // Verificar se população está estagnada
        for (qint32 idSaida = 0; idSaida < qtSaidas; ++idSaida) {
            qreal bestFitness = m_state->Adj.Pop[idSaida].at(0).aptidao;
            
            // Calcular melhoria relativa
            qreal improvementRatio = 
                (pruneState.lastBestFitness - bestFitness) / 
                qAbs(pruneState.lastBestFitness);
            
            if (improvementRatio < FITNESS_IMPROVEMENT_THRESHOLD) {
                pruneState.stalledGenerations++;
            } else {
                pruneState.stalledGenerations = 0;
                pruneState.lastBestFitness = bestFitness;
            }
            
            // Se estagnado POR MUITO TEMPO, fazer poda agressiva
            if (pruneState.stalledGenerations >= 3 &&
                (numeroCiclos - pruneState.lastPruneGen) >= MIN_GEN_BETWEEN_PRUNES) {
                
                qDebug() << "[Poda Adaptativa @Gen" << numeroCiclos << "]"
                        << "Estagnação detectada após" << pruneState.stalledGenerations 
                        << "gerações";
                
                // Poda em toda população
                for (qint32 i = 0; i < m_state->Adj.Pop[idSaida].size(); ++i) {
                    qint32 removed = m_chromoSvc->probabilisticTermPruning(
                        m_state->Adj.Pop[idSaida][i],
                        0.02,   // threshold mais alto (2%) para poda agressiva
                        0.40    // removalRate mais alto (40%)
                    );
                    
                    if (removed > 0) {
                        qDebug() << "  Indivíduo" << i << ": removidos" << removed;
                    }
                }
                
                // Re-rankear
                qSortPop(0, tamPop - 1, idSaida);
                pruneState.lastPruneGen = numeroCiclos;
                pruneState.stalledGenerations = 0;  // Reset contador
            }
        }
    }
}

// ============================================================================
// OPÇÃO 4: Análise diagnóstica (uso offline, não integrado)
// ============================================================================
// Localização: Função separada para Debug/Análise pós-evolução
// Uso: Chamar após encontrar melhor solução

void diagnosticAnalysis(ChromosomeService *chromoSvc, const Cromossomo &bestChromo)
{
    qDebug() << "\n========== DIAGNÓSTICO DE IMPORTÂNCIA ==========";
    qDebug() << "Cromossomo: " << bestChromo.regress.size() << "regressores";
    
    // Calcular importância de todos os termos
    QVector<QVector<qreal>> termImportance;
    bool ok = chromoSvc->calculateTermImportance(bestChromo, termImportance);
    
    if (!ok) {
        qDebug() << "ERRO: Não foi possível calcular importância";
        return;
    }
    
    // Exibir para cada regressor
    qint32 totalTerms = 0;
    qreal avgImportance = 0.0;
    
    for (qint32 r = 0; r < termImportance.size(); ++r) {
        qDebug() << "\n--- Regressor" << r << "(" << termImportance.at(r).size() << "termos) ---";
        
        for (qint32 t = 0; t < termImportance.at(r).size(); ++t) {
            qreal imp = termImportance.at(r).at(t);
            totalTerms++;
            avgImportance += imp;
            
            QString bar;
            qint32 bars = (qint32)(imp * 20);  // 20-char bar
            for (int i = 0; i < bars; ++i) bar += "█";
            for (int i = bars; i < 20; ++i) bar += "░";
            
            qDebug() << qSetRealNumberPrecision(3)
                    << "  T" << t << ": " << bar 
                    << " " << (imp * 100) << "%";
        }
    }
    
    avgImportance /= totalTerms;
    
    qDebug() << "\n========== RESUMO ==========";
    qDebug() << "Total de termos:        " << totalTerms;
    qDebug() << "Importância média:      " << (avgImportance * 100) << "%";
    qDebug() << "MSE atual:              " << bestChromo.erro;
    qDebug() << "BIC atual:              " << bestChromo.aptidao;
    qDebug() << "=========================================\n";
}

// ============================================================================
// EXEMPLO DE CHAMADA
// ============================================================================

/*
    No main.cpp ou ThreadWorker::run(), após encontrar melhor cromossomo:

    // Análise diagnóstica (opcional)
    diagnosticAnalysis(chromoSvc, melhorCromossomo);
    
    // Poda se desejado
    if (confirmaUserDialog("Deseja fazer poda probabilística?")) {
        chromoSvc->probabilisticTermPruning(
            melhorCromossomo,
            0.01,
            0.25
        );
        qDebug() << "Cromossomo podado. BIC novo:" << melhorCromossomo.aptidao;
    }
*/

// ============================================================================
// RECOMENDAÇÕES DE USO
// ============================================================================

/*
 * 1. PARA RAPIDEZ (evolução rápida):
 *    Use OPÇÃO 2 (cadência baixa, a cada 10-20 gerações)
 *    Parâmetros: threshold=0.01, removeRate=0.25
 * 
 * 2. PARA EVOLUÇÃO LONGA (horas):
 *    Use OPÇÃO 3 (adaptativa, quando estagna)
 *    Parâmetros: threshold=0.02, removeRate=0.30
 *    Benefício: Remove estrutura inútil quando evolução trava
 * 
 * 3. PARA DESENVOLVIMENTO/DEBUG:
 *    Use OPÇÃO 4 (diagnóstico offline)
 *    Analisar melhor cromossomo ao final
 * 
 * 4. NÃO RECOMENDADO:
 *    Evitar OPÇÃO 1 (poda a cada trial)
 *    Muito custoso computacionalmente (~50-100× mais lento)
 */
