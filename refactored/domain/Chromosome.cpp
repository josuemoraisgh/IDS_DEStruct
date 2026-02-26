#include "Chromosome.h"
#include <algorithm>

namespace Domain {

Chromosome::Chromosome()
    : m_error(0.0)
    , m_fitness(0.0)
    , m_outputId(0)
    , m_maxDelay(0)
{
}

Chromosome::Chromosome(const Chromosome &other)
    : m_error(other.m_error)
    , m_fitness(other.m_fitness)
    , m_outputId(other.m_outputId)
    , m_maxDelay(other.m_maxDelay)
    , m_errors(other.m_errors)
    , m_coefficients(other.m_coefficients)
    , m_regressions(other.m_regressions)
{
}

Chromosome& Chromosome::operator=(const Chromosome &other)
{
    if (this != &other) {
        m_error = other.m_error;
        m_fitness = other.m_fitness;
        m_outputId = other.m_outputId;
        m_maxDelay = other.m_maxDelay;
        
        m_errors = other.m_errors;
        m_coefficients = other.m_coefficients;
        m_regressions = other.m_regressions;
    }
    return *this;
}

bool Chromosome::operator==(const Chromosome &other) const
{
    return (qAbs(m_error - other.m_error) < 1e-10) &&
           (qAbs(m_fitness - other.m_fitness) < 1e-10) &&
           (m_outputId == other.m_outputId) &&
           (m_maxDelay == other.m_maxDelay);
}

void Chromosome::setRegressions(const QVector<QVector<CompositeTerm>>& regressions)
{
    m_regressions = regressions;
}

void Chromosome::setErrors(const Utils::MathVector<qreal>& errors)
{
    m_errors = errors;
}

void Chromosome::setCoefficients(const Utils::MathVector<qreal>& coefficients)
{
    m_coefficients = coefficients;
}

void Chromosome::addRegression(const QVector<CompositeTerm>& regression)
{
    m_regressions.append(regression);
}

void Chromosome::removeRegression(qint32 index)
{
    if (index >= 0 && index < m_regressions.size()) {
        m_regressions.remove(index);
    }
}

void Chromosome::removeTerm(qint32 regressionIndex, qint32 termIndex)
{
    if (regressionIndex >= 0 && regressionIndex < m_regressions.size()) {
        if (termIndex >= 0 && termIndex < m_regressions[regressionIndex].size()) {
            m_regressions[regressionIndex].remove(termIndex);
        }
    }
}

void Chromosome::clear()
{
    m_error = 0.0;
    m_fitness = 0.0;
    m_outputId = 0;
    m_maxDelay = 0;
    m_errors.clear();
    m_coefficients.clear();
    m_regressions.clear();
}

bool Chromosome::isValid() const
{
    return !m_regressions.isEmpty() && m_outputId >= 0;
}

qint32 Chromosome::getTermCount() const
{
    qint32 count = 0;
    for (const auto& regression : m_regressions) {
        count += regression.size();
    }
    return count;
}

} // namespace Domain
