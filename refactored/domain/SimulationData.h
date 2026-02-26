#ifndef REFACTORED_SIMULATION_DATA_H
#define REFACTORED_SIMULATION_DATA_H

#include <QString>
#include <QList>
#include "../utils/MathMatrix.h"

namespace Domain {

/**
 * @brief Classe que encapsula os dados de simulação
 * 
 * Responsabilidade: Armazenar e gerenciar dados de entrada/saída
 * para o processo de identificação estrutural.
 */
class SimulationData
{
public:
    SimulationData();
    ~SimulationData() = default;

    // Getters
    qint32 getOutputCount() const { return m_outputCount; }
    const QList<QString>& getNames() const { return m_names; }
    const QList<qreal>& getMaxValues() const { return m_maxValues; }
    const QList<qreal>& getMinValues() const { return m_minValues; }
    const Utils::MathMatrix<qreal>& getValues() const { return m_values; }

    // Setters
    void setOutputCount(qint32 count) { m_outputCount = count; }
    void setNames(const QList<QString>& names) { m_names = names; }
    void setMaxValues(const QList<qreal>& values) { m_maxValues = values; }
    void setMinValues(const QList<qreal>& values) { m_minValues = values; }
    void setValues(const Utils::MathMatrix<qreal>& values) { m_values = values; }

    // Métodos de manipulação
    void addVariable(const QString& name, qreal minVal, qreal maxVal);
    void clear();
    bool isValid() const;
    
    // Normalização
    QList<qreal> normalizeRow(qint32 rowIndex) const;
    void denormalizeRow(qint32 rowIndex, QList<qreal>& data) const;

private:
    qint32 m_outputCount;                // Quantidade de saídas
    QList<QString> m_names;              // Nomes das variáveis
    QList<qreal> m_maxValues;            // Valores máximos
    QList<qreal> m_minValues;            // Valores mínimos
    Utils::MathMatrix<qreal> m_values;   // Matriz de valores (linhas=variáveis, colunas=atrasos)
};

} // namespace Domain

#endif // REFACTORED_SIMULATION_DATA_H
