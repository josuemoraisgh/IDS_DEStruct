#ifndef MODEL_PRUNING_CONFIG_H
#define MODEL_PRUNING_CONFIG_H

#include <QtGlobal>

///////////////////////////////////////////////////////////////////////////////
/// Configurações centralizadas de poda por importância de termos.
///
/// Objetivo:
/// - concentrar valores fixos (hardcoded) em um único lugar;
/// - facilitar ajuste fino sem alterar lógica espalhada pelo código.
///////////////////////////////////////////////////////////////////////////////
namespace ModelPruningConfig {

/// Número mínimo de termos totais no trial para ativar poda por roleta.
/// Ex.: com 4, poda só roda quando totalTerms > 4.
constexpr qint32 kMinTermsToEnablePruning = 4;

/// Ciclo mínimo para começar poda por importância durante a evolução.
/// Evita homogeneização precoce da população.
constexpr qint64 kPruningStartCycle = 20;

/// Limiar padrão de importância [0..1].
/// Termos com importância abaixo desse valor passam a ter chance de remoção.
constexpr qreal kDefaultImportanceThreshold = 0.01;

/// Taxa padrão de remoção [0..1] usada pela API pública de poda.
/// Ex.: 0.30 permite remover até 30% dos termos por regressor.
constexpr qreal kDefaultRemovalRate = 0.30;

/// Taxa de remoção [0..1] usada na poda automática durante geração de trial.
/// Normalmente um pouco mais conservadora que a API pública.
constexpr qreal kTrialRemovalRate = 0.25;

/// Penalidade de erro quando um teste remove o último termo do regressor.
/// Mantém o termo como altamente importante nesse cenário.
constexpr qreal kEmptyRegressorPenaltyMse = 1e6;

/// Quantidade mínima de termos que um regressor deve manter após poda.
constexpr qint32 kMinTermsPerRegressor = 1;

}

///////////////////////////////////////////////////////////////////////////////
/// Configurações de diversidade para mitigar convergência prematura no DE.
///////////////////////////////////////////////////////////////////////////////
namespace DEDiversityConfig {

/// Aumenta exploração inicial: top-p% usado para escolher pbest durante warmup.
constexpr qreal kWarmupPbestRate = 0.35;
constexpr qint64 kWarmupCycles = 20;

/// Amostragem adaptativa de F e CR por trial (mais diversidade estrutural).
constexpr bool kUseAdaptiveTrialFCR = true;
constexpr qreal kFMin = 0.45;
constexpr qreal kFMax = 0.95;
constexpr qreal kCRMin = 0.60;
constexpr qreal kCRMax = 0.95;

/// Random immigrants: substitui periodicamente piores indivíduos.
constexpr bool kUseRandomImmigrants = true;
constexpr qint64 kImmigrantPeriodCycles = 10;
constexpr qint64 kImmigrantStartCycle = 10;
constexpr qreal kImmigrantFraction = 0.08; // 8%

}

#endif // MODEL_PRUNING_CONFIG_H
