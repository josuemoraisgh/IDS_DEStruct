# Guia de Migração - Do Código Original para Refatorado

## Visão Geral

Este guia ajudará a entender como o código original foi refatorado e como migrar funcionalidades existentes.

## Mapeamento de Classes

### Código Original → Código Refatorado

| Original | Refatorado | Camada | Responsabilidade |
|----------|-----------|--------|------------------|
| `compTermo` | `Domain::CompositeTerm` | Domain | Representação de termo |
| `Cromossomo` | `Domain::Chromosome` | Domain | Cromossomo do AG |
| `Config` | `Domain::Configuration` | Domain | Configuração do sistema |
| `PSim` | `Domain::SimulationData` | Domain | Dados de simulação |
| `DEStruct` | `Services::DifferentialEvolutionService` | Services | Algoritmo DE |
| `JMathVar<T>` | `Utils::MathVector<T>` | Utils | Vetor matemático |
| `xmatriz.h` (parte) | `Services::MathematicalService` | Services | Cálculos matemáticos |
| `ICarregar` | `Presentation::MainWindowPresenter` | Presentation | Carregar dados (lógica) |
| `imainwindow` | `Presentation::MainWindowPresenter` | Presentation | Janela principal (lógica) |
| `xmlreaderwriter` | `Infrastructure::XMLConfigRepository` | Infrastructure | Persistência XML |

## Antes e Depois - Exemplos Práticos

### Exemplo 1: Criar e Avaliar Cromossomo

**ANTES (Original):**
```cpp
// Código espalhado em DEStruct.cpp com variáveis globais
Cromossomo cr;
cr.erro = 0.0;
cr.aptidao = 0.0;
// ... código de 100+ linhas misturando criação, avaliação e I/O
DES_CalcERR(cr, metodo);
DES_calAptidao(cr, tamErro);
```

**DEPOIS (Refatorado):**
```cpp
// Criação separada da avaliação, responsabilidades claras
Domain::Chromosome chromosome;
chromosome.setOutputId(0);

// Avaliação delegada ao serviço
Services::DifferentialEvolutionService deService(&mathService);
deService.evaluateChromosome(chromosome);

// Cálculo de fitness separado
mathService.bayesianInformationCriterion(sse, n, k);
```

### Exemplo 2: Carregar Dados

**ANTES (Original):**
```cpp
// Em ICarregar, mistura UI com I/O
void ICarregar::slot_UL_CarDados() {
    QString fileName = QFileDialog::getOpenFileName(...);
    QFile file(fileName);
    file.open(...);
    // ... 200+ linhas de parsing, validação e atualização de UI
}
```

**DEPOIS (Refatorado):**
```cpp
// Separado em camadas com responsabilidades claras

// 1. Presenter (lógica de apresentação)
void MainWindowPresenter::loadDataFile(const QString& fileName) {
    Domain::SimulationData data;
    if (m_dataRepository->loadData(fileName, data)) {
        m_configuration.getAlgorithmData().variables = data;
        emit dataLoaded(fileName);
    }
}

// 2. Repository (I/O)
bool FileDataRepository::loadData(const QString& fileName, 
                                 Domain::SimulationData& data) {
    QFile file(fileName);
    // ... apenas parsing e criação de objetos Domain
    return true;
}

// 3. View (UI) - em outro arquivo
void MainWindow::onLoadDataClicked() {
    QString fileName = QFileDialog::getOpenFileName(...);
    m_presenter->loadDataFile(fileName);
}
```

### Exemplo 3: Executar Algoritmo

**ANTES (Original):**
```cpp
// Em imainwindow.cpp, controle direto e acoplado
void ICalc::slot_MW_IniciarFinalizar() {
    if (!DEStruct::DES_isCarregar) {
        // ... validação misturada com lógica de UI
    }
    for(int i=0; i < QThread::idealThreadCount(); i++) {
        ags->at(i)->start(); // Acesso direto ao thread
    }
    // ... código de 50+ linhas misturando estados
}
```

