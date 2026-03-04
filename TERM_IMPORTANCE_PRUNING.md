# Sistema de Análise de Importância de Termos e Poda Probabilística

## Visão Geral

Este documento descreve as três novas funções implementadas em `ChromosomeService` para analisar e remover termos redundantes ou pouco impactantes em regressores NARX:

1. **`calculateTermImportance()`** — Calcula importância relativa de cada termo
2. **`evaluateWithoutTerm()`** — Avalia MSE removendo temporariamente um termo
3. **`probabilisticTermPruning()`** — Remove termos com baixa importância usando roleta

---

## Problema: Regressores Inchados

Muitas vezes durante evolução diferencial, cromossomos crescem com:
- Termos redundantes (repetindo informação)
- Termos que se anulam (cancelamento)
- Termos com efeito negligenciável no modelo

**Sintoma**: MSE permanece igual, mas nº de termos cresce → BIC aumenta → modelo piora.

**Solução**: Análise de **sensibilidade termo-a-termo** para remover inúteis.

---

## Função 1: `calculateTermImportance()`

### Assinatura

```cpp
bool ChromosomeService::calculateTermImportance(
    const Cromossomo &cr,
    QVector<QVector<qreal>> &termImportance
) const;
```

### Descrição

Calcula a importância relativa de **cada termo** em **cada regressor** usando análise de sensibilidade:

1. **Baseline**: Avalia MSE com cromossomo completo (MSE₀)
2. **Para cada regressor**:
   - Para cada termo nele:
     - Chamar `evaluateWithoutTerm()` → MSE₁
     - Calcular: **Importância = (MSE₁ - MSE₀) / MSE₀**
     - Normalizar em [0, 1]

### Semântica

- **Importância ≈ 0%**: Termo não faz diferença (removível)
- **Importância ≈ 50%**: Termo contribui moderadamente
- **Importância ≈ 100%**: Termo crítico para o modelo

### Retorno

```cpp
QVector<QVector<qreal>> termImportance;
//  termImportance[regIdx][termIdx] ∈ [0.0, 1.0]
```

### Exemplo de Uso

```cpp
Cromossomo cr = ... // cromossomo com 3 regressores

QVector<QVector<qreal>> importances;
bool ok = chromService.calculateTermImportance(cr, importances);

if (ok) {
    for (int r = 0; r < importances.size(); ++r) {
        qDebug() << "Regressor" << r << ":";
        for (int t = 0; t < importances[r].size(); ++t) {
            qreal imp = importances[r][t];
            qDebug() << "  Termo" << t << ": importância =" << imp;
        }
    }
}

// Saída esperada:
// Regressor 0:
//   Termo 0: importância = 0.95  (crítico)
//   Termo 1: importância = 0.45  (moderado)
//   Termo 2: importância = 0.02  (descartável)
// Regressor 1:
//   Termo 0: importância = 0.88
//   ...
```

---

## Função 2: `evaluateWithoutTerm()`

### Assinatura

```cpp
qreal ChromosomeService::evaluateWithoutTerm(
    const Cromossomo &cr,
    qint32 regIdx,
    qint32 termIdx
) const;
```

### Descrição

Avalia o **MSE do cromossomo sem um termo específico**.

**Algoritmo**:
1. Copiar cromossomo
2. Remover termo `[regIdx][termIdx]`
3. Se regressor ficar vazio → retornar 1e6 (penalidade)
4. Chamar `evaluate()` para re-calcular BIC/MSE
5. Retornar novo MSE

### Retorno

- **MSE com o termo removido** (valor positivo)
- **-1** se índices inválidos
- **1e6** se regressor fica vazio (penalidade)

### Uso Interno

Chamada automaticamente por `calculateTermImportance()`, mas pode ser usada diretamente:

```cpp
Cromossomo cr = ...;

// Avaliar MSE removendo termo 2 do regressor 0
qreal mseWithout = chromService.evaluateWithoutTerm(cr, 0, 2);

if (mseWithout > cr.erro) {
    qDebug() << "Termo importante: piora" << (mseWithout - cr.erro);
} else {
    qDebug() << "Termo inútil: não piora";
}
```

---

