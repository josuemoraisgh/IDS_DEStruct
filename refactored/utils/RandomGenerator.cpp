#include "RandomGenerator.h"
#include <QDateTime>
#include <QtMath>

namespace Utils {

RandomGenerator::RandomGenerator()
    : m_seed(QDateTime::currentMSecsSinceEpoch())
{
    m_generator = new QRandomGenerator(m_seed);
}

RandomGenerator::RandomGenerator(quint32 seed)
    : m_seed(seed)
{
    m_generator = new QRandomGenerator(m_seed);
}

void RandomGenerator::setSeed(quint32 seed)
{
    m_seed = seed;
    delete m_generator;
    m_generator = new QRandomGenerator(m_seed);
}

qint32 RandomGenerator::randInt(qint32 min, qint32 max)
{
    return min + (m_generator->generate() % (max - min + 1));
}

quint32 RandomGenerator::randUInt(quint32 min, quint32 max)
{
    return min + (m_generator->generate() % (max - min + 1));
}

qreal RandomGenerator::randReal()
{
    return m_generator->generateDouble();
}

qreal RandomGenerator::randReal(qreal min, qreal max)
{
    return min + (max - min) * m_generator->generateDouble();
}

qreal RandomGenerator::randNormal(qreal mean, qreal stddev)
{
    // Box-Muller transform
    qreal u1 = randReal();
    qreal u2 = randReal();
    
    qreal z0 = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
    return mean + stddev * z0;
}

qreal RandomGenerator::randExponential(qreal lambda)
{
    qreal u = randReal();
    return -qLn(u) / lambda;
}

} // namespace Utils
