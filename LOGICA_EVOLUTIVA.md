# Lógica Evolutiva - IDS_DEStruct
## Core do Sistema de Identificação e Síntese de Estrutura Dinâmica

---

## 1. ESTRUTURA FUNDAMENTAL: O CROMOSSOMO

### 1.1 Representação
Todo cromossomo representa uma **equação racional recursiva com termos de erro**:

$$\mathbf{y(k) = \frac{Num(k) + E(k)}{Den(k)}}$$

Onde:
- **Num(k)**: Numerador com regressores selecionados
- **Den(k)**: Denominador com regressores (padrão: 1 se vazio)
- **E(k)**: Termos de erro recursivo (resíduos com atrasos)

### 1.2 Composição Interna

```
Cromossomo {
    regress[]:         // Array de regressores ordenados
        [0]: constante (numerador)
        [1...n]: termos de numerador
        [n+1...m]: termos de denominador
    
    vlrsCoefic[]:      // Coeficientes calculados para cada regressor
    err[]:             // Taxa de redução do erro (ERR) de cada regressor
    
    erro:              // SSE (Sum of Squared Errors)
    aptidao:           // BIC (Bayesian Information Criterion)
    maiorAtraso:       // Maior lag/atraso temporal usado
}
```

### 1.3 Termo Regressor
```
compTermo {
    var:      // ID variável (1...numVariáveis)
    atraso:   // Delay/lag temporal (0...30 amostras)
    reg:      // Ordem do regressor (numerador=1..511, denominador=1..511)
    nd:       // Flag: numerador(1) ou denominador(0)
    expoente: // Grau de não-linearidade (real, inteiro ou natural)
}
```

---

## 2. ALGORITMO DE EVOLUÇÃO DIFERENCIAL (DE)

### 2.1 Loop Principal Evolutivo

**Estrutura com Pipeline Paralelo (4 stages)**:

```
DE_Loop ∞:
    ├─ Stage 0: Seleção inicial da população
    ├─ Stage 1: Cruzamento + Mutação
    ├─ Stage 2: Avaliação + Ranking Elitista
    └─ Stage 3: Verificação de convergência
```

### 2.2 Seleção de Pais

Para cada geração, seleciona 3 cromossomos aleatoriamente:
```
cr0, cr1, cr2 = Pop[randInt(0,tamPop-1)] (sem repetição)
```

Estes representam:
- **cr0**: Base (contribui para estrutura média)
- **cr1, cr2**: Diferença evolutiva (exploração do espaço)

### 2.3 Fórmula de Mutação (Variante DE/best/2)

$$crAvali = cr0 + F \cdot (cr1 - cr2)$$

Onde:
- **F (Fator de Escala)**: ∈ [-2, 2] (aleatório por cromossomo)
- **Aplicação**: Por termo (expoente sofre mutação)

```cpp
auxTermo.expoente = cr0.exp + F*(cr1.exp - cr0.exp) + F*(cr2.exp - cr3.exp)
```

---

## 3. CRUZAMENTO (CROSSOVER) COM SELEÇÃO GENÉTICA

### 3.1 Pool de Termos Candidatos

Combina termos de 5 fontes:
```
Candidatos = cr0.termos ∪ crNew.termos ∪ cr1.termos ∪ cr2.termos ∪ crAvali.termos
```

### 3.2 Estratégia de Seleção

1. **Eliminação de duplicatas**: Termos idênticos são concatenados (expoentes somados)
2. **Bitstring de seleção**: Cada bit representa "incluir termo" (sim/não)
3. **Hierarquia de regressor**: Se numerador selecionado → denominador pode ser selecionado → erro pode ser selecionado
4. **Limite de complexidade**: máximo de `testeSize` termos por categoria

```cpp
for(i=0; i<size; i++)
    if((teste >> i) & 1)  // Bit i ligado?
        vetTermosSelecionados.append(candidatos[i])
```

### 3.4 Montagem do Novo Cromossomo

```
crA1.regress[0] = {termos de numerador selecionados}
crA1.regress[1] = {termos de denominador selecionados}
crA1.regress[2] = {termos de erro selecionados}
```

---

## 4. CÁLCULO DE APTIDÃO: ELS ESTENDIDO

### 4.1 Fase 1: Montagem de Regressores (`DES_MontaVlrs`)

Converte estrutura simbólica em matriz numérica:

$$\mathbf{A} = [Rnum | -Rden \cdot y | e(k-1) | e(k-2) | \ldots]$$

Onde:
- **Rnum**: Matriz de regressores do numerador (apenas valores)
- **-Rden·y**: Regressores denominador, multiplicados por -1 e pela saída (pseudolinearização)
- **e(k-lag)**: Vetores de erro recursivo

**Processamento por termo**:
```cpp
Para cada variável i com atraso τ e expoente α:
    valores_regressor = (var_i(k-τ))^α
    
Se numerador:
    Rnum[:, reg_index] = valores_regressor
Senão (denominador):
    Rden[:, reg_index] = valores_regressor
    auxDen[:, reg_index] = valores_regressor * (-1) * y[:]
```

