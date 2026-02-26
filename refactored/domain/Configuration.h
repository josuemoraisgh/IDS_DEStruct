#ifndef REFACTORED_CONFIG_H
#define REFACTORED_CONFIG_H

#include <QVector>
#include <QList>
#include <QString>
#include <QDateTime>
#include "Chromosome.h"
#include "SimulationData.h"

namespace Domain {

/**
 * @brief Informações sobre os dados do algoritmo genético
 */
struct AlgorithmInfo
{
    SimulationData variables;    // Estrutura de dados das variáveis
    qint32 elitismIndex;         // Índice do elitismo
    qint32 previousVarCount;     // Quantidade de variáveis anteriores
    qint32 populationSize;       // Tamanho da população
    qreal initialTime;           // Tempo inicial em segundos
    qreal finalTime;             // Tempo final em segundos
    volatile qint16 isElitism;   // 0: não fazer nada; 1: aproveitar elitismo; 2: começar do zero
    
    AlgorithmInfo() 
        : elitismIndex(0)
        , previousVarCount(0)
        , populationSize(0)
        , initialTime(0.0)
        , finalTime(0.0)
        , isElitism(0)
    {}
};

/**
 * @brief Classe de configuração principal do sistema
 * 
 * Responsabilidade: Centralizar todas as configurações do algoritmo
 * e estado da aplicação de forma organizada.
 */
class Configuration
{
public:
    Configuration();
    ~Configuration() = default;

    // Getters - Estado da aplicação
    bool isStartEnabled() const { return m_isStartEnabled; }
    bool isAutoSave() const { return m_autoSave; }
    bool isRational() const { return m_isRational; }
    bool isCreated() const { return m_isCreated; }
    
    // Getters - Controle de operação
    quint8 getStopContinueState() const { return m_stopContinueState; }
    volatile qint16 getOperationMode() const { return m_operationMode; }
    quint8 getExponentType() const { return m_exponentType; }
    
    // Getters - Parâmetros do algoritmo
    quint32 getSavedChromosomeCount() const { return m_savedChromosomeCount; }
    quint32 getSaveLoadedData() const { return m_saveLoadedData; }
    quint32 getCycleCount() const { return m_cycleCount; }
    qreal getSSE() const { return m_sse; }
    qreal getJNRR() const { return m_jnrr; }
    
    // Getters - Dados do algoritmo
    const AlgorithmInfo& getAlgorithmData() const { return m_algorithmData; }
    AlgorithmInfo& getAlgorithmData() { return m_algorithmData; }
    
    // Getters - População e elite
    const QList<QVector<Chromosome>>& getPopulation() const { return m_population; }
    QList<QVector<Chromosome>>& getPopulation() { return m_population; }
    
    // Getters - Temporização e iterações
    volatile qint64 getIterations() const { return m_iterations; }
    volatile qint64 getPreviousIterations() const { return m_previousIterations; }
    const QDateTime& getDateTime() const { return m_dateTime; }
    
    // Getters - Arquivos
    const QString& getConfigFileName() const { return m_configFileName; }
    
    // Getters - Decimação
    const QVector<qint32>& getDecimation() const { return m_decimation; }
    const QVector<qint32>& getDecimationSize() const { return m_decimationSize; }
    
    // Getters - Melhor aptidão anterior
    const QVector<qreal>& getBestPreviousFitness() const { return m_bestPreviousFitness; }

    // Setters - Estado da aplicação
    void setStartEnabled(bool enabled) { m_isStartEnabled = enabled; }
    void setAutoSave(bool enable) { m_autoSave = enable; }
    void setRational(bool rational) { m_isRational = rational; }
    void setCreated(bool created) { m_isCreated = created; }
    
    // Setters - Controle de operação
    void setStopContinueState(quint8 state) { m_stopContinueState = state; }
    void setOperationMode(qint16 mode) { m_operationMode = mode; }
    void setExponentType(quint8 type) { m_exponentType = type; }
    
    // Setters - Parâmetros do algoritmo
    void setSavedChromosomeCount(quint32 count) { m_savedChromosomeCount = count; }
    void setSaveLoadedData(quint32 value) { m_saveLoadedData = value; }
    void setCycleCount(quint32 count) { m_cycleCount = count; }
    void setSSE(qreal value) { m_sse = value; }
    void setJNRR(qreal value) { m_jnrr = value; }
    
    // Setters - População
    void setPopulation(const QList<QVector<Chromosome>>& pop) { m_population = pop; }
    
    // Setters - Iterações
    void setIterations(qint64 iter) { m_iterations = iter; }
    void setPreviousIterations(qint64 iter) { m_previousIterations = iter; }
    void incrementIterations() { ++m_iterations; }
    
    // Setters - Arquivos
    void setConfigFileName(const QString& name) { m_configFileName = name; }
    
    // Setters - Decimação
    void setDecimation(const QVector<qint32>& dec) { m_decimation = dec; }
    void setDecimationSize(const QVector<qint32>& size) { m_decimationSize = size; }
    
    // Setters - Melhor aptidão
    void setBestPreviousFitness(const QVector<qreal>& fitness) { m_bestPreviousFitness = fitness; }

    // Métodos de utilidade
    void reset();
    bool isValid() const;

private:
    // Estado da aplicação
    bool m_isStartEnabled;
    bool m_autoSave;
    bool m_isRational;
    bool m_isCreated;
    
    // Controle de operação
    quint8 m_stopContinueState;      // 0: Desabilitado; 1: Parar; 2: Continuar
    volatile qint16 m_operationMode; // Modo de operação das threads
    quint8 m_exponentType;           // 0: Real; 1: Inteiro; 2: Natural
    
    // Parâmetros do algoritmo
    quint32 m_savedChromosomeCount;
    quint32 m_saveLoadedData;
    quint32 m_cycleCount;
    qreal m_sse;                     // Sum of Squared Errors
    qreal m_jnrr;                    // JNRR parameter
    
    // Dados do algoritmo
    AlgorithmInfo m_algorithmData;
    
    // População e resultados
    QList<QVector<Chromosome>> m_population;           // Saídas -> Cromossomo
    QVector<qreal> m_bestPreviousFitness;              // Melhor aptidão no último reset de cada saída
    
    // Temporização
    QDateTime m_dateTime;
    volatile qint64 m_iterations;
    volatile qint64 m_previousIterations;
    
    // Arquivos
    QString m_configFileName;
    
    // Decimação
    QVector<qint32> m_decimation;
    QVector<qint32> m_decimationSize;
};

} // namespace Domain

#endif // REFACTORED_CONFIG_H
