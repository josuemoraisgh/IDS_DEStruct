# 📑 Índice Completo - Código Refatorado

## 📊 Estatísticas

- **Total de arquivos**: 33
- **Headers (.h)**: 15
- **Implementações (.cpp)**: 12
- **Documentação (.md)**: 5
- **Projeto Qt (.pro)**: 1

## 🗂️ Estrutura Completa

### 📖 Documentação (5 arquivos)

| Arquivo | Descrição | Linhas |
|---------|-----------|--------|
| [EXECUTIVE_SUMMARY.md](EXECUTIVE_SUMMARY.md) | Resumo executivo com comparações | ~300 |
| [README.md](README.md) | Documentação principal com instruções | ~350 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Diagramas e fluxos de arquitetura | ~400 |
| [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md) | Guia de migração do código original | ~500 |
| INDEX.md (este arquivo) | Índice completo de arquivos | ~200 |

### 🔵 Domain Layer (8 arquivos)

Entidades de domínio puras, sem dependências externas.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [domain/Term.h](domain/Term.h) | ~100 | Define estrutura de termo com expoente |
| [domain/Term.cpp](domain/Term.cpp) | ~60 | Implementa operações em termos |
| [domain/Chromosome.h](domain/Chromosome.h) | ~100 | Define cromossomo do algoritmo genético |
| [domain/Chromosome.cpp](domain/Chromosome.cpp) | ~80 | Implementa operações em cromossomos |
| [domain/SimulationData.h](domain/SimulationData.h) | ~70 | Define dados de simulação |
| [domain/SimulationData.cpp](domain/SimulationData.cpp) | ~60 | Implementa normalização de dados |
| [domain/Configuration.h](domain/Configuration.h) | ~150 | Configuração completa do sistema |
| [domain/Configuration.cpp](domain/Configuration.cpp) | ~40 | Implementa reset e validação |

**Total Domain**: ~660 linhas

### 🔶 Interfaces Layer (3 arquivos)

Contratos abstratos seguindo Dependency Inversion Principle.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [interfaces/IDataRepository.h](interfaces/IDataRepository.h) | ~50 | Interface para repositório de dados |
| [interfaces/IConfigRepository.h](interfaces/IConfigRepository.h) | ~40 | Interface para repositório de config |
| [interfaces/IAlgorithmService.h](interfaces/IAlgorithmService.h) | ~70 | Interface para serviço de algoritmo |

**Total Interfaces**: ~160 linhas

### 🟢 Services Layer (4 arquivos)

Lógica de negócio e algoritmos.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [services/MathematicalService.h](services/MathematicalService.h) | ~100 | Declaração operações matemáticas |
| [services/MathematicalService.cpp](services/MathematicalService.cpp) | ~300 | Implementa cálculos (Gauss, BIC, normas) |
| [services/DifferentialEvolutionService.h](services/DifferentialEvolutionService.h) | ~120 | Declaração algoritmo DE |
| [services/DifferentialEvolutionService.cpp](services/DifferentialEvolutionService.cpp) | ~250 | Implementa algoritmo DE completo |

**Total Services**: ~770 linhas

### 🟡 Infrastructure Layer (4 arquivos)

Persistência e acesso a recursos externos.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [infrastructure/FileDataRepository.h](infrastructure/FileDataRepository.h) | ~50 | Interface repositório de arquivos |
| [infrastructure/FileDataRepository.cpp](infrastructure/FileDataRepository.cpp) | ~150 | Lê/escreve arquivos de dados |
| [infrastructure/XMLConfigRepository.h](infrastructure/XMLConfigRepository.h) | ~50 | Interface repositório XML |
| [infrastructure/XMLConfigRepository.cpp](infrastructure/XMLConfigRepository.cpp) | ~200 | Lê/escreve XML de configuração |

**Total Infrastructure**: ~450 linhas

### 🟣 Presentation Layer (4 arquivos)

Camada de apresentação seguindo padrão MVP.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [presentation/MainWindowPresenter.h](presentation/MainWindowPresenter.h) | ~100 | Declaração do presenter principal |
| [presentation/MainWindowPresenter.cpp](presentation/MainWindowPresenter.cpp) | ~200 | Lógica de apresentação (coordenação) |
| [presentation/PlotManager.h](presentation/PlotManager.h) | ~80 | Interface gerenciador de gráficos |
| [presentation/PlotManager.cpp](presentation/PlotManager.cpp) | ~250 | Implementação plotting com Qwt |

**Total Presentation**: ~630 linhas

### ⚪ Utils Layer (4 arquivos)

Utilitários matemáticos reutilizáveis.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [utils/MathVector.h](utils/MathVector.h) | ~150 | Template de vetor matemático |
| [utils/MathMatrix.h](utils/MathMatrix.h) | ~200 | Template de matriz matemática |
| [utils/RandomGenerator.h](utils/RandomGenerator.h) | ~50 | Gerador de números aleatórios |
| [utils/RandomGenerator.cpp](utils/RandomGenerator.cpp) | ~60 | Implementa distribuições estatísticas |

**Total Utils**: ~460 linhas

### 🚀 Main & Build (2 arquivos)

Ponto de entrada e configuração de build.

