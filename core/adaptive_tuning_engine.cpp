#include "adaptive_tuning_engine.h"
#include <algorithm>
#include <numeric>
#include <QDebug>

QVector<IndicatorSnapshot> AdaptiveTuningEngine::s_history;

AdaptiveTuningEngine::AdaptiveTuningEngine()
{
}

///////////////////////////////////////////////////////////////////////////////
/// Essa função é o coração do sistema adaptativo.
/// Calculam-se todos os indicadores e aplica-se as regras de adaptação.
///////////////////////////////////////////////////////////////////////////////
AdaptiveParameters AdaptiveTuningEngine::updateAndGetParameters(
    const QVector<qreal>& fitness_population,
    const qint32 num_outputs,
    const qint32 generation,
    const qreal jn_best_previous,
    IndicatorSnapshot& outSnapshot
)
{
    AdaptiveParameters params;

    // ------- Indicadores globais da população -------
    outSnapshot.generation = generation;

    // Jn_best e Jn_median
    QVector<qreal> sorted_fitness = fitness_population;
    std::sort(sorted_fitness.begin(), sorted_fitness.end());

    outSnapshot.jn_best = sorted_fitness.first();
    outSnapshot.jn_median = sorted_fitness.at(sorted_fitness.size() / 2);
    outSnapshot.diversity = outSnapshot.jn_median - outSnapshot.jn_best;

    // Delta Jn: melhoria em relação à geração anterior
    outSnapshot.delta_jn = jn_best_previous - outSnapshot.jn_best; // positivo = melhoria
    if (generation == 0) {
        outSnapshot.delta_jn = 0.0;
    }

    // ------- Detecção de estagnação e convergência prematura -------
    recordSnapshot(outSnapshot); // grava no histórico
    auto history = getSnapshotHistory(10);

    outSnapshot.is_stagnant = detectStagnation(history);
    outSnapshot.is_premature_convergence = detectPrematureConvergence(
        outSnapshot.diversity,
        outSnapshot.entropy_population
    );

    // ------- Cálculo de indicadores estruturais (simplificado por enquanto) -------
    // Nota: no futuro, esses valores virão da análise real dos cromossomos.
    // Por enquanto, usamos valores padrão para demonstração.

    outSnapshot.num_regressores_best = 3 + (generation % 5); // placeholder
    outSnapshot.mean_terms_best = 2.5 + (generation % 3) * 0.5;
    outSnapshot.mean_monomial_degree = 1.8 + (generation % 4) * 0.2;
    outSnapshot.effective_lag_weight = 0.6 + (generation % 5) * 0.05;
    outSnapshot.ratio_input = 0.4 + (generation % 7) * 0.05;

    // ------- Entropia estrutural -------
    // Simplificado: assumir distribuição uniforme entre entrada, saída e erro
    qint32 total_count = 100; // fictício
    qint32 count_entrada = 40;
    qint32 count_saida = 35;
    qint32 count_erro = 25;

    outSnapshot.entropy_structure = calculateStructuralEntropy(
        count_entrada, count_saida, count_erro, total_count
    );

    // ------- Indicadores da população -------
    outSnapshot.mean_regressores_population = 3;
    outSnapshot.mean_terms_population = 2.4;
    outSnapshot.mean_monomial_deg_population = 1.7;

    // Entropia estrutural da população: mede diversidade estrutural
    // Vetor de características [numRegressores, meanTerms, meanMonomialDegree]
    QVector<qreal> pop_features;
    pop_features << (qreal)outSnapshot.mean_regressores_population
                 << outSnapshot.mean_terms_population
                 << outSnapshot.mean_monomial_deg_population;

    // Normalizar e calcular entropia
    qreal max_feature = *std::max_element(pop_features.begin(), pop_features.end());
    if (max_feature > 0) {
        for (auto& f : pop_features) {
            f /= max_feature;
        }
    }
    outSnapshot.entropy_population = calculateEntropy(pop_features);

    // Novelty score: distância média ao vizinho (simplificado)
    outSnapshot.novelty_score = outSnapshot.entropy_population * 0.5 + 
                                 outSnapshot.diversity * 0.5;

    // ------- Ganho de informação -------
    // Baseline: persistência y(k) = y(k-1)
    // Assumir que a população tem erro menor que baseline
    outSnapshot.mse_baseline = 0.5; // fictício
    outSnapshot.mse_model = outSnapshot.jn_best; // usar Jn como proxy para erro
    outSnapshot.info_gain = 1.0 - (outSnapshot.mse_model / (outSnapshot.mse_baseline + 1e-10));

    // ------- Aplicar regras de adaptação -------
    applyAdaptationRules(outSnapshot, params);

    // ===== CONTROLE PROGRESSIVO DE TERMOS DE SAÍDA =====
    // Calcula qual lag máximo de saída deve estar liberado nesta geração
    // Começa bloqueado (-1), depois libera progressivamente a cada 50 gerações
    const qint32 totalLags = 30; // máximo de atrasos possível
    params.max_output_lag_allowed = calculateMaxOutputLagAllowed(generation, totalLags, 50);

    return params;
}

