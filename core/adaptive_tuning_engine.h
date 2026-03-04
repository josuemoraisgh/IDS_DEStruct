#ifndef ADAPTIVE_TUNING_ENGINE_H
#define ADAPTIVE_TUNING_ENGINE_H

#include <QtGlobal>
#include <QVector>
#include <QMap>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
/// Snapshot de indicadores calculados em uma geração.
///////////////////////////////////////////////////////////////////////////////
struct IndicatorSnapshot {
    // Indicadores globais da população
    qreal jn_best = 0.0;                    ///< Melhor fitness da população
    qreal jn_median = 0.0;                  ///< Mediana de fitness
    qreal diversity = 0.0;                  ///< jn_median - jn_best
    qreal delta_jn = 0.0;                   ///< Melhoria do best em relação ao ciclo anterior
    qint64 generation = 0;                  ///< Geração/ciclo

    // Indicadores estruturais do melhor modelo
    qint32 num_regressores_best = 0;        ///< Quantidade de regressores
    qreal mean_terms_best = 0.0;            ///< Média de termos por regressor
    qreal mean_monomial_degree = 0.0;       ///< Grau médio dos monômios
    qreal effective_lag_weight = 0.0;       ///< Peso relativo de atrasos
    qreal ratio_input = 0.0;                ///< E_in / (E_in + E_out), entrada vs saída

    // Indicadores da população
    qreal entropy_structure = 0.0;          ///< Entropia estrutural dos regressores
    qint32 mean_regressores_population = 0; ///< Média de regressores na população
    qreal mean_terms_population = 0.0;      ///< Média de termos na população
    qreal mean_monomial_deg_population = 0.0; ///< Grau monomial médio da população
    qreal entropy_population = 0.0;         ///< Entropia estrutural da população
    qreal novelty_score = 0.0;              ///< Structural novelty (distância média ao vizinho)

    // Indicadores de ganho de informação
    qreal info_gain = 0.0;                  ///< 1 - (MSE_model / MSE_baseline)
    qreal mse_baseline = 0.0;               ///< Erro da persistência y(k) = y(k-1)
    qreal mse_model = 0.0;                  ///< Erro do modelo

    // Flag de convergência/estagnação
    bool is_stagnant = false;               ///< Detectou estagnação
    bool is_premature_convergence = false;  ///< Detectou convergência prematura
};

///////////////////////////////////////////////////////////////////////////////
/// Estrutura para adaptação de parâmetros.
///////////////////////////////////////////////////////////////////////////////
struct AdaptiveParameters {
    qreal pruning_threshold = 0.01;         ///< Limiar de importância
    qreal removal_rate = 0.30;              ///< Taxa de remoção
    qreal trial_removal_rate = 0.25;        ///< Taxa de remoção em trial
    qreal penalty_complexity = 1.0;         ///< Penalidade de complexidade (multiplicador)
    qreal penalty_lag = 1.0;                ///< Penalidade de atrasos (multiplicador)
    qreal penalty_autoregressive = 1.0;     ///< Penalidade de termos autorregressivos
    qint32 max_output_lag_allowed = -1;     ///< Max lag de saída permitido (-1=bloqueado, 0=lag0, 1=lag0-1, etc)
};

///////////////////////////////////////////////////////////////////////////////
/// Motor de auto-tuning adaptativo.
///////////////////////////////////////////////////////////////////////////////
class AdaptiveTuningEngine {
public:
    AdaptiveTuningEngine();

    /// Executado a cada 5 gerações para atualizar parâmetros adaptativos.
    /// Popula 'snapshot' com indicadores calculados e retorna parâmetros ajustados.
    static AdaptiveParameters updateAndGetParameters(
        const QVector<qreal>& fitness_population,
        const qint32 num_outputs,
        const qint32 generation,
        const qreal jn_best_previous,
        IndicatorSnapshot& outSnapshot
    );

    /// Registra snapshot para análise posterior
    static void recordSnapshot(const IndicatorSnapshot& snapshot);

    /// Retorna histórico de snapshots (últimas N gerações)
    static QVector<IndicatorSnapshot> getSnapshotHistory(qint32 lastN = 20);

    /// Limpa histórico
    static void clearHistory();

    /// Calcula qual é o lag máximo de saída permitido na geração atual
    /// Segue schedule progressivo: -1 -> 0 -> 1 -> ... até maxLag
    /// A cada K gerações, libera um novo lag
    static qint32 calculateMaxOutputLagAllowed(qint64 generation, qint32 totalLags,
                                               qint32 generationsPerLagRelease = 50);

private:
    static QVector<IndicatorSnapshot> s_history;

    /// Detecta estagnação: deltaJn muito pequeno por várias gerações consecutivas
    static bool detectStagnation(const QVector<IndicatorSnapshot>& history);

    /// Detecta convergência prematura: diversidade muito baixa
    static bool detectPrematureConvergence(qreal diversity, qreal entropy_population);

    /// Calcula entropia de um vetor de valores normalizados (probabilidades)
    static qreal calculateEntropy(const QVector<qreal>& probabilities);

    /// Calcula entropia estrutural da população baseada em categorias de regressores
    static qreal calculateStructuralEntropy(
        qint32 count_entrada,
        qint32 count_saida,
        qint32 count_erro,
        qint32 total
    );

    /// Aplica regras de adaptação ao snapshot
    static void applyAdaptationRules(
        const IndicatorSnapshot& snapshot,
        AdaptiveParameters& params
    );
};

#endif // ADAPTIVE_TUNING_ENGINE_H
