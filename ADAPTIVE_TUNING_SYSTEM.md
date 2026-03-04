## 🤖 Sistema de Auto-Tuning Adaptativo para IDS_DEStruct

### Visão Geral

Um **motor de auto-tuning em tempo de execução** que monitora indicadores do algoritmo evolutivo e ajusta **dinamicamente** os parâmetros de poda e diversidade a cada 5 gerações.

---

## 📊 Arquitetura

### 1. **AdaptiveTuningEngine** (`core/adaptive_tuning_engine.h/.cpp`)

**Responsabilidade**: Calcular indicadores e aplicar regras de adaptação.

#### Indicadores Calculados (Snapshot)

| Indicador | Descrição | Uso |
|-----------|-----------|-----|
| `jn_best` | Melhor fitness da população | Referência de qualidade |
| `jn_median` | Mediana de fitness | Detectar homogeneidade |
| `diversity` | `jn_median - jn_best` | Medir "gap" de qualidade |
| `delta_jn` | Melhoria por geração | Detectar estagnação |
| `entropy_population` | Entropia estrutural da população | Homogeneidade estrutural |
| `novelty_score` | Diversidade estrutural média | Inovação na população |
| `info_gain` | `1 - (MSE_model / MSE_baseline)` | Ganho em relação à persistência |

#### Métodos Principais

```cpp
// Pulmão do sistema (chamado a cada 5 gerações)
static AdaptiveParameters updateAndGetParameters(
    const QVector<qreal>& fitness_population,  // Todos os fitness
    const qint32 num_outputs,                   // Quantidade de saídas
    const qint32 generation,                    // Número da geração
    const qreal jn_best_previous,               // Melhor anterior
    IndicatorSnapshot& outSnapshot              // Retorna indicadores
);

// Histórico para análise
static QVector<IndicatorSnapshot> getSnapshotHistory(qint32 lastN = 20);
```

---

### 2. **AdaptiveState** (`core/adaptive_state.h`)

**Responsabilidade**: Armazenar com segurança entre threads.

```cpp
struct AdaptiveState {
    AdaptiveParameters current_parameters;     // Parâmetros atualizados
    IndicatorSnapshot last_snapshot;           // Último cálculo
    qint64 last_update_generation;             // Controle de frequência
    qreal previous_best_fitness;               // Para calcular delta
    mutable QMutex mutex;                      // Segurança multi-thread
};
```

**Integrado em**: `Config::adaptiveState` (xtipodados.h)

---

### 3. **AdaptiveParameters** Dinâmicos

```cpp
struct AdaptiveParameters {
    qreal pruning_threshold = 0.01;          // Limiar de importância [0.005, 0.05]
    qreal removal_rate = 0.30;               // Taxa de remoção [0.15, 0.50]
    qreal trial_removal_rate = 0.25;         // Taxa por trial [0.15, 0.40]
    qreal penalty_complexity = 1.0;          // Multiplicador [0.5, 2.0]
    qreal penalty_lag = 1.0;                 // Multiplicador [0.7, 2.0]
    qreal penalty_autoregressive = 1.0;      // Multiplicador [0.7, 2.0]
};
```

**Substituem constantes em `model_pruning_config.h`**

---

## 🔄 Fluxo de Execução

### 1. **Loop Evolutivo** (evolution_engine.cpp)

```
Para cada ciclo:
  ├─ Mutação + Avaliação (DE loop)
  ├─ Incrementa iteracoes++
  └─ [A CADA 5 GERAÇÕES]
     ├─ Coleta fitness de TODAS saídas
     ├─ Chama AdaptiveTuningEngine::updateAndGetParameters()
     ├─ Armazena em m_state->Adj.adaptiveState
     └─ Loga indicadores (DEBUG)
```

**Código em evolution_engine.cpp (linhas ~273-307)**:
```cpp
if (m_state->Adj.adaptiveState.shouldUpdate(m_state->Adj.iteracoes)) {
    // Coleta fitness global
    QVector<qreal> all_fitness;
    for (qint32 i = 0; i < qtSaidas; ++i)
        for (qint32 j = 0; j < tamPop; ++j)
            all_fitness.append(m_state->Adj.Pop[i][j].aptidao);

    // Atualiza motor
    IndicatorSnapshot snapshot;
    AdaptiveParameters new_params = AdaptiveTuningEngine::updateAndGetParameters(
        all_fitness, qtSaidas, m_state->Adj.iteracoes,
        m_state->Adj.adaptiveState.previous_best_fitness, snapshot
    );

    // Armazena
    m_state->Adj.adaptiveState.update(new_params, snapshot, m_state->Adj.iteracoes);
    m_state->Adj.adaptiveState.previous_best_fitness = crBest.at(0).aptidao;
}
```

### 2. **Poda Probabilística** (chromosome_service.cpp, linhas ~745-767)

```cpp
if (m_state->Adj.iteracoes >= ModelPruningConfig::kPruningStartCycle
    && totalTerms > ModelPruningConfig::kMinTermsToEnablePruning) {
    
    // AQUI: Busca parâmetros ADAPTATIVOS
    AdaptiveParameters params = m_state->Adj.adaptiveState.getParameters();
    qreal threshold = params.pruning_threshold > 0 
                    ? params.pruning_threshold 
                    : ModelPruningConfig::kDefaultImportanceThreshold;
    qreal removal_rate = params.trial_removal_rate > 0 
                       ? params.trial_removal_rate 
                       : ModelPruningConfig::kTrialRemovalRate;
    
    // Poda com valores DINÂMICOS
    probabilisticTermPruning(trial, threshold, removal_rate);
}
```

