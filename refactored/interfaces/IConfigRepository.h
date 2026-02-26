#ifndef REFACTORED_ICONFIG_REPOSITORY_H
#define REFACTORED_ICONFIG_REPOSITORY_H

#include <QString>
#include "../domain/Configuration.h"

namespace Interfaces {

/**
 * @brief Interface para repositório de configurações
 * 
 * Define contrato para persistência de configurações,
 * seguindo o Princípio de Inversão de Dependência
 */
class IConfigRepository
{
public:
    virtual ~IConfigRepository() = default;

    /**
     * @brief Carrega configuração de um arquivo
     * @param fileName Caminho do arquivo
     * @param config Objeto onde a configuração será armazenada
     * @return true se sucesso, false caso contrário
     */
    virtual bool loadConfiguration(const QString& fileName, Domain::Configuration& config) = 0;

    /**
     * @brief Salva configuração em um arquivo
     * @param fileName Caminho do arquivo
     * @param config Configuração a ser salva
     * @return true se sucesso, false caso contrário
     */
    virtual bool saveConfiguration(const QString& fileName, const Domain::Configuration& config) = 0;

    /**
     * @brief Verifica se uma configuração é válida
     * @param config Configuração a ser verificada
     * @return true se válida, false caso contrário
     */
    virtual bool validateConfiguration(const Domain::Configuration& config) const = 0;
};

} // namespace Interfaces

#endif // REFACTORED_ICONFIG_REPOSITORY_H
