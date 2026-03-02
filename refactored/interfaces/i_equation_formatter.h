#ifndef I_EQUATION_FORMATTER_H
#define I_EQUATION_FORMATTER_H

#include <QString>
#include <QVector>
#include "xtipodados.h"

///////////////////////////////////////////////////////////////////////////////
/// Interface para formatação de equações (SRP + ISP)
/// Responsabilidade única: converter cromossomo em string legível.
///////////////////////////////////////////////////////////////////////////////
class IEquationFormatter
{
public:
    virtual ~IEquationFormatter() = default;

    virtual QString format(const QVector<Cromossomo> &crBest,
                           qint64 iteracoes,
                           const QVector<qreal> &somaEr) const = 0;
};

#endif // I_EQUATION_FORMATTER_H