**DEPOIS (Refatorado):**
```cpp
// 1. Presenter coordena (sem conhecer implementação)
void MainWindowPresenter::startAlgorithm() {
    if (!m_configuration.isValid()) {
        emit errorOccurred("Configuração inválida");
        return;
    }
    m_algorithmService->initialize(m_configuration);
    m_algorithmService->start();
    emit algorithmStarted();
}

// 2. Service gerencia threads internamente
void DifferentialEvolutionService::start() {
    m_running = true;
    QTimer::singleShot(0, this, &DifferentialEvolutionService::runAlgorithm);
}

// 3. View apenas conecta sinais
connect(m_presenter, &MainWindowPresenter::algorithmStarted,
        this, &MainWindow::onAlgorithmStarted);
```

## Padrões de Migração

### Padrão 1: Variáveis Estáticas Globais → Injeção de Dependências

**ANTES:**
```cpp
class DEStruct {
    static Config DES_Adj;  // Global estático
    static QList<QVector<Cromossomo>> DES_crMut;  // Global estático
};

// Acesso direto em qualquer lugar
void ICalc::algumMetodo() {
    DEStruct::DES_Adj.tamPop = 100;  // Acoplamento forte
}
```

**DEPOIS:**
```cpp
// Configuração como objeto independente
Domain::Configuration config;
config.getAlgorithmData().populationSize = 100;

// Injetado onde necessário
DifferentialEvolutionService service(&mathService);
service.initialize(config);  // Passa configuração explicitamente
```

### Padrão 2: Funções Livres → Métodos de Serviço

**ANTES:**
```cpp
// Função livre em destruct.cpp
qreal sign(const qreal &x) {
    if(x>0) return 1;
    else if(x==0) return 0;
    else return -1;
}

// Usada em vários lugares sem contexto
qreal resultado = sign(valor);
```

**DEPOIS:**
```cpp
// Método de serviço com contexto claro
class MathematicalService {
public:
    qreal sign(qreal x) {
        if (x > 0) return 1.0;
        if (x < 0) return -1.0;
        return 0.0;
    }
};

// Uso via serviço
qreal resultado = mathService->sign(valor);
```

### Padrão 3: Classe Monolítica → Múltiplas Classes Especializadas

**ANTES:**
```cpp
class ICalc : public QMainWindow {
    // ~1000 linhas com múltiplas responsabilidades:
    void slot_MW_CarDados();      // I/O
    void slot_MW_Desenha();       // Visualização
    void slot_MW_IniciarFinalizar();  // Controle de algoritmo
    void ini_MW_interface();      // UI
    void ini_MW_qwtPlot1();       // Plotting
    // ... 50+ métodos misturados
};
```

**DEPOIS:**
```cpp
// Separado em classes especializadas

class MainWindowPresenter {  // Lógica de apresentação
    void loadDataFile(const QString& fileName);
    void startAlgorithm();
};

class PlotManager {  // Apenas plotting
    void plotData(QwtPlot* plot, ...);
    void setupPlot(QwtPlot* plot, ...);
};

class MainWindow : public QMainWindow {  // Apenas UI
    void setupUi();
    void connectSignals();
};
```

### Padrão 4: Locks Globais → Encapsulamento

**ANTES:**
```cpp
class DEStruct {
    static QReadWriteLock lock_DES_BufferSR;  // Global
    static QList<QList<QVector<Cromossomo>>> DES_BufferSR;  // Global
};

// Acesso direto com lock manual em todo lugar
DEStruct::lock_DES_BufferSR.lockForRead();
auto data = DEStruct::DES_BufferSR.at(i);
DEStruct::lock_DES_BufferSR.unlock();
```

**DEPOIS:**
```cpp
class DifferentialEvolutionService {
private:
    mutable QMutex m_mutex;  // Encapsulado
    QList<QVector<Chromosome>> m_population;  // Privado
    
public:
    Chromosome getBestChromosome(qint32 outputId) const {
        QMutexLocker locker(&m_mutex);  // RAII
        return m_bestChromosomes[outputId];
    }
};
```

## Checklist de Migração

### Para Migrar uma Funcionalidade:

