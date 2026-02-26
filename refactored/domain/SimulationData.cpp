#include "SimulationData.h"

namespace Domain {

SimulationData::SimulationData()
    : m_outputCount(0)
{
}

void SimulationData::addVariable(const QString& name, qreal minVal, qreal maxVal)
{
    m_names.append(name);
    m_minValues.append(minVal);
    m_maxValues.append(maxVal);
}

void SimulationData::clear()
{
    m_outputCount = 0;
    m_names.clear();
    m_maxValues.clear();
    m_minValues.clear();
    m_values.clear();
}

bool SimulationData::isValid() const
{
    return !m_names.isEmpty() && 
           (m_names.size() == m_maxValues.size()) &&
           (m_names.size() == m_minValues.size()) &&
           (m_outputCount > 0);
}

QList<qreal> SimulationData::normalizeRow(qint32 rowIndex) const
{
    QList<qreal> normalized;
    if (rowIndex < 0 || rowIndex >= m_values.numRows()) {
        return normalized;
    }

    qreal minVal = m_minValues.value(rowIndex, 0.0);
    qreal maxVal = m_maxValues.value(rowIndex, 1.0);
    qreal range = maxVal - minVal;

    if (qAbs(range) < 1e-10) {
        range = 1.0;
    }

    for (qint32 col = 0; col < m_values.numCols(); ++col) {
        qreal value = m_values.at(rowIndex, col);
        normalized.append((value - minVal) / range);
    }

    return normalized;
}

void SimulationData::denormalizeRow(qint32 rowIndex, QList<qreal>& data) const
{
    if (rowIndex < 0 || rowIndex >= m_minValues.size()) {
        return;
    }

    qreal minVal = m_minValues.value(rowIndex, 0.0);
    qreal maxVal = m_maxValues.value(rowIndex, 1.0);
    qreal range = maxVal - minVal;

    for (qreal& value : data) {
        value = value * range + minVal;
    }
}

} // namespace Domain
