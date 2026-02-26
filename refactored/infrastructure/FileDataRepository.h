#ifndef REFACTORED_FILE_DATA_REPOSITORY_H
#define REFACTORED_FILE_DATA_REPOSITORY_H

#include "../interfaces/IDataRepository.h"
#include <QFile>
#include <QTextStream>

namespace Infrastructure {

/**
 * @brief Implementação de repositório de dados baseado em arquivos
 * 
 * Responsabilidade: Ler e escrever dados de simulação em arquivos de texto
 */
class FileDataRepository : public Interfaces::IDataRepository
{
public:
    FileDataRepository();
    ~FileDataRepository() override = default;

    // Implementação da interface IDataRepository
    bool loadData(const QString& fileName, Domain::SimulationData& data) override;
    bool saveData(const QString& fileName, const Domain::SimulationData& data) override;
    bool fileExists(const QString& fileName) const override;
    qint64 getFileSize(const QString& fileName) const override;

    // Configurações específicas
    void setDelimiter(const QString& delimiter) { m_delimiter = delimiter; }
    QString getDelimiter() const { return m_delimiter; }

private:
    QString m_delimiter;
    
    // Métodos auxiliares
    bool parseHeader(const QString& header, QList<QString>& names);
    bool parseDataLine(const QString& line, QList<qreal>& values);
    QString formatHeader(const QList<QString>& names);
    QString formatDataLine(const QList<qreal>& values);
};

} // namespace Infrastructure

#endif // REFACTORED_FILE_DATA_REPOSITORY_H
