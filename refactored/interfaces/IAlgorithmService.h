#ifndef REFACTORED_IALGORITHM_SERVICE_H
#define REFACTORED_IALGORITHM_SERVICE_H

#include <QObject>
#include "../domain/Configuration.h"
#include "../domain/Chromosome.h"

namespace Interfaces {

/**
 * @brief Interface para serviço de algoritmo de otimização
 * 
 * Define contrato para execução do algoritmo de evolução diferencial,
 * seguindo o Princípio de Inversão de Dependência
 */
class IAlgorithmService : public QObject
{
    Q_OBJECT

public:
    explicit IAlgorithmService(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IAlgorithmService() = default;

    /**
     * @brief Inicializa o algoritmo com uma configuração
     * @param config Configuração do algoritmo
     * @return true se sucesso, false caso contrário
     */
    virtual bool initialize(const Domain::Configuration& config) = 0;

    /**
     * @brief Inicia a execução do algoritmo
     */
    virtual void start() = 0;

    /**
     * @brief Para a execução do algoritmo
     */
    virtual void stop() = 0;

    /**
     * @brief Pausa a execução do algoritmo
     */
    virtual void pause() = 0;

    /**
     * @brief Continua a execução após pausa
     */
    virtual void resume() = 0;

    /**
     * @brief Retorna se o algoritmo está em execução
     */
    virtual bool isRunning() const = 0;

    /**
     * @brief Obtém o melhor cromossomo encontrado para uma saída
     * @param outputId ID da saída
     * @return Melhor cromossomo
     */
    virtual Domain::Chromosome getBestChromosome(qint32 outputId) const = 0;

signals:
    /**
     * @brief Sinal emitido quando o status do algoritmo é atualizado
     * @param iterations Número de iterações
     * @param errors Erros por saída
     * @param bestChromosomes Melhores cromossomos por saída
     */
    void statusUpdated(qint64 iterations, 
                      const QVector<qreal>& errors,
                      const QVector<Domain::Chromosome>& bestChromosomes);

    /**
     * @brief Sinal emitido quando o algoritmo termina
     */
    void finished();

    /**
     * @brief Sinal emitido quando o algoritmo é pausado
     */
    void paused();

    /**
     * @brief Sinal emitido quando ocorre um erro
     * @param message Mensagem de erro
     */
    void errorOccurred(const QString& message);
};

} // namespace Interfaces

#endif // REFACTORED_IALGORITHM_SERVICE_H