- [ ] **1. Identificar a responsabilidade principal**
  - É lógica de negócio? → Services
  - É dados puros? → Domain  
  - É I/O? → Infrastructure
  - É lógica de apresentação? → Presentation

- [ ] **2. Extrair dependências**
  - Listar todas as classes/funções usadas
  - Criar interfaces se necessário
  - Injetar via construtor

- [ ] **3. Remover acoplamentos**
  - Substituir variáveis estáticas por membros de instância
  - Remover `#include` de implementações, usar apenas interfaces
  - Usar sinais/slots do Qt para comunicação

- [ ] **4. Criar testes**
  - Criar mocks das dependências
  - Testar classe isoladamente
  - Verificar integração

- [ ] **5. Documentar**
  - Adicionar comentários de responsabilidade
  - Documentar contratos das interfaces
  - Atualizar diagramas

## Exemplo Completo de Migração

### Migrar: Cálculo de Aptidão por BIC

**Passo 1: Código Original**
```cpp
// Em destruct.cpp, misturado com outras funcionalidades
void DEStruct::DES_calAptidao(Cromossomo &cr, const quint32 &tamErro) const {
    const qint32 iMed = DES_Adj.Dados.variaveis.valores.numColunas();
    const qreal tam = ((qreal) iMed)*((qreal) DES_Adj.Dados.qtSaidas);
    const qint32 p = cr.termos.size() + 1;
    cr.aptidao = tam*qLn(cr.erro/tam) + ((qreal)p)*qLn(tam);
}
```

**Passo 2: Extrair para Serviço Matemático**
```cpp
// Em services/MathematicalService.h
class MathematicalService {
public:
    qreal bayesianInformationCriterion(qreal sse, qint32 n, qint32 k);
};

// Em services/MathematicalService.cpp
qreal MathematicalService::bayesianInformationCriterion(qreal sse, 
                                                        qint32 n, 
                                                        qint32 k) {
    if (n <= 0 || sse < 0) return 0.0;
    return n * qLn(sse / n) + k * qLn(n);
}
```

**Passo 3: Usar no Serviço DE**
```cpp
// Em services/DifferentialEvolutionService.cpp
void DifferentialEvolutionService::calculateFitness(Chromosome& chromosome) {
    qreal sse = chromosome.getError();
    qint32 n = 100;  // Número de observações
    qint32 k = chromosome.getTermCount();
    
    qreal fitness = m_mathService->bayesianInformationCriterion(sse, n, k);
    chromosome.setFitness(fitness);
}
```

**Passo 4: Criar Teste**
```cpp
// Em tests/TestMathematicalService.cpp
void TestMathematicalService::testBIC() {
    MathematicalService service;
    
    qreal sse = 10.0;
    qint32 n = 100;
    qint32 k = 5;
    
    qreal bic = service.bayesianInformationCriterion(sse, n, k);
    
    // Valor esperado calculado manualmente
    qreal expected = 100 * qLn(10.0/100) + 5 * qLn(100);
    QCOMPARE(bic, expected);
}
```

## Benefícios Obtidos

### Original:
- ❌ 1 método de 50+ linhas
- ❌ Acoplado com DEStruct
- ❌ Usa variável global DES_Adj
- ❌ Impossível testar isoladamente
- ❌ Mistura cálculo com lógica de threads

### Refatorado:
- ✅ 1 método de 10 linhas
- ✅ Independente, reutilizável
- ✅ Recebe parâmetros explícitos
- ✅ Facilmente testável
- ✅ Responsabilidade única (cálculo BIC)

## Conclusão

A refatoração segue um padrão consistente:

1. **Identificar responsabilidade** → Escolher camada apropriada
2. **Extrair dependências** → Criar interfaces
3. **Injetar dependências** → Via construtor
4. **Remover globais** → Substituir por injeção
5. **Separar concerns** → Uma classe = uma responsabilidade
6. **Testar** → Usar mocks e testes unitários

O resultado é um código mais:
- **Manutenível**: mudanças localizadas
- **Testável**: componentes isolados
- **Escalável**: fácil adicionar features
- **Legível**: responsabilidades claras
