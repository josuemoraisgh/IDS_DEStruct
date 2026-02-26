# Diagrama da Arquitetura Refatorada

## Visão Geral em Camadas

```
┌─────────────────────────────────────────────────────────────┐
│                    PRESENTATION LAYER                        │
│  ┌──────────────────┐         ┌──────────────────┐         │
│  │ MainWindowPresenter│◄──────►│  PlotManager     │         │
│  │  (MVP Presenter)  │         │  (Visualização)  │         │
│  └────────┬──────────┘         └──────────────────┘         │
└───────────┼──────────────────────────────────────────────────┘
            │ Usa (via interfaces)
┌───────────▼──────────────────────────────────────────────────┐
│                     INTERFACES LAYER                         │
│  ┌──────────────────┐  ┌──────────────────┐                 │
│  │ IAlgorithmService│  │ IDataRepository  │                 │
│  └──────────────────┘  └──────────────────┘                 │
│  ┌──────────────────┐                                        │
│  │IConfigRepository │                                        │
│  └──────────────────┘                                        │
└──────────┬─────────────────────┬────────────────────────────┘
           │                     │
           │ Implementado por    │ Implementado por
           │                     │
┌──────────▼─────────────┐  ┌───▼───────────────────────────┐
│    SERVICES LAYER      │  │  INFRASTRUCTURE LAYER         │
│ ┌────────────────────┐ │  │ ┌──────────────────────────┐ │
│ │DifferentialEvolution│ │  │ │ FileDataRepository       │ │
│ │Service              │ │  │ │ (Arquivos texto)         │ │
│ └──────┬─────────────┘ │  │ └──────────────────────────┘ │
│        │ Usa           │  │ ┌──────────────────────────┐ │
│ ┌──────▼─────────────┐ │  │ │ XMLConfigRepository      │ │
│ │MathematicalService │ │  │ │ (Arquivos XML)           │ │
│ └────────────────────┘ │  │ └──────────────────────────┘ │
└────────┬───────────────┘  └────────────────────────────────┘
         │ Manipula
┌────────▼───────────────────────────────────────────────────┐
│                      DOMAIN LAYER                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Chromosome   │  │ Term         │  │Configuration │     │
│  │ (Entidade)   │  │ (Entidade)   │  │ (Entidade)   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│  ┌──────────────┐                                          │
│  │SimulationData│                                          │
│  │ (Entidade)   │                                          │
│  └──────────────┘                                          │
└─────────────────────────────────────────────────────────────┘
         │ Usa
┌────────▼───────────────────────────────────────────────────┐
│                       UTILS LAYER                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ MathVector   │  │ MathMatrix   │  │RandomGenerator│     │
│  │ (Template)   │  │ (Template)   │  │              │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## Fluxo de Dependências (Seguindo DIP)

```
main.cpp (Composition Root)
    │
    ├─► Cria: MathematicalService
    ├─► Cria: DifferentialEvolutionService(MathematicalService)
    ├─► Cria: FileDataRepository
    ├─► Cria: XMLConfigRepository
    │
    └─► Cria: MainWindowPresenter(
            IAlgorithmService → DifferentialEvolutionService,
            IDataRepository → FileDataRepository,
            IConfigRepository → XMLConfigRepository
        )
```

## Fluxo de Dados - Execução do Algoritmo

```
1. User Action (UI)
   │
   ▼
2. MainWindowPresenter.startAlgorithm()
   │
   ├─► Valida Configuration (Domain)
   │
   ▼
3. IAlgorithmService.initialize(Configuration)
   │
   ▼
4. DifferentialEvolutionService
   │
   ├─► Cria população inicial (Chromosome)
   ├─► MathematicalService.solveLinearSystem()
   ├─► Calcula fitness (MathematicalService.BIC)
   │
   ▼
5. Emite sinais de progresso
   │
   ▼
6. MainWindowPresenter.onAlgorithmStatusUpdated()
   │
   ▼
