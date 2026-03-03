#ifndef EQUATION_FORMATTER_H
#define EQUATION_FORMATTER_H

#include "../interfaces/i_equation_formatter.h"

class SharedState;

///////////////////////////////////////////////////////////////////////////////
/// EquationFormatter — Implementa IEquationFormatter
///
/// SRP: Responsabilidade única = formatar equação a partir do cromossomo best.
/// Extrai slot_MW_EscreveEquacao() (parte de formatação) de imainwindow.cpp.
///////////////////////////////////////////////////////////////////////////////
class EquationFormatter : public IEquationFormatter
{
public:
    explicit EquationFormatter(SharedState *state);

    QString format(const QVector<Cromossomo> &crBest,
                   qint64 iteracoes,
                   const QVector<qreal> &somaEr) const override;

private:
    SharedState *m_state;
};

#endif // EQUATION_FORMATTER_H
