#ifndef REFACTORED_CHROMOSOME_H
#define REFACTORED_CHROMOSOME_H

#include <QVector>
#include <QMetaType>
#include "Term.h"
#include "../utils/MathMatrix.h"

namespace Domain {

/**
 * @brief Classe que representa um cromossomo no algoritmo de evolução diferencial
 * 
 * Responsabilidade única: Armazenar e gerenciar os dados de um indivíduo
 * na população do algoritmo genético.
 */
class Chromosome
{
public:
    Chromosome();
    Chromosome(const Chromosome &other);
    ~Chromosome() = default;

    // Operadores
    Chromosome& operator=(const Chromosome &other);
    bool operator==(const Chromosome &other) const;

    // Getters
    qreal getError() const { return m_error; }
    qreal getFitness() const { return m_fitness; }
    qint32 getOutputId() const { return m_outputId; }
    qint32 getMaxDelay() const { return m_maxDelay; }
    
    const QVector<QVector<CompositeTerm>>& getRegressions() const { return m_regressions; }
    const Utils::MathVector<qreal>& getErrors() const { return m_errors; }
    const Utils::MathVector<qreal>& getCoefficients() const { return m_coefficients; }

    // Setters
    void setError(qreal error) { m_error = error; }
    void setFitness(qreal fitness) { m_fitness = fitness; }
    void setOutputId(qint32 id) { m_outputId = id; }
    void setMaxDelay(qint32 delay) { m_maxDelay = delay; }
    
    void setRegressions(const QVector<QVector<CompositeTerm>>& regressions);
    void setErrors(const Utils::MathVector<qreal>& errors);
    void setCoefficients(const Utils::MathVector<qreal>& coefficients);

    // Métodos de manipulação
    void addRegression(const QVector<CompositeTerm>& regression);
    void removeRegression(qint32 index);
    void removeTerm(qint32 regressionIndex, qint32 termIndex);
    void clear();
    
    // Validação
    bool isValid() const;
    qint32 getTermCount() const;
    qint32 getRegressionCount() const { return m_regressions.size(); }

private:
    qreal m_error;              // Erro calculado
    qreal m_fitness;            // Aptidão (fitness) calculada pelo método BIC
    qint32 m_outputId;          // Identificador da saída para este cromossomo
    qint32 m_maxDelay;          // Maior atraso presente
    
    Utils::MathVector<qreal> m_errors;      // Vetor de erros
    Utils::MathVector<qreal> m_coefficients; // Coeficientes calculados
    QVector<QVector<CompositeTerm>> m_regressions; // Regressões
};

/**
 * @brief Functor para comparação de cromossomos por aptidão
 */
class ChromosomeFitnessComparator
{
public:
    bool operator()(const Chromosome &a, const Chromosome &b) const
    {
        if (qAbs(a.getFitness() - b.getFitness()) < 1e-10) {
            return a.getError() < b.getError();
        }
        return a.getFitness() < b.getFitness();
    }
};

} // namespace Domain

Q_DECLARE_METATYPE(Domain::Chromosome)
Q_DECLARE_METATYPE(QVector<Domain::Chromosome>)

#endif // REFACTORED_CHROMOSOME_H
