#ifndef REFACTORED_MAIN_WINDOW_PRESENTER_H
#define REFACTORED_MAIN_WINDOW_PRESENTER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTimer>
#include "../domain/Configuration.h"
#include "../domain/Chromosome.h"
#include "../interfaces/IAlgorithmService.h"
#include "../interfaces/IDataRepository.h"
#include "../interfaces/IConfigRepository.h"

namespace Presentation {

/**
 * @brief Presenter para a janela principal
 * 
 * Responsabilidades:
 * - Coordenar interações entre a view e os serviços
 * - Manter lógica de apresentação separada da UI
 * - Gerenciar estado da aplicação
 * 
 * Seguindo o padrão MVP (Model-View-Presenter)
 */
class MainWindowPresenter : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowPresenter(Interfaces::IAlgorithmService* algorithmService,
                                Interfaces::IDataRepository* dataRepository,
                                Interfaces::IConfigRepository* configRepository,
                                QObject* parent = nullptr);
    ~MainWindowPresenter() override;

    // Métodos de controle do algoritmo
    void startAlgorithm();
    void stopAlgorithm();
    void pauseAlgorithm();
    void resumeAlgorithm();
    
    // Métodos de dados
    void loadData(const QString& fileName);
    bool loadDataFile(const QString& fileName);
    bool saveDataFile(const QString& fileName);
    bool loadConfiguration(const QString& fileName);
    bool saveConfiguration(const QString& fileName);
    void showConfigurationDialog();
    
    // Getters
    const Domain::Configuration& getConfiguration() const { return m_configuration; }
    Domain::Configuration& getConfiguration() { return m_configuration; }
    bool isAlgorithmRunning() const;
    Domain::Chromosome getBestChromosome(qint32 outputId) const;
    Interfaces::IAlgorithmService* algorithmService() const { return m_algorithmService; }
    
    // Setters
    void setConfiguration(const Domain::Configuration& config);

signals:
    // Sinais para a view
    void dataLoaded(const QString& fileName);
    void dataLoadFailed(const QString& error);
    void configurationLoaded();
    void configurationLoadFailed(const QString& error);
    
    void algorithmStarted();
    void algorithmStopped();
    void algorithmPaused();
    void algorithmFinished();
    
    void statusUpdated(qint64 iterations, 
                       const QVector<qreal>& errors,
                       const QVector<Domain::Chromosome>& bestChromosomes);
    void progressUpdated(qint64 iterations, 
                        const QVector<qreal>& errors,
                        const QVector<Domain::Chromosome>& bestChromosomes);
    
    void statusMessageChanged(const QString& message);
    void errorOccurred(const QString& error);

private slots:
    // Slots conectados aos serviços
    void onAlgorithmStatusUpdated(qint64 iterations,
                                 const QVector<qreal>& errors,
                                 const QVector<Domain::Chromosome>& bestChromosomes);
    void onAlgorithmFinished();
    void onAlgorithmPaused();
    void onAlgorithmError(const QString& message);

private:
    // Serviços (injetados - Inversão de Dependência)
    Interfaces::IAlgorithmService* m_algorithmService;
    Interfaces::IDataRepository* m_dataRepository;
    Interfaces::IConfigRepository* m_configRepository;
    
    // Estado
    Domain::Configuration m_configuration;
    QString m_currentDataFile;
    QString m_currentConfigFile;
    
    // Métodos auxiliares
    void connectServices();
    void disconnectServices();
    void validateConfiguration();
    void pollProgress();

    // Timer de polling para atualização da UI (backup robusto)
    QTimer* m_progressTimer;
};

} // namespace Presentation

#endif // REFACTORED_MAIN_WINDOW_PRESENTER_H
