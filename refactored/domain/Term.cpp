#include "Term.h"

namespace Domain {

CompositeTerm::CompositeTerm()
    : m_exponent(0.0)
    , m_basisType(BasisPow)
{
    m_term.compact = 0;
}

CompositeTerm::CompositeTerm(const CompositeTerm &other)
    : m_exponent(other.m_exponent)
    , m_basisType(other.m_basisType)
{
    m_term.compact = other.m_term.compact;
}

CompositeTerm& CompositeTerm::operator=(const CompositeTerm &other)
{
    if (this != &other) {
        m_term.compact = other.m_term.compact;
        m_exponent = other.m_exponent;
        m_basisType = other.m_basisType;
    }
    return *this;
}

bool CompositeTerm::operator==(const CompositeTerm &other) const
{
    return (m_term.compact == other.m_term.compact) && 
           (qAbs(m_exponent - other.m_exponent) < 1e-10) &&
           (m_basisType == other.m_basisType);
}

bool CompositeTerm::operator>(const CompositeTerm &other) const
{
    return m_term.compact > other.m_term.compact;
}

CompositeTerm CompositeTerm::inverse() const
{
    CompositeTerm result(*this);
    result.m_exponent = -m_exponent;
    return result;
}

CompositeTerm CompositeTerm::add(const CompositeTerm &other) const
{
    if (m_term.compact != other.m_term.compact) {
        qWarning("Cannot add terms with different variables");
        return *this;
    }
    
    CompositeTerm result(*this);
    result.m_exponent = m_exponent + other.m_exponent;
    return result;
}

CompositeTerm CompositeTerm::multiply(quint32 factor) const
{
    CompositeTerm result(*this);
    result.m_exponent = factor * m_exponent;
    return result;
}

} // namespace Domain
