#ifndef REFACTORED_RANDOM_GENERATOR_H
#define REFACTORED_RANDOM_GENERATOR_H

#include <QRandomGenerator>
#include <QtGlobal>

namespace Utils {

/**
 * @brief Gerador de números aleatórios encapsulado
 * 
 * Responsabilidade: Fornecer geração de números aleatórios
 * de forma encapsulada e thread-safe quando necessário
 */
class RandomGenerator
{
public:
    RandomGenerator();
    explicit RandomGenerator(quint32 seed);
    ~RandomGenerator() = default;

    // Geração de números inteiros
    qint32 randInt(qint32 min, qint32 max);
    quint32 randUInt(quint32 min, quint32 max);

    // Geração de números reais
    qreal randReal();                    // [0, 1)
    qreal randReal(qreal min, qreal max); // [min, max)
    
    // Distribuições específicas
    qreal randNormal(qreal mean = 0.0, qreal stddev = 1.0);
    qreal randExponential(qreal lambda = 1.0);

    // Configuração
    void setSeed(quint32 seed);
    quint32 getSeed() const { return m_seed; }

private:
    QRandomGenerator* m_generator;
    quint32 m_seed;
};

} // namespace Utils

#endif // REFACTORED_RANDOM_GENERATOR_H