7. UI atualizada via sinais
```

## Fluxo de Dados - Carregar Dados

```
1. User: "Carregar Dados"
   │
   ▼
2. MainWindowPresenter.loadDataFile(fileName)
   │
   ▼
3. IDataRepository.loadData(fileName, SimulationData)
   │
   ▼
4. FileDataRepository.loadData()
   │
   ├─► Lê arquivo texto
   ├─► Parseia dados
   ├─► Cria SimulationData (Domain)
   │
   ▼
5. Atualiza Configuration.algorithmData.variables
   │
   ▼
6. Emite sinal: dataLoaded(fileName)
   │
   ▼
7. UI atualizada
```

## Separação de Responsabilidades

### Domain Layer
- **O QUE**: Entidades e regras de negócio
- **Responsabilidade**: Representar o problema
- **Não depende de**: Nada (camada mais interna)

### Services Layer  
- **O QUE**: Lógica de aplicação e algoritmos
- **Responsabilidade**: Executar casos de uso
- **Depende de**: Domain

### Infrastructure Layer
- **O QUE**: Implementações de I/O
- **Responsabilidade**: Persistência e recursos externos
- **Depende de**: Domain (através de interfaces)

### Presentation Layer
- **O QUE**: Lógica de apresentação (MVP)
- **Responsabilidade**: Coordenar UI e Services
- **Depende de**: Interfaces (não implementações)

### Interfaces Layer
- **O QUE**: Contratos abstratos
- **Responsabilidade**: Definir fronteiras entre camadas
- **Depende de**: Domain

## Princípio da Inversão de Dependências

```
ANTES (acoplado):
┌──────────────────┐
│ MainWindow       │
│                  │
├──────────────────┤
│ new DEStruct()   │◄── Acoplamento direto
│ new FileIO()     │◄── Acoplamento direto
└──────────────────┘

DEPOIS (desacoplado):
┌──────────────────────┐
│ MainWindowPresenter  │
│                      │
├──────────────────────┤
│ IAlgorithmService*   │◄── Depende de abstração
│ IDataRepository*     │◄── Depende de abstração
└──────────────────────┘
         ▲
         │ Injeta
         │
┌────────┴────────────────┐
│ main.cpp                │
│ (Composition Root)      │
├─────────────────────────┤
│ new DifferentialEvolution()
│ new FileDataRepository()
│ new MainWindowPresenter(...)
└─────────────────────────┘
```

## Testabilidade

```
TESTE UNITÁRIO - MathematicalService:
┌─────────────────────────┐
│ TestMathematicalService │
├─────────────────────────┤
│ testSolveLinearSystem() │◄── Testa isoladamente
│ testNorm()              │◄── Sem dependências
│ testBIC()               │◄── Rápido e confiável
└─────────────────────────┘

TESTE DE INTEGRAÇÃO - Presenter:
┌──────────────────────────────┐
│ TestMainWindowPresenter      │
├──────────────────────────────┤
│ MockAlgorithmService         │◄── Mock da interface
│ MockDataRepository           │◄── Mock da interface
│ MainWindowPresenter(mocks)   │
│                              │
│ testLoadData()               │◄── Testa lógica
│ testStartAlgorithm()         │    sem I/O real
└──────────────────────────────┘
```

## Extensibilidade

### Adicionar Novo Algoritmo (PSO):

```
1. Criar classe:
   class ParticleSwarmService : public IAlgorithmService { ... }

2. Em main.cpp, trocar:
   auto algo = new ParticleSwarmService(mathService);

3. Zero mudanças em:
   - MainWindowPresenter
   - Infrastructure
   - Domain
```

### Adicionar Banco de Dados:

```
1. Criar classe:
   class DatabaseRepository : public IDataRepository { ... }

2. Em main.cpp, trocar:
   auto dataRepo = new DatabaseRepository(connectionString);

3. Zero mudanças em:
   - MainWindowPresenter
   - Services
   - Domain
```

Isso demonstra **Open/Closed Principle**: aberto para extensão, fechado para modificação!