## Função 3: `probabilisticTermPruning()`

### Assinatura

```cpp
qint32 ChromosomeService::probabilisticTermPruning(
    Cromossomo &cr,
    qreal importanceThreshold = 0.01,  // 1%
    qreal removalRate = 0.3             // até 30%
) const;
```

### Descrição

Remove termos com **baixa importância** usando **roleta probabilística**.

**Algoritmo**:
1. Calcular importância de todos os termos (via `calculateTermImportance()`)
2. Para cada regressor:
   - Definir máximo de remoções: `max = tamanho × removalRate`
   - Iterar termos (de trás para frente):
     - Se `importância < threshold`:
       - Probabilidade de remoção = `1 - (importância / threshold)`
       - Sortear se remove
     - Respeitar limite máximo
3. Re-avaliar cromossomo

### Parâmetros

| Parâmetro | Default | Significado |
|-----------|:-------:|-----------|
| `importanceThreshold` | 0.01 | Limiar (1%): termos com imp < 1% têm alta chance de remoção |
| `removalRate` | 0.3 | Até 30% dos termos podem ser removidos por regressor |

### Retorno

Número total de termos removidos.

### Exemplo de Uso

```cpp
Cromossomo cr = ...;

// Remover termos com importância < 5%, até 40% por regressor
qint32 removed = chromService.probabilisticTermPruning(
    cr,
    0.05,   // threshold 5%
    0.40    // removalRate 40%
);

qDebug() << "Termos removidos:" << removed;
qDebug() << "BIC novo:" << cr.aptidao;
qDebug() << "MSE novo:" << cr.erro;
```

### Comportamento da Roleta

Para cada termo com `importância < threshold`:

$$
P_{remoção} = 1 - \frac{importância}{threshold}
$$

| Importância | P(remoção) | Interpretação |
|:----------:|:--------:|-----------|
| 0.00% | 100% | Termo inútil → quase certeza de remoção |
| 0.02% | 60% | Termo muito fraco → provável remoção |
| 0.04% | 20% | Termo fraco → pode ser removido |
| 0.05% (threshold) | 0% | Limite → nunca remove |

**Exemplo**: `removeRate=0.3`, regressor com 10 termos
- Máximo de remoções = `10 × 0.3 = 3 termos`
- Mesmo que 8 termos tenham P(remoção)=100%, remove apenas 3

---

## Integração Sugerida

### 1. Após ERR e antes de `evaluate()`

```cpp
// Em EvolutionEngine::run() ou ChromosomeService::evaluate()

Cromossomo trial = ... ;
calcERR(trial, 0.95);  // ERR padrão

// Agora limpar termos fracos
qint32 removed = trial->probabilisticTermPruning(
    trial,
    0.01,   // threshold 1%
    0.25    // até 25%
);

if (removed > 0) {
    qDebug() << "Removidos" << removed << "termos após ERR";
}

evaluate(trial, 1);  // Re-avaliar
```

### 2. Análise Diagnóstica (Debug)

```cpp
// Para entender composição do melhor cromossomo
QVector<QVector<qreal>> importances;
chromService->calculateTermImportance(bestChromosome, importances);

for (int r = 0; r < bestChromosome.regress.size(); ++r) {
    double avgImp = 0;
    for (double imp : importances[r]) avgImp += imp;
    avgImp /= importances[r].size();
    
    qDebug() << "Regressor" << r 
             << ": tamanho=" << importances[r].size()
             << "importância-média=" << avgImp;
}
```

### 3. Poda Multietapa

```cpp
// Iterativo: remover fracos, depois medium

// Etapa 1: remover muito fracos (< 0.5%)
trial.probabilisticTermPruning(trial, 0.005, 0.2);

// Etapa 2: remover fracos (< 2%)
trial.probabilisticTermPruning(trial, 0.02, 0.15);

// Resultado: modelo compacto
```

---

## Complexidade Computacional

### `calculateTermImportance()`

- **N** = número total de termos em cromossomo
- **Para cada termo**: chamar `evaluate()` (O(M) onde M = amostras)
- **Total**: **O(N × M)**

⚠️ **Custoso!** Avaliar ~50 termos × 1000 amostras = 50k operações.