### 4.2 Fase 2: ELS Estendido (`DES_calAptidao`)

**Iteração com estimação de variância de erro**:

$$[A'A - var_e \cdot \Sigma_1] \mathbf{c} = A'b - var_e \cdot \Sigma_2$$

Onde:
- **var_e**: Variância estimada dos resíduos (atualizada a cada iteração)
- **Σ₁**: Matriz de correlação cruzada dos regressores de denominador
- **Σ₂**: Vetor de soma dos regressores de denominador
- **b**: Saída do sistema (medições)

**Loop iterativo**:
```
var = 0  (inicializa com OLS - Ordinary Least Squares)
Para count = 0 até tamErro:
    
    // Resolve sistema linear
    var_novo = SolverLinear(A'A - var*Σ₁, A'b - var*Σ₂)
    
    // Calcula resíduuos com termos recursivos
    residuo[k] = y[k] - estimado[k] 
                 - Σ(coef[lag_i] * residuo[k-lag_i])
    
    // Atualiza variância
    var = cov(residuo)
    
    // Critério de convergência
    Se |c_novo - c_anterior| < 1e-3 E |var_novo - var_anterior| < 1e-3:
        break

SSE = residuo'*residuo / num_amostras
```

### 4.3 Fase 3: Cálculo de Saída Corrigida (`DES_CalcVlrsEstRes`)

**CORREÇÃO APLICADA** - Termos de erro no numerador antes da divisão:

```cpp
// Montagem correta: (Num + Erro) / (1 + Den)

vlrsEstimado = Rnum * cNum              // Numerador puro
den = 1 + Rden * cDen                   // Denominador com constante
aux = 1 + (1 + Rden * cDen)            // Inicializa com denominador

// Termos de erro somados AO NUMERADOR
Para k = 0 até tam_dados:
    Para i = 0 até tamErro:
        vlrsEstimado[k] += coef[tamNum+tamDen+i] * residuo[k-i-1]
    
    // Divisão só DEPOIS de somar erro
    vlrsEstimado[k] /= den[k]
    residuo[k] = y[k] - vlrsEstimado[k]
```

**Antes (INCORRETO)**:
$$y = \frac{R_{num} \cdot c_{num}}{1 + R_{den} \cdot c_{den}} + E_i \cdot c_e$$

**Depois (CORRETO)**:
$$y = \frac{R_{num} \cdot c_{num} + E_i \cdot c_e}{1 + R_{den} \cdot c_{den}}$$

---

## 5. APTIDÃO: CRITÉRIO BIC

$$BIC = n \cdot \ln(SSE) + (2 \cdot k + r) \cdot \ln(n)$$

Onde:
- **n**: Número de amostras
- **SSE**: Soma dos erros quadráticos
- **k**: Número de regressores (estrutural)
- **r**: Número de termos de erro recursivo
- **Objetivo**: Minimizar (menor = melhor)

**Balanceamento**: Penaliza complexidade enquanto recompensa precisão

---

## 6. SELEÇÃO E ELITISMO

### 6.1 Ordenação por Fitness

Após cada geração, população é reordenada por aptidão:

```cpp
vetElitismo[idSaida].sort(by: Pop[idSaida][index].aptidao)
// Melhores ficam no início (índices 0, 1, ...)
```

### 6.2 Estratégia Elitista

Dois modos configuráveis:
- **Modo 1**: 10% melhores indivíduos são preservados com prioridade
- **Modo 2**: Recomeça do zero (explora novo espaço)

Melhor cromossomo é sempre preservado:
```cpp
crBest = Pop[0]  // Sempre mantém o melhor

Se aptidao[melhor] não melhorou por numeroCiclos iterações:
    Entra em modo "Parada" (aguarda reinicialização)
```

---

## 7. CONVERGÊNCIA E CRITÉRIOS DE PARADA

### 7.1 Detecção de Melhoria

```cpp
Se (melhorAptidaoAnt - aptidaoAtual) >= jnrr:  // jnrr: limiar mínimo
    melhorAptidaoAnt = aptidaoAtual
    iteracoesAnt = iteracao_atual
Senão:
    Se iteracao_atual >= iteracoesAnt + numeroCiclos:
        Ativa modo "Parada"
```

### 7.2 Modelo Paralelo (Pipeline)

**4 stages processam simultaneamente**:
- Stage 0: Cria/avalia cromossomos iniciais
- Stage 1: Faz cruzamento/mutação
- Stage 2: Ordena e seleciona
- Stage 3: Prepara próximo batch

**Sincronização**:
- Barriers entre stages
- Lock-free quando possível (read mode para leitura)
- Critical sections para atualização de melhores

---

## 8. FLUXO GERAL DE UMA GERAÇÃO

```
Geração T:
    ├─ [SELEÇÃO]
    │  └─ Escolhe cr0, cr1, cr2 aleatoriamente
    │
    ├─ [MUTAÇÃO]
    │  ├─ Calcula: expoentes_novo = cr0 + F*(cr1 - cr2)
    │  └─ Gera população intermediária
    │
    ├─ [CRUZAMENTO]
    │  ├─ Pool = 5 pais (cr0, crNew, cr1, cr2, crAvali)
    │  ├─ Remove duplicatas
    │  ├─ Aplica seleção por bitstring
    │  └─ Monta novo cromossomo (crA1)
    │
    ├─ [AVALIAÇÃO - ELS Estendido]
    │  ├─ DES_MontaVlrs: Constrói matrizes de regressores
    │  ├─ DES_calAptidao: Resolve sistema linear iterativo
    │  │   └─ Itera com estimação de variância
    │  ├─ DES_CalcVlrsEstRes: Calcula saída (já corrigida)
    │  └─ Calcula BIC
    │
    ├─ [COMPARAÇÃO]
    │  └─ Se BIC(crA1) < BIC(crAvali):
    │      └─ crAvali = crA1
    │
    └─ [ELITISMO]
       ├─ Ordena população por BIC
       ├─ Verifica convergência
       └─ Emite sinal para UI (a cada 6s ou se houver melhoria)

Geração T+1: Repete com nova população
```

---

## 9. PSEUDO-CÓDIGO DA LÓGICA CORE

```
POPULACIONAL:
    Pop[saida][indiv] = Cromossomo

DE-LOOP:
    Para cada geração G:
        Para cada saída S:
            
            // Seleção
            idx0, idx1, idx2 = Random(0, tamPop-1) sem repetição
            cr0 = Pop[S][idx0]
            cr1 = Pop[S][idx1]
            cr2 = Pop[S][idx2]
            
            // Mutação por expoente
            F = Random(-2, 2)
            Para cada termo em cr0, cr1, cr2:
                exp_novo = exp_cr0 + F*(exp_cr1 - exp_cr0) + F*(exp_cr2 - exp_cr3)
            
            // Cruzamento
            candidatos = Merge(cr0, crNew, cr1, cr2, crAvaliAnt)
            RemoveDuplicatas(candidatos)
            
            teste = bitstring_aleatorio()
            Para cada i em candidatos:
                Se (teste >> i) & 1:
                    Adiciona candidato[i] ao novo cromossomo
            
            // Avaliação
            crA1 = Cromossomo_vazio()
            DES_MontaVlrs(crA1)    // Cria matriz A
            DES_calAptidao(crA1)   // ELS iterativo
            DES_CalcVlrsEstRes(crA1) // Calcula y = (Num+Err)/Den
            
            // Seleção
            Se BIC(crA1) < BIC(crAvali):
                crAvali = crA1
        
        // Elitismo global
        Ordena(Pop[S]) por BIC
        Se convergência detectada:
            Pausa ou reinicia população

Retorna: crAvali para cada saída (melhor equação encontrada)
```

---

## 10. PUNTOS CRÍTICOS DE LÓGICA

### 10.1 Pseudolinearização do Denominador

O denominador é multiplicado por **-1** e pela saída **y** durante a montagem:

```cpp
auxDen[:, reg] = -1 * y[:] * regressorDen
```

Isto transforma a equação racional não-linear:
$$y = \frac{a_0 + a_1 r_1}{1 + b_1 r_1}$$

Em uma **forma linear parametrizável**:
$$y \cdot (1 + b_1 r_1) = a_0 + a_1 r_1$$
$$y = a_0 + a_1 r_1 - b_1 r_1 \cdot y$$

Que se resolve com OLS em:
$$A'A \mathbf{c} = A'b$$

### 10.2 Termos de Erro Recursivo

Loop iterativo resolve:
- **1ª iteração**: Apenas numerador + denominador (OLS puro)
- **2ª iteração**: Inclui e(k-1) como regressor
- **3ª iteração**: Inclui e(k-1), e(k-2), ...

Parada quando |c_novo - c_anterior| < 1e-3

### 10.3 Correção da Equação de Saída

**Ponto crítico da correção**:
```
Antes: y = Num/(1+Den) + Erro
Agora: y = (Num + Erro)/(1+Den)
```

O erro deve estar **NO NUMERADOR** para ser coerente com o cálculo de aptidão via ELS.

### 10.4 Barreira de Convergência

```cpp
limiar_melhoria = jnrr (configurável)

Se melhoria >= jnrr:
    iteracoesAnt = iteracao_atual
Senão:
    Se iteracao_atual > iteracoesAnt + numeroCiclos:
        Pausa (aguarda reinicialização ou término)
```

---

## RESUMO FINAL

O IDS_DEStruct implementa um **Algoritmo de Evolução Diferencial Paralelo** com **Mínimos Quadrados Estendidos Iterativos** para sintetizar equações racionais recursivas que aproximem sistemas dinâmicos. A lógica é evolutiva (DE), mas a avaliação é analítica (ELS), criando um híbrido eficiente que escape de ótimos locais enquanto converge rapidamente para estruturas promissoras.
