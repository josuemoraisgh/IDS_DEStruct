#ifndef REFACTORED_TERM_H
#define REFACTORED_TERM_H

#include <QtGlobal>

/**
 * @brief Representa um termo na estrutura de regressão
 * 
 * Esta classe encapsula a representação de um termo individual
 * usado no algoritmo de evolução diferencial para identificação estrutural.
 * Segue o princípio de responsabilidade única.
 */

// Constantes para máscaras de bits
#define NUMDENOM    1
#define REGRESS     9
#define ATRASOS     11
#define VARIAVEL    11
#define MASKREG     ((1<<REGRESS)-1)

namespace Domain {

/**
 * @brief Tipo de função-base aplicada ao termo
 *
 * Permite diversificar a estrutura do modelo com diferentes
 * bases não-lineares, mantendo a representação compacta original.
 */
enum BasisType : quint8 {
    BasisPow     = 0,  // pow(x, e)           — original
    BasisAbsPow  = 1,  // |x|^e
    BasisLogPow  = 2,  // log(1 + |x|)^e
    BasisExp     = 3,  // exp(alpha * x)       — exponent stores alpha
    BasisTanhPow = 4,  // tanh(x)^e
    BasisCount   = 5   // sentinela (total de tipos)
};

/**
 * @brief Estrutura para armazenar informação detalhada do termo
 */
struct TermDetail1
{
    quint32 var    : VARIAVEL;
    quint32 atraso : ATRASOS;
    quint32 reg    : REGRESS;
    quint32 nd     : NUMDENOM; // 1 Numerador, 0 Denominador
};

/**
 * @brief Estrutura alternativa para armazenar informação do termo
 */
struct TermDetail2
{
    quint32 idVar   : ATRASOS+VARIAVEL;
    quint32 idReg   : NUMDENOM+REGRESS;
};

/**
 * @brief União para diferentes representações do termo
 */
union TermUnion {
    quint32      compact;    // Representação compacta
    TermDetail1  detailed1;  // Representação detalhada tipo 1
    TermDetail2  detailed2;  // Representação detalhada tipo 2
};

/**
 * @brief Classe que representa um termo completo com expoente
 */
class CompositeTerm
{
public:
    CompositeTerm();
    CompositeTerm(const CompositeTerm &other);
    ~CompositeTerm() = default;

    // Operadores
    CompositeTerm& operator=(const CompositeTerm &other);
    bool operator==(const CompositeTerm &other) const;
    bool operator>(const CompositeTerm &other) const;

    // Getters
    quint32 getCompact() const { return m_term.compact; }
    qreal getExponent() const { return m_exponent; }
    quint32 getVariable() const { return m_term.detailed1.var; }
    quint32 getDelay() const { return m_term.detailed1.atraso; }
    quint32 getRegression() const { return m_term.detailed1.reg; }
    bool isNumerator() const { return m_term.detailed1.nd == 1; }
    quint32 getRegGroupId() const { return m_term.detailed2.idReg; }
    BasisType getBasisType() const { return m_basisType; }

    // Setters
    void setCompact(quint32 value) { m_term.compact = value; }
    void setExponent(qreal value) { m_exponent = value; }
    void setVariable(quint32 value) { m_term.detailed1.var = value; }
    void setDelay(quint32 value) { m_term.detailed1.atraso = value; }
    void setRegression(quint32 value) { m_term.detailed1.reg = value; }
    void setNumerator(bool isNum) { m_term.detailed1.nd = isNum ? 1 : 0; }
    void setBasisType(BasisType bt) { m_basisType = bt; }

    // Operações matemáticas
    CompositeTerm inverse() const;
    CompositeTerm add(const CompositeTerm &other) const;
    CompositeTerm multiply(quint32 factor) const;

private:
    TermUnion m_term;
    qreal m_exponent;
    BasisType m_basisType;
};

} // namespace Domain

#endif // REFACTORED_TERM_H
