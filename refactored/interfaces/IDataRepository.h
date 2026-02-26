#ifndef REFACTORED_IDATA_REPOSITORY_H
#define REFACTORED_IDATA_REPOSITORY_H

#include <QString>
#include <QList>
#include "../domain/SimulationData.h"

namespace Interfaces {

/**
 * @brief Interface para repositório de dados
 * 
 * Define contrato para carregamento e salvamento de dados,
 * seguindo o Princípio de Inversão de Dependência
 */
class IDataRepository
{
public:
    virtual ~IDataRepository() = default;

    /**
     * @brief Carrega dados de um arquivo
     * @param fileName Caminho do arquivo
     * @param data Objeto onde os dados serão armazenados
     * @return true se sucesso, false caso contrário
     */
    virtual bool loadData(const QString& fileName, Domain::SimulationData& data) = 0;

    /**
     * @brief Salva dados em um arquivo
     * @param fileName Caminho do arquivo
     * @param data Dados a serem salvos
     * @return true se sucesso, false caso contrário
     */
    virtual bool saveData(const QString& fileName, const Domain::SimulationData& data) = 0;

    /**
     * @brief Verifica se um arquivo existe
     * @param fileName Caminho do arquivo
     * @return true se existe, false caso contrário
     */
    virtual bool fileExists(const QString& fileName) const = 0;

    /**
     * @brief Obtém o tamanho do arquivo em bytes
     * @param fileName Caminho do arquivo
     * @return Tamanho do arquivo
     */
    virtual qint64 getFileSize(const QString& fileName) const = 0;
};

} // namespace Interfaces

#endif // REFACTORED_IDATA_REPOSITORY_H