///////////////////////////////////////////////////////////////////////////////
void AdaptiveTuningEngine::recordSnapshot(const IndicatorSnapshot& snapshot)
{
    s_history.append(snapshot);

    // Manter apenas últimas 100 gerações
    if (s_history.size() > 100) {
        s_history.removeFirst();
    }
}

///////////////////////////////////////////////////////////////////////////////
QVector<IndicatorSnapshot> AdaptiveTuningEngine::getSnapshotHistory(qint32 lastN)
{
    if (s_history.size() <= lastN) {
        return s_history;
    }
    return s_history.mid(s_history.size() - lastN);
}

///////////////////////////////////////////////////////////////////////////////
void AdaptiveTuningEngine::clearHistory()
{
    s_history.clear();
}

///////////////////////////////////////////////////////////////////////////////
bool AdaptiveTuningEngine::detectStagnation(const QVector<IndicatorSnapshot>& history)
{
    if (history.size() < 3) {
        return false;
    }

    // Estagnação: deltaJn < 0.0001 por 3 gerações consecutivas
    qint32 stagnant_count = 0;
    for (int i = history.size() - 3; i < history.size(); ++i) {
        if (history.at(i).delta_jn < 0.0001) {
            stagnant_count++;
        }
    }

    return stagnant_count >= 2; // 2 de 3 últimas gerações estagnadas
}

///////////////////////////////////////////////////////////////////////////////
bool AdaptiveTuningEngine::detectPrematureConvergence(
    qreal diversity,
    qreal entropy_population
)
{
    // Convergência prematura detectada se:
    // 1. Diversidade muito baixa (jn_median ≈ jn_best)
    // 2. Entropia estrutural muito baixa (população homogênea)

    bool low_diversity = diversity < 0.01; // limiar de diversidade
    bool low_structure_entropy = entropy_population < 0.3; // limiar de entropia

    return low_diversity || low_structure_entropy;
}

///////////////////////////////////////////////////////////////////////////////
qreal AdaptiveTuningEngine::calculateEntropy(const QVector<qreal>& probabilities)
{
    if (probabilities.isEmpty()) {
        return 0.0;
    }

    qreal total = std::accumulate(probabilities.begin(), probabilities.end(), 0.0);
    if (total <= 0.0) {
        return 0.0;
    }

    qreal entropy = 0.0;
    for (qreal p : probabilities) {
        qreal prob = p / total;
        if (prob > 1e-10) { // evitar log(0)
            entropy -= prob * std::log(prob);
        }
    }

    return entropy;
}

///////////////////////////////////////////////////////////////////////////////
qreal AdaptiveTuningEngine::calculateStructuralEntropy(
    qint32 count_entrada,
    qint32 count_saida,
    qint32 count_erro,
    qint32 total
)
{
    QVector<qreal> counts;
    counts << count_entrada << count_saida << count_erro;

    return calculateEntropy(counts);
}

