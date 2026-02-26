#include "MainWindowPresenter.h"
#include <QDebug>

namespace Presentation {

MainWindowPresenter::MainWindowPresenter(Interfaces::IAlgorithmService* algorithmService,
                                       Interfaces::IDataRepository* dataRepository,
                                       Interfaces::IConfigRepository* configRepository,
                                       QObject* parent)
    : QObject(parent)
    , m_algorithmService(algorithmService)
    , m_dataRepository(dataRepository)
    , m_configRepository(configRepository)
{
    connectServices();
}

MainWindowPresenter::~MainWindowPresenter()
{
    disconnectServices();
}

void MainWindowPresenter::connectServices()
{
    if (m_algorithmService) {
        connect(m_algorithmService, &Interfaces::IAlgorithmService::statusUpdated,
                this, &MainWindowPresenter::onAlgorithmStatusUpdated);
        connect(m_algorithmService, &Interfaces::IAlgorithmService::finished,
                this, &MainWindowPresenter::onAlgorithmFinished);
        connect(m_algorithmService, &Interfaces::IAlgorithmService::paused,
                this, &MainWindowPresenter::onAlgorithmPaused);
        connect(m_algorithmService, &Interfaces::IAlgorithmService::errorOccurred,
                this, &MainWindowPresenter::onAlgorithmError);
    }
}

void MainWindowPresenter::disconnectServices()
{
    if (m_algorithmService) {
        disconnect(m_algorithmService, nullptr, this, nullptr);
    }
}

void MainWindowPresenter::startAlgorithm()
{
    if (!m_algorithmService) {
        emit errorOccurred("Serviço de algoritmo não disponível");
        return;
    }

    // Verificar se a configuração mínima está presente
    if (!m_configuration.isCreated()) {
        emit errorOccurred("Configuração não foi criada. Por favor, carregue os dados primeiro.");
        return;
    }

    // Inicializa o serviço com a configuração atual
    if (!m_algorithmService->initialize(m_configuration)) {
        emit statusMessageChanged("Aviso: Inicialização parcial do algoritmo");
    }

    // Inicia execução
    m_algorithmService->start();
    emit algorithmStarted();
    emit statusMessageChanged(QString("Algoritmo iniciado - Pop: %1, Ciclos: %2")
        .arg(m_configuration.getAlgorithmData().populationSize)
        .arg(m_configuration.getCycleCount()));
}

void MainWindowPresenter::stopAlgorithm()
{
    if (m_algorithmService && m_algorithmService->isRunning()) {
        m_algorithmService->stop();
        emit algorithmStopped();
        emit statusMessageChanged("Algoritmo parado");
    }
}

void MainWindowPresenter::pauseAlgorithm()
{
    if (m_algorithmService && m_algorithmService->isRunning()) {
        m_algorithmService->pause();
        emit algorithmPaused();
        emit statusMessageChanged("Algoritmo pausado");
    }
}

void MainWindowPresenter::resumeAlgorithm()
{
    if (m_algorithmService) {
        m_algorithmService->resume();
        emit algorithmStarted();
        emit statusMessageChanged("Algoritmo retomado...");
    }
}

bool MainWindowPresenter::loadDataFile(const QString& fileName)
{
    if (!m_dataRepository) {
        emit dataLoadFailed("Repositório de dados não disponível");
        return false;
    }

    Domain::SimulationData data;
    if (!m_dataRepository->loadData(fileName, data)) {
        emit dataLoadFailed("Falha ao carregar dados do arquivo: " + fileName);
        return false;
    }

    // Atualiza configuração com os dados carregados
    m_configuration.getAlgorithmData().variables = data;
    m_currentDataFile = fileName;
    
    emit dataLoaded(fileName);
    emit statusMessageChanged("Dados carregados: " + fileName);
    return true;
}

void MainWindowPresenter::loadData(const QString& fileName)
{
    loadDataFile(fileName);
}

bool MainWindowPresenter::saveDataFile(const QString& fileName)
{
    if (!m_dataRepository) {
        emit errorOccurred("Repositório de dados não disponível");
        return false;
    }

    const auto& data = m_configuration.getAlgorithmData().variables;
    if (!m_dataRepository->saveData(fileName, data)) {
        emit errorOccurred("Falha ao salvar dados no arquivo: " + fileName);
        return false;
    }

    emit statusMessageChanged("Dados salvos: " + fileName);
    return true;
}

bool MainWindowPresenter::loadConfiguration(const QString& fileName)
{
    if (!m_configRepository) {
        emit configurationLoadFailed("Repositório de configuração não disponível");
        return false;
    }

    Domain::Configuration config;
    if (!m_configRepository->loadConfiguration(fileName, config)) {
        emit configurationLoadFailed("Falha ao carregar configuração: " + fileName);
        return false;
    }

    m_configuration = config;
    m_currentConfigFile = fileName;
    
    emit configurationLoaded();
    emit statusMessageChanged("Configuração carregada: " + fileName);
    return true;
}

bool MainWindowPresenter::saveConfiguration(const QString& fileName)
{
    if (!m_configRepository) {
        emit errorOccurred("Repositório de configuração não disponível");
        return false;
    }

    if (!m_configRepository->saveConfiguration(fileName, m_configuration)) {
        emit errorOccurred("Falha ao salvar configuração: " + fileName);
        return false;
    }

    m_currentConfigFile = fileName;
    emit statusMessageChanged("Configuração salva: " + fileName);
    return true;
}

bool MainWindowPresenter::isAlgorithmRunning() const
{
    return m_algorithmService && m_algorithmService->isRunning();
}

Domain::Chromosome MainWindowPresenter::getBestChromosome(qint32 outputId) const
{
    if (m_algorithmService)
        return m_algorithmService->getBestChromosome(outputId);
    return Domain::Chromosome();
}

void MainWindowPresenter::setConfiguration(const Domain::Configuration& config)
{
    m_configuration = config;
    validateConfiguration();
}

void MainWindowPresenter::onAlgorithmStatusUpdated(qint64 iterations,
                                                  const QVector<qreal>& errors,
                                                  const QVector<Domain::Chromosome>& bestChromosomes)
{
    // Atualiza iterações na configuração
    m_configuration.setIterations(iterations);
    
    // Repassa para a view
    emit statusUpdated(iterations, errors, bestChromosomes);
    emit progressUpdated(iterations, errors, bestChromosomes);
    
    // Atualiza mensagem de status
    QString message = QString("Iteração %1 - Melhor BIC: %2")
                         .arg(iterations)
                         .arg(errors.isEmpty() ? 0.0 : errors[0], 0, 'g', 6);
    emit statusMessageChanged(message);
}

void MainWindowPresenter::onAlgorithmFinished()
{
    emit algorithmFinished();
    emit algorithmStopped();
    emit statusMessageChanged("Algoritmo finalizado");
    
    qDebug() << "Algoritmo finalizado após" << m_configuration.getIterations() << "iterações";
}

void MainWindowPresenter::showConfigurationDialog()
{
    // Aqui você pode abrir um diálogo de configuração
    // Por enquanto, apenas emite uma mensagem
    emit statusMessageChanged("Diálogo de configuração será exibido");
    qDebug() << "Abrindo diálogo de configuração...";
}

void MainWindowPresenter::onAlgorithmPaused()
{
    emit algorithmPaused();
    emit statusMessageChanged("Algoritmo pausado");
}

void MainWindowPresenter::onAlgorithmError(const QString& message)
{
    emit errorOccurred("Erro no algoritmo: " + message);
    emit algorithmStopped();
}

void MainWindowPresenter::validateConfiguration()
{
    if (!m_configuration.isValid()) {
        emit errorOccurred("Configuração inválida");
    }
}

} // namespace Presentation
