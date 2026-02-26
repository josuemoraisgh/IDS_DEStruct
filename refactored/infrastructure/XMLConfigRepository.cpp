#include "XMLConfigRepository.h"
#include <QFile>
#include <QDebug>

namespace Infrastructure {

XMLConfigRepository::XMLConfigRepository()
{
}

bool XMLConfigRepository::loadConfiguration(const QString& fileName, Domain::Configuration& config)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível abrir arquivo de configuração:" << fileName;
        return false;
    }

    QXmlStreamReader xml(&file);
    config.reset();

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "Configuration") {
                if (!readConfigElement(xml, config)) {
                    file.close();
                    return false;
                }
            }
        }
    }

    file.close();

    if (xml.hasError()) {
        qWarning() << "Erro ao ler XML:" << xml.errorString();
        return false;
    }

    return validateConfiguration(config);
}

bool XMLConfigRepository::saveConfiguration(const QString& fileName, 
                                           const Domain::Configuration& config)
{
    if (!validateConfiguration(config)) {
        qWarning() << "Configuração inválida";
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível criar arquivo de configuração:" << fileName;
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    
    xml.writeStartElement("Configuration");
    writeConfigElement(xml, config);
    xml.writeEndElement(); // Configuration

    xml.writeEndDocument();
    file.close();

    return true;
}

bool XMLConfigRepository::validateConfiguration(const Domain::Configuration& config) const
{
    return config.isValid();
}

bool XMLConfigRepository::readConfigElement(QXmlStreamReader& xml, Domain::Configuration& config)
{
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Configuration")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "StartEnabled") {
                config.setStartEnabled(readNextInt(xml) != 0);
            }
            else if (name == "AutoSave") {
                config.setAutoSave(readNextInt(xml) != 0);
            }
            else if (name == "Rational") {
                config.setRational(readNextInt(xml) != 0);
            }
            else if (name == "OperationMode") {
                config.setOperationMode(readNextInt(xml));
            }
            else if (name == "ExponentType") {
                config.setExponentType(readNextInt(xml));
            }
            else if (name == "SavedChromosomeCount") {
                config.setSavedChromosomeCount(readNextInt(xml));
            }
            else if (name == "CycleCount") {
                config.setCycleCount(readNextInt(xml));
            }
            else if (name == "SSE") {
                config.setSSE(readNextReal(xml));
            }
            else if (name == "JNRR") {
                config.setJNRR(readNextReal(xml));
            }
            else if (name == "AlgorithmInfo") {
                Domain::AlgorithmInfo info;
                if (readAlgorithmInfo(xml, info)) {
                    config.getAlgorithmData() = info;
                }
            }
        }
        xml.readNext();
    }

    return true;
}

bool XMLConfigRepository::readAlgorithmInfo(QXmlStreamReader& xml, Domain::AlgorithmInfo& info)
{
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "AlgorithmInfo")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "PopulationSize") {
                info.populationSize = readNextInt(xml);
            }
            else if (name == "InitialTime") {
                info.initialTime = readNextReal(xml);
            }
            else if (name == "FinalTime") {
                info.finalTime = readNextReal(xml);
            }
            else if (name == "ElitismIndex") {
                info.elitismIndex = readNextInt(xml);
            }
            else if (name == "IsElitism") {
                info.isElitism = readNextInt(xml);
            }
            else if (name == "SimulationData") {
                readSimulationData(xml, info.variables);
            }
        }
        xml.readNext();
    }

    return true;
}

bool XMLConfigRepository::readSimulationData(QXmlStreamReader& xml, Domain::SimulationData& data)
{
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "SimulationData")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            if (name == "OutputCount") {
                data.setOutputCount(readNextInt(xml));
            }
            // Adicionar mais campos conforme necessário
        }
        xml.readNext();
    }

    return true;
}

void XMLConfigRepository::writeConfigElement(QXmlStreamWriter& xml, 
                                            const Domain::Configuration& config)
{
    xml.writeTextElement("StartEnabled", QString::number(config.isStartEnabled() ? 1 : 0));
    xml.writeTextElement("AutoSave", QString::number(config.isAutoSave() ? 1 : 0));
    xml.writeTextElement("Rational", QString::number(config.isRational() ? 1 : 0));
    xml.writeTextElement("OperationMode", QString::number(config.getOperationMode()));
    xml.writeTextElement("ExponentType", QString::number(config.getExponentType()));
    xml.writeTextElement("SavedChromosomeCount", QString::number(config.getSavedChromosomeCount()));
    xml.writeTextElement("CycleCount", QString::number(config.getCycleCount()));
    xml.writeTextElement("SSE", QString::number(config.getSSE(), 'g', 10));
    xml.writeTextElement("JNRR", QString::number(config.getJNRR(), 'g', 10));

    xml.writeStartElement("AlgorithmInfo");
    writeAlgorithmInfo(xml, config.getAlgorithmData());
    xml.writeEndElement();
}

void XMLConfigRepository::writeAlgorithmInfo(QXmlStreamWriter& xml, 
                                            const Domain::AlgorithmInfo& info)
{
    xml.writeTextElement("PopulationSize", QString::number(info.populationSize));
    xml.writeTextElement("InitialTime", QString::number(info.initialTime, 'g', 10));
    xml.writeTextElement("FinalTime", QString::number(info.finalTime, 'g', 10));
    xml.writeTextElement("ElitismIndex", QString::number(info.elitismIndex));
    xml.writeTextElement("IsElitism", QString::number(info.isElitism));

    xml.writeStartElement("SimulationData");
    writeSimulationData(xml, info.variables);
    xml.writeEndElement();
}

void XMLConfigRepository::writeSimulationData(QXmlStreamWriter& xml, 
                                             const Domain::SimulationData& data)
{
    xml.writeTextElement("OutputCount", QString::number(data.getOutputCount()));
    // Adicionar mais campos conforme necessário
}

QString XMLConfigRepository::readNextText(QXmlStreamReader& xml)
{
    xml.readNext();
    return xml.text().toString();
}

qreal XMLConfigRepository::readNextReal(QXmlStreamReader& xml)
{
    return readNextText(xml).toDouble();
}

qint32 XMLConfigRepository::readNextInt(QXmlStreamReader& xml)
{
    return readNextText(xml).toInt();
}

} // namespace Infrastructure
