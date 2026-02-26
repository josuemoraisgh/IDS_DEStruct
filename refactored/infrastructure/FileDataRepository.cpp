#include "FileDataRepository.h"
#include <QFileInfo>
#include <QDebug>

namespace Infrastructure {

FileDataRepository::FileDataRepository()
    : m_delimiter("\t")
{
}

bool FileDataRepository::loadData(const QString& fileName, Domain::SimulationData& data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível abrir o arquivo:" << fileName;
        return false;
    }

    QTextStream in(&file);
    data.clear();

    // Lê cabeçalho
    if (in.atEnd()) {
        qWarning() << "Arquivo vazio";
        file.close();
        return false;
    }

    QString headerLine = in.readLine();
    QList<QString> names;
    if (!parseHeader(headerLine, names)) {
        qWarning() << "Erro ao ler cabeçalho";
        file.close();
        return false;
    }

    data.setNames(names);

    // Lê dados
    Utils::MathMatrix<qreal> values;
    QList<QList<qreal>> allValues;
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QList<qreal> rowValues;
        if (!parseDataLine(line, rowValues)) {
            qWarning() << "Erro ao ler linha de dados";
            continue;
        }

        if (rowValues.size() != names.size()) {
            qWarning() << "Número de colunas não corresponde ao cabeçalho";
            continue;
        }

        allValues.append(rowValues);
    }

    file.close();

    // Converte para matriz (transpõe: colunas do arquivo viram linhas da matriz)
    if (!allValues.isEmpty()) {
        qint32 rows = names.size();
        qint32 cols = allValues.size();
        values.resize(rows, cols);

        for (qint32 col = 0; col < cols; ++col) {
            for (qint32 row = 0; row < rows; ++row) {
                values.at(row, col) = allValues[col][row];
            }
        }

        data.setValues(values);

        // Calcula min/max para cada variável
        QList<qreal> minValues, maxValues;
        for (qint32 row = 0; row < rows; ++row) {
            auto rowVec = values.getRow(row);
            minValues.append(rowVec.min());
            maxValues.append(rowVec.max());
        }

        data.setMinValues(minValues);
        data.setMaxValues(maxValues);
    }

    return true;
}

bool FileDataRepository::saveData(const QString& fileName, const Domain::SimulationData& data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Não foi possível criar o arquivo:" << fileName;
        return false;
    }

    QTextStream out(&file);

    // Escreve cabeçalho
    out << formatHeader(data.getNames()) << "\n";

    // Escreve dados (transpõe: linhas da matriz viram colunas do arquivo)
    const auto& values = data.getValues();
    qint32 numCols = values.numCols();
    qint32 numRows = values.numRows();

    for (qint32 col = 0; col < numCols; ++col) {
        QList<qreal> rowData;
        for (qint32 row = 0; row < numRows; ++row) {
            rowData.append(values.at(row, col));
        }
        out << formatDataLine(rowData) << "\n";
    }

    file.close();
    return true;
}

bool FileDataRepository::fileExists(const QString& fileName) const
{
    return QFileInfo::exists(fileName);
}

qint64 FileDataRepository::getFileSize(const QString& fileName) const
{
    QFileInfo info(fileName);
    return info.size();
}

bool FileDataRepository::parseHeader(const QString& header, QList<QString>& names)
{
    names.clear();
    QStringList parts = header.split(m_delimiter, Qt::SkipEmptyParts);
    
    for (const QString& part : parts) {
        names.append(part.trimmed());
    }

    return !names.isEmpty();
}

bool FileDataRepository::parseDataLine(const QString& line, QList<qreal>& values)
{
    values.clear();
    QStringList parts = line.split(m_delimiter, Qt::SkipEmptyParts);

    bool ok;
    for (const QString& part : parts) {
        qreal value = part.trimmed().toDouble(&ok);
        if (!ok) {
            return false;
        }
        values.append(value);
    }

    return !values.isEmpty();
}

QString FileDataRepository::formatHeader(const QList<QString>& names)
{
    return names.join(m_delimiter);
}

QString FileDataRepository::formatDataLine(const QList<qreal>& values)
{
    QStringList parts;
    for (qreal value : values) {
        parts.append(QString::number(value, 'g', 10));
    }
    return parts.join(m_delimiter);
}

} // namespace Infrastructure
