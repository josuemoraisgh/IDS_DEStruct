# Arquitetura Refatorada - IDS_DEStruct

## Visão Geral

Este diretório contém a versão refatorada do sistema IDS_DEStruct, seguindo princípios de arquitetura limpa, SOLID e separação em camadas.

## Estrutura de Diretórios

```
refactored/
├── domain/              # Camada de Domínio (Entidades)
│   ├── Term.h/cpp              # Representação de termos
│   ├── Chromosome.h/cpp        # Cromossomo do algoritmo genético
│   ├── SimulationData.h/cpp    # Dados de simulação
│   └── Configuration.h/cpp     # Configuração do sistema
│
├── interfaces/          # Interfaces (Inversão de Dependência)
│   ├── IDataRepository.h       # Contrato para repositório de dados
│   ├── IConfigRepository.h     # Contrato para repositório de config
│   └── IAlgorithmService.h     # Contrato para serviço de algoritmo
│
├── services/            # Camada de Serviços (Lógica de Negócio)
│   ├── MathematicalService.h/cpp           # Operações matemáticas
│   └── DifferentialEvolutionService.h/cpp  # Algoritmo DE
│
├── infrastructure/      # Camada de Infraestrutura (Persistência)
│   ├── FileDataRepository.h/cpp    # Repositório de arquivos texto
│   └── XMLConfigRepository.h/cpp   # Repositório XML
│
├── presentation/        # Camada de Apresentação (MVP)
│   ├── MainWindowPresenter.h/cpp   # Presenter principal
│   └── PlotManager.h/cpp           # Gerenciador de gráficos
│
├── utils/               # Utilitários
│   ├── MathVector.h            # Vetor matemático template
│   ├── MathMatrix.h            # Matriz matemática template
│   └── RandomGenerator.h/cpp   # Gerador de números aleatórios
│
├── ui/                  # Arquivos de interface (Qt Designer)
├── main.cpp             # Ponto de entrada com DI
└── IDS_DEStruct_Refactored.pro  # Arquivo de projeto Qt
```

## Princípios Aplicados

### 1. Responsabilidade Única (SRP)
Cada classe tem uma única responsabilidade bem definida:
- `MathematicalService`: Apenas operações matemáticas
- `FileDataRepository`: Apenas leitura/escrita de arquivos
- `MainWindowPresenter`: Apenas lógica de apresentação

### 2. Aberto/Fechado (OCP)
- Novas implementações podem ser adicionadas sem modificar código existente
- Exemplo: Criar `BinaryDataRepository` sem modificar `IDataRepository`

### 3. Substituição de Liskov (LSP)
- Todas as implementações de interfaces são intercambiáveis
- `FileDataRepository` e `DatabaseRepository` podem substituir `IDataRepository`

### 4. Segregação de Interface (ISP)
- Interfaces específicas para cada responsabilidade
- `IDataRepository` separado de `IConfigRepository`

### 5. Inversão de Dependência (DIP)
- Classes dependem de abstrações (interfaces), não de implementações concretas
- Injeção de dependências via construtor

## Padrões de Projeto Utilizados

### MVP (Model-View-Presenter)
- **Model**: Camadas Domain + Services
- **View**: Qt Widgets (QMainWindow, etc.)
- **Presenter**: `MainWindowPresenter` coordena entre View e Model

### Repository Pattern
- `IDataRepository` e `IConfigRepository` abstraem persistência
- Facilita testes e troca de tecnologia de armazenamento

### Service Layer
- Serviços encapsulam lógica de negócio complexa
- Podem ser reutilizados por diferentes presenters

### Dependency Injection
- Dependências injetadas via construtor
- Facilita testes unitários e manutenção

## Benefícios da Refatoração

### Escalabilidade
✓ Fácil adicionar novos algoritmos (implementar `IAlgorithmService`)
✓ Fácil adicionar novos formatos de dados (implementar `IDataRepository`)
✓ Fácil adicionar novas visualizações (criar novo Presenter)

### Testabilidade
✓ Cada componente pode ser testado isoladamente
✓ Mocks podem ser facilmente criados para interfaces
✓ Testes unitários não dependem de UI ou I/O

### Manutenibilidade
✓ Mudanças localizadas em uma camada
✓ Código mais legível e autodocumentado
✓ Responsabilidades claras

### Desacoplamento
✓ Camadas independentes
✓ UI pode ser trocada sem afetar lógica
✓ Lógica de negócio independente de framework

## Como Compilar

### Usando qmake (recomendado)

```bash
cd refactored
mkdir build
cd build
qmake ../IDS_DEStruct_Refactored.pro
make  # ou nmake no Windows com MSVC
```

### Usando Qt Creator

1. Abra `IDS_DEStruct_Refactored.pro` no Qt Creator
2. Configure o projeto
3. Build → Build Project
4. Run → Run

## Exemplo de Uso

```cpp
// Cria implementações concretas
auto mathService = new MathematicalService();
auto algorithmService = new DifferentialEvolutionService(mathService);
auto dataRepo = new FileDataRepository();
auto configRepo = new XMLConfigRepository();

// Injeta dependências no presenter
auto presenter = new MainWindowPresenter(
    algorithmService,
    dataRepo,
    configRepo
);

// Conecta com a view
connect(presenter, &MainWindowPresenter::progressUpdated,
        view, &MainWindow::onProgressUpdated);

// Usa o presenter para controlar o algoritmo
presenter->loadDataFile("dados.txt");
presenter->startAlgorithm();
```

## Extensibilidade

### Adicionar Novo Algoritmo

```cpp
class ParticleSwarmService : public IAlgorithmService {
    // Implementação do PSO
};

// No main.cpp, troca facilmente:
auto algorithmService = new ParticleSwarmService(mathService);
```

### Adicionar Nova Fonte de Dados

```cpp
class DatabaseRepository : public IDataRepository {
    // Lê dados de banco SQL
};

// Injeta nova implementação:
auto dataRepo = new DatabaseRepository();
auto presenter = new MainWindowPresenter(algo, dataRepo, configRepo);
```

### Adicionar Nova Visualização

```cpp
class CustomPlotManager : public PlotManager {
    // Visualização customizada
};
```

## Comparação com Código Original

| Aspecto | Original | Refatorado |
|---------|----------|------------|
| **Arquivos** | ~15 arquivos monolíticos | ~30 arquivos organizados |
| **Acoplamento** | Alto (classes estáticas globais) | Baixo (injeção de dependências) |
| **Testabilidade** | Difícil (dependências hard-coded) | Fácil (mocks de interfaces) |
| **Manutenção** | Mudanças propagam em cascata | Mudanças localizadas |
| **Complexidade** | Classe de 1000+ linhas | Classes de 100-300 linhas |
| **Reutilização** | Difícil (código acoplado) | Fácil (componentes independentes) |

## Roadmap Futuro

- [ ] Implementar testes unitários para cada camada
- [ ] Adicionar logging estruturado
- [ ] Implementar padrão Command para undo/redo
- [ ] Adicionar suporte a plugins
- [ ] Implementar persistência em banco de dados
- [ ] Adicionar API REST para controle remoto
- [ ] Paralelização avançada com ThreadPool
- [ ] Sistema de eventos para desacoplamento adicional

## Licença

Mesma licença do projeto original.

## Contato

LASEC - FEELT - UFU