**Recomendação**: Usar apenas:
- Em cromossomos finais (diagnóstico)
- Em modo debug/análise
- Não a cada geração (muito lento)

### `probabilisticTermPruning()`

- Chama `calculateTermImportance()`: **O(N × M)**
- Iteração para poda: **O(N)**
- Re-avaliação: **O(M)**
- **Total**: **O(N × M)**

**Recomendação**: Aplicar **seletivamente**:
- À cada K gerações (ex: a cada 10)
- Apenas em trials que melhoram
- OU ao final da evolução

---

## Exemplo Prático Completo

### Cenário: Cromossomo Inchado do Benchmark

```
Cromossomo encontrado na geração 20:
  Regressor 0: 5 termos [x₁(k), x₂(k-1), x₃(k-2), x₁(k-3), y(k-1)]
  Regressor 1: 4 termos [x₂(k), x₃(k-1), x₁(k-5), erro(k-1)]
  Total: 9 termos
  MSE: 0.052
  BIC: -125.3

Hipótese: alguns termos não contribuem tanto
```

### Aplicação

```cpp
Cromossomo cr = ... ;
qreal mseBefore = cr.erro;
qint32 termsBefore = 0;
for (auto &reg : cr.regress) termsBefore += reg.size();

// Análise
QVector<QVector<qreal>> imp;
chromService.calculateTermImportance(cr, imp);

// Diagnóstico
qDebug() << "DIAGNÓSTICO PRÉ-PODA:";
for (int r = 0; r < imp.size(); ++r) {
    for (int t = 0; t < imp[r].size(); ++t) {
        qDebug() << "Reg" << r << "Term" << t 
                 << "importância:" << QString::number(imp[r][t], 'f', 3);
    }
}

// Poda
qint32 removed = chromService.probabilisticTermPruning(cr, 0.01, 0.3);

qreal mseAfter = cr.erro;
qint32 termsAfter = 0;
for (auto &reg : cr.regress) termsAfter += reg.size();

// Resultado
qDebug() << "\nRESULTADO PÓS-PODA:";
qDebug() << "Termos removidos:" << removed;
qDebug() << "Estrutura:" << termsBefore << "→" << termsAfter;
qDebug() << "MSE:" << QString::number(mseBefore, 'f', 4) 
         << "→" << QString::number(mseAfter, 'f', 4);
qDebug() << "BIC:" << QString::number(cr.aptidao, 'f', 1);
```

### Saída Esperada

```
DIAGNÓSTICO PRÉ-PODA:
Reg 0 Term 0 importância: 0.950
Reg 0 Term 1 importância: 0.420
Reg 0 Term 2 importância: 0.280
Reg 0 Term 3 importância: 0.008  ← fraco
Reg 0 Term 4 importância: 0.550
Reg 1 Term 0 importância: 0.750
Reg 1 Term 1 importância: 0.002  ← muito fraco
Reg 1 Term 2 importância: 0.450
Reg 1 Term 3 importância: 0.600

Termo removido: Reg0 Term3 Importância: 0.008
Termo removido: Reg1 Term1 Importância: 0.002

RESULTADO PÓS-PODA:
Termos removidos: 2
Estrutura: 9 → 7
MSE: 0.0520 → 0.0521
BIC: -125.3 → -130.2
```

**Análise**: MSE praticamente igual (+0.02%), mas **5 termos removidos** → BIC melhora 4.9 pontos!

---

## Boas Práticas

✅ **Fazer**:
- Usar `calculateTermImportance()` para diagnóstico
- Aplicar `probabilisticTermPruning()` em cadência baixa (a cada 10+ gerações)
- Combinar com ERR para máxima limpeza

❌ **Não Fazer**:
- Chamar `calculateTermImportance()` a cada geração (muito lento)
- Usar `removalRate > 0.5` (risco de remover termos críticos)
- Aplicar sem considerar impacto em BIC

---

## Referências

- **Sensitivity Analysis**: Remover feature, medir mudança em métrica
- **ANOVA**: Decomposição de variância (similar em conceito)
- **Probabilistic Pruning**: Seleção estocástica vs. determinística (permite escape de ótimos locais)