---

## 📈 Regras de Adaptação

### Detecta Problema → Aplica Solução

| **Problema Detectado** | **Indicador** | **Ação** | **Efeito** |
|------------------------|---------------|---------|-----------|
| **Convergência prematura** | `diversity < 0.01` ou `entropy_pop < 0.3` | ↓ `penalty_complexity` (×0.8), ↑ `removal_rate` (+0.10) | Permite mais termos, relaxa poda |
| **Estagnação** | `delta_jn < 0.0001` por 2+ ger | ↑ `penalty_complexity` (×1.3), ↓ `removal_rate` (-0.05) | Força simplificação |
| **Complexidade excessiva** | `mean_terms_best > 4.0` | ↑ `penalty_complexity` (×1.15), ↓ `removal_rate` (-0.05) | Mais agressiva em poda |
| **Dominação autoregressiva** | `ratio_input < 0.3` | ↑ `penalty_autoregressive` (×1.2) | Penaliza `y(k-d)` |
| **Atrasos longos** | `effective_lag_weight > 0.8` | ↑ `penalty_lag` (×1.2) | Restringe memória |
| **Baixa diversidade estrutural** | `novelty_score < 0.25` | ↓ `penalty_complexity` (×0.9) | Encoraja exploração |
| **Ganho de informação baixo** | `info_gain < 0.1` (após gen 50) | ↑ `penalty_complexity` (×1.1) | Força otimização |
| **Melhoria significativa** | `delta_jn > 0.001` | ↓ `penalty_complexity` (×0.95) | Explora mais |

---

## 🔒 Limites de Segurança

Todos os parâmetros têm **clamp min/max** para evitar valores patológicos:

```cpp
pruning_threshold:      [0.005,  0.05]
removal_rate:           [0.15,   0.50]
trial_removal_rate:     [0.15,   0.40]
penalty_complexity:     [0.5,    2.0]
penalty_lag:            [0.7,    2.0]
penalty_autoregressive: [0.7,    2.0]
```

---

## 📋 Exemplo: Comportamento em Tempo Real

### Geração 0
```
Jn_best: 0.5000, Jn_median: 0.5001, diversity: 0.0001
→ Convergência prematura detectada!
→ penalty_complexity *= 0.8 (permite mais complexidade)
→ removal_rate = 0.40 (relaxa poda)
```

### Geração 10
```
Jn_best: 0.4500, Jn_median: 0.4510, delta_jn: 0.0500
✓ Melhoria significativa!
→ penalty_complexity *= 0.95 (continua explorando)
```

### Geração 50
```
Jn_best: 0.4200, Jn_median: 0.4205, delta_jn: 0.00001
→ Estagnação detectada
→ penalty_complexity *= 1.3 (penaliza complexidade)
→ removal_rate: 0.25 (aumenta poda)
```

### Geração 100
```
Jn_best: 0.4100, mean_terms_best: 5.5
→ Complexidade alta
→ penalty_complexity *= 1.15
→ removal_rate: 0.20
```

---

## 🧪 Como Validar o Sistema

### 1. **Verificar Logs**
```
[AUTO-TUNE] Gen 5  | Div:0.0001 | DeltaJn:0.0500 | Stag:false | ConvPrem:true
[AUTO-TUNE] Gen 10 | Div:0.0005 | DeltaJn:0.0200 | Stag:false | ConvPrem:false
[AUTO-TUNE] Gen 15 | Div:0.0002 | DeltaJn:0.00001| Stag:true  | ConvPrem:false
```

### 2. **Comparar com/sem**
Desabilitar: Comentar em `evolution_engine.cpp` a chamada `shouldUpdate()`

### 3. **Analisar Histórico**
```cpp
auto history = AdaptiveTuningEngine::getSnapshotHistory(50);
// Plotar diversity, entropy_population, info_gain ao longo do tempo
```

---

## 🎯 Benefícios Esperados

✅ **Menos convergência prematura** → diversidade mantida mais tempo  
✅ **Ajuste automático** → sem tuning manual  
✅ **Anti-estagnação** → aumenta penalidades quando percebe platô  
✅ **Modelos simples** → reduz complexidade quando útil  
✅ **Exploração adaptativa** → permite complexidade quando gera ganho  

---

## 📦 Integração Total

```
model_pruning_config.h (constantes base)
    ↓
adaptive_tuning_engine.h/cpp (regras + cálculos)
    ↓
adaptive_state.h (armazenamento multi-thread)
    ↓
xtipodados.h (Config::adaptiveState)
    ↓
evolution_engine.cpp (coleta dados, chama engine)
    ↓
chromosome_service.cpp (usa parâmetros dinâmicos)
```

---

## 🔧 Frequência de Update

- **Coleta de dados**: A cada geração (linha 264)
- **Cálculo de adaptação**: A cada 5 gerações (line ~305)
- **Aplicação**: Imediata na próxima poda (chromosome_service)

Balança entre **responsividade** (5 gen é rápido) e **estabilidade** (não reage a ruído instantâneo).

---

## 🚀 Próximas Possibilidades

1. **Persistência em XML**: Salvar histórico de snapshots por execução
2. **UI Dinâmica**: Mostrar gráficos de indicadores em tempo real
3. **Meta-tuning**: Ajustar as **regras** de adaptação baseado em performance histórica
4. **Per-output adaptation**: Ajustes diferenciados por saída
5. **Population diversity metrics**: Usar distâncias geométricas entre indivíduos

