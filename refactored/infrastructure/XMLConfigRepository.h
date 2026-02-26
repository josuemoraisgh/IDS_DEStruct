#ifndef REFACTORED_XML_CONFIG_REPOSITORY_H
#define REFACTORED_XML_CONFIG_REPOSITORY_H

#include "../interfaces/IConfigRepository.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Infrastructure {

/**
 * @brief Implementação de repositório de configurações baseado em XML
 * 
 * Responsabilidade: Ler e escrever configurações em arquivos XML
 */
class XMLConfigRepository : public Interfaces::IConfigRepository
{
public:
    XMLConfigRepository();
    ~XMLConfigRepository() override = default;

    // Implementação da interface IConfigRepository
    bool loadConfiguration(const QString& fileName, Domain::Configuration& config) override;
    bool saveConfiguration(const QString& fileName, const Domain::Configuration& config) override;
    bool validateConfiguration(const Domain::Configuration& config) const override;

private:
    // Leitura XML
    bool readConfigElement(QXmlStreamReader& xml, Domain::Configuration& config);
    bool readAlgorithmInfo(QXmlStreamReader& xml, Domain::AlgorithmInfo& info);
    bool readSimulationData(QXmlStreamReader& xml, Domain::SimulationData& data);
    
    // Escrita XML
    void writeConfigElement(QXmlStreamWriter& xml, const Domain::Configuration& config);
    void writeAlgorithmInfo(QXmlStreamWriter& xml, const Domain::AlgorithmInfo& info);
    void writeSimulationData(QXmlStreamWriter& xml, const Domain::SimulationData& data);
    
    // Utilidades
    QString readNextText(QXmlStreamReader& xml);
    qreal readNextReal(QXmlStreamReader& xml);
    qint32 readNextInt(QXmlStreamReader& xml);
};

} // namespace Infrastructure

#endif // REFACTORED_XML_CONFIG_REPOSITORY_H