| Arquivo | Linhas | Responsabilidade |
|---------|--------|------------------|
| [main.cpp](main.cpp) | ~100 | Composition Root (DI manual) |
| [IDS_DEStruct_Refactored.pro](IDS_DEStruct_Refactored.pro) | ~180 | Arquivo de projeto Qt/qmake |

**Total Main**: ~280 linhas

## 📊 Resumo por Camada

| Camada | Arquivos | Linhas | % Total |
|--------|----------|--------|---------|
| Domain | 8 | ~660 | 19% |
| Interfaces | 3 | ~160 | 5% |
| Services | 4 | ~770 | 22% |
| Infrastructure | 4 | ~450 | 13% |
| Presentation | 4 | ~630 | 18% |
| Utils | 4 | ~460 | 13% |
| Main/Build | 2 | ~280 | 8% |
| Documentação | 5 | ~1750 | 2% (não conta no código) |

**Total de Código**: ~3.410 linhas  
**Total com Docs**: ~5.160 linhas

## 🎯 Métricas de Qualidade

### Complexidade por Arquivo
- ✅ Média: ~103 linhas/arquivo
- ✅ Máximo: ~300 linhas (MathematicalService.cpp)
- ✅ Mínimo: ~40 linhas (Configuration.cpp)

### Separação de Responsabilidades
- ✅ 1 responsabilidade por classe
- ✅ Métodos com < 30 linhas
- ✅ Complexidade ciclomática < 10

### Acoplamento
- ✅ Domain: 0 dependências externas
- ✅ Services: Apenas Domain + Utils
- ✅ Presentation: Apenas Interfaces
- ✅ Infrastructure: Apenas Interfaces + Domain

## 🔍 Encontrar Arquivos

### Por Funcionalidade

**Trabalhar com Cromossomos:**
- `domain/Chromosome.h/cpp`
- `services/DifferentialEvolutionService.h/cpp`

**Operações Matemáticas:**
- `utils/MathVector.h`
- `utils/MathMatrix.h`
- `services/MathematicalService.h/cpp`

**Carregar/Salvar Dados:**
- `interfaces/IDataRepository.h`
- `infrastructure/FileDataRepository.h/cpp`
- `infrastructure/XMLConfigRepository.h/cpp`

**Controlar Algoritmo:**
- `interfaces/IAlgorithmService.h`
- `services/DifferentialEvolutionService.h/cpp`
- `presentation/MainWindowPresenter.h/cpp`

**Plotar Gráficos:**
- `presentation/PlotManager.h/cpp`

### Por Conceito

**SOLID Principles:**
- SRP: Cada arquivo em cada camada
- OCP: `interfaces/` permitem extensão
- LSP: Implementações de `I*.h`
- ISP: Interfaces específicas em `interfaces/`
- DIP: `main.cpp` faz injeção manual

**Design Patterns:**
- MVP: `presentation/MainWindowPresenter.*`
- Repository: `infrastructure/*Repository.*`
- Service Layer: `services/*.cpp`
- Dependency Injection: `main.cpp`

**Testabilidade:**
- Mocks: Criar classes que implementam `interfaces/I*.h`
- Isolamento: Cada `services/*.cpp` testável isoladamente
- Fixtures: Usar `domain/*.cpp` como dados de teste

## 📝 Notas de Implementação

### Arquivos Template (Header-Only)
- `utils/MathVector.h` - Template de vetor
- `utils/MathMatrix.h` - Template de matriz

Estes não têm .cpp porque são templates C++.

### Arquivos de Interface (Header-Only)
- `interfaces/*.h` - Apenas declarações virtuais puras

Estes não têm .cpp porque são interfaces abstratas.

### Arquivos Principais
- `main.cpp` - **Único ponto de criação de objetos concretos**
- Aplica Dependency Injection manualmente
- Composition Root pattern

## 🚀 Próximos Passos

### Para Desenvolvedores

1. **Ler documentação na ordem:**
   - `EXECUTIVE_SUMMARY.md` (visão geral)
   - `README.md` (instruções detalhadas)
   - `ARCHITECTURE.md` (diagramas)
   - `MIGRATION_GUIDE.md` (migração do código)

2. **Explorar código na ordem:**
   - `domain/` (entidades)
   - `interfaces/` (contratos)
   - `services/` (lógica)
   - `presentation/` (MVP)
   - `main.cpp` (DI)

3. **Compilar e testar:**
   ```bash
   cd refactored
   mkdir build && cd build
   qmake ../IDS_DEStruct_Refactored.pro
   make
   ```

### Para Manutenção

**Adicionar nova funcionalidade:**
1. Criar classes em camada apropriada
2. Seguir princípios SOLID
3. Adicionar testes unitários
4. Atualizar documentação

**Modificar funcionalidade existente:**
1. Identificar arquivo pela tabela acima
2. Modificar apenas esse arquivo (SRP)
3. Atualizar testes
4. Verificar interfaces não mudaram

## 📧 Contato

- **Projeto**: IDS_DEStruct Refatorado
- **Instituição**: LASEC - FEELT - UFU
- **Arquitetura**: Clean Architecture + SOLID
- **Padrões**: MVP, Repository, DI, Service Layer

---

**Este índice serve como mapa completo do código refatorado. Use-o para navegar rapidamente entre arquivos e entender a organização do sistema! 🗺️**