///////////////////////////////////////////////////////////////////////////////
void AdaptiveTuningEngine::applyAdaptationRules(
    const IndicatorSnapshot& snapshot,
    AdaptiveParameters& params
)
{
    // Regra 1: Convergência prematura → aumentar diversidade
    if (snapshot.is_premature_convergence) {
        params.removal_rate = 0.40; // Relaxar poda
        params.trial_removal_rate = 0.35;
        params.penalty_complexity *= 0.8; // Reduzir penalidade para permitir mais termos
        qDebug() << "AUTO-TUNE: Convergência prematura detectada - relaxando poda";
    }

    // Regra 2: Estagnação → aumentar penalização estrutural
    if (snapshot.is_stagnant) {
        params.pruning_threshold = 0.015; // Aumentar limiar
        params.penalty_complexity *= 1.3; // Penalizar mais termos
        params.trial_removal_rate = 0.35;
        qDebug() << "AUTO-TUNE: Estagnação detectada - aumentando penalização";
    }

    // Regra 3: Modelos muito complexos
    if (snapshot.mean_terms_best > 4.0) {
        params.penalty_complexity *= 1.15;
        params.removal_rate -= 0.05; // Aumentar poda
        qDebug() << "AUTO-TUNE: Complexidade alta - aumentando poda";
    }

    // Regra 4: Modelo dominado por saída (baixo ratio_input)
    if (snapshot.ratio_input < 0.3) {
        params.penalty_autoregressive *= 1.2; // Penalizar termos y(k-d)
        qDebug() << "AUTO-TUNE: Modelo autorregressivo demais - penalizando";
    }

    // Regra 5: Atrasos excessivos
    if (snapshot.effective_lag_weight > 0.8) {
        params.penalty_lag *= 1.2;
        qDebug() << "AUTO-TUNE: Atrasos excessivos - penalizando";
    }

    // Regra 6: Baixa diversidade estrutural da população
    if (snapshot.novelty_score < 0.25) {
        params.penalty_complexity *= 0.9; // Relaxar penalidade para incentivar exploração
        // Nota: Injetar random immigrants seria feito no loop evolutivo
        qDebug() << "AUTO-TUNE: Baixa diversidade estrutural - injetando variação";
    }

    // Regra 7: Ganho de informação baixo
    if (snapshot.info_gain < 0.1 && snapshot.generation > 50) {
        params.penalty_complexity *= 1.1;
        params.removal_rate -= 0.03;
        qDebug() << "AUTO-TUNE: Ganho de informação baixo - simplificando modelo";
    }

    // Regra 8: Melhoria significativa → relaxar ligeiramente
    if (snapshot.delta_jn > 0.001 && !snapshot.is_stagnant) {
        params.penalty_complexity *= 0.95; // Encorajar complexidade quando melhora
        qDebug() << "AUTO-TUNE: Melhoria significativa - permitindo mais complexidade";
    }

    // Garantir limites razoáveis
    params.pruning_threshold = std::max(0.005, std::min(0.05, params.pruning_threshold));
    params.removal_rate = std::max(0.15, std::min(0.50, params.removal_rate));
    params.trial_removal_rate = std::max(0.15, std::min(0.40, params.trial_removal_rate));
    params.penalty_complexity = std::max(0.5, std::min(2.0, params.penalty_complexity));
    params.penalty_lag = std::max(0.7, std::min(2.0, params.penalty_lag));
    params.penalty_autoregressive = std::max(0.7, std::min(2.0, params.penalty_autoregressive));
}

///////////////////////////////////////////////////////////////////////////////
/// calculateMaxOutputLagAllowed — Schedule progressivo de liberação de lags
///
/// Começa bloqueado (-1), depois gradualmente libera:
/// Gen 0-49:    -1 (bloqueado - sem saídas)
/// Gen 50-99:    0 (só lag 0)
/// Gen 100-149:  1 (lag 0-1)
/// ...
/// Até liberar todos os lags disponíveis
///////////////////////////////////////////////////////////////////////////////
qint32 AdaptiveTuningEngine::calculateMaxOutputLagAllowed(qint64 generation,
                                                         qint32 totalLags,
                                                         qint32 generationsPerLagRelease)
{
    if (generation < generationsPerLagRelease) {
        return -1; // Bloqueado
    }

    // Quantos "intervalos" completamos?
    qint32 intervals = static_cast<qint32>(generation / generationsPerLagRelease);
    
    // Max lag permitido = intervals - 1 (primeira geração, intervals=1, max_lag=0)
    qint32 max_lag = intervals - 1;
    
    // Limitar ao max disponível
    return std::min(max_lag, totalLags - 1);
}

