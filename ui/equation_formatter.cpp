#include "equation_formatter.h"
#include "../threading/shared_state.h"
#include <cmath>
#include <QString>

///////////////////////////////////////////////////////////////////////////////
EquationFormatter::EquationFormatter(SharedState *state)
    : m_state(state)
{
}

///////////////////////////////////////////////////////////////////////////////
/// format — Lógica de formatação extraída de slot_MW_EscreveEquacao()
///          (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
QString EquationFormatter::format(const QVector<Cromossomo> &crBest,
                                   qint64 iteracoes,
                                   const QVector<qreal> &somaEr) const
{
    bool isFeito = false;
    QString str, strErr, strNum, strDen, strErrNum, strErrDen, strRegress;
    qint32 countRegress = 0, i = 0, index = 0, idVariavel = 0, idAtraso = 0, numColuna = 0, idSaida = 0;
    qreal idCoefic = 0., aux = 0., varAux = 0., jn = 0., jnM = 0., rsme = 0., idExpo = 0.;

    str.append(QString("Cl:= ") + QString::number(iteracoes) + QString("; "));

    for (i = 0; i < m_state->Adj.Dados.variaveis.nome.size(); i++) {
        if (i < m_state->Adj.Dados.variaveis.qtSaidas)
            str.append(m_state->Adj.Dados.variaveis.nome.at(i)
                + QString(": Max = ") + QString::number(m_state->Adj.Dados.variaveis.Vmaior.at(i))
                + QString(", Min = ") + QString::number(m_state->Adj.Dados.variaveis.Vmenor.at(i))
                + QString(", Decimacao = ") + QString::number(m_state->Adj.decimacao.at(i))
                + QString("; "));
        else
            str.append(m_state->Adj.Dados.variaveis.nome.at(i)
                + QString(": Max = ") + QString::number(m_state->Adj.Dados.variaveis.Vmaior.at(i))
                + QString(", Min = ") + QString::number(m_state->Adj.Dados.variaveis.Vmenor.at(i))
                + QString("; "));
    }

    for (idSaida = 0; idSaida < m_state->Adj.Dados.variaveis.qtSaidas; idSaida++) {
        numColuna = m_state->Adj.Dados.variaveis.valores.numColunas() - crBest.at(idSaida).maiorAtraso;
        strNum = ""; strDen = ""; strErrNum = ""; strErrDen = ""; strRegress = "";
        isFeito = false;

        for (countRegress = 0; countRegress < crBest.at(idSaida).regress.size(); countRegress++) {
            idCoefic = crBest.at(idSaida).vlrsCoefic.at(countRegress);
            if (crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.reg) {
                for (i = 0; i < crBest.at(idSaida).regress.at(countRegress).size(); i++) {
                    idVariavel = crBest.at(idSaida).regress.at(countRegress).at(i).vTermo.tTermo1.var;
                    if (!m_state->Adj.isTipoExpo)
                        idExpo = crBest.at(idSaida).regress.at(countRegress).at(i).expoente;
                    else {
                        idExpo = (qint32)crBest.at(idSaida).regress.at(countRegress).at(i).expoente;
                        idExpo += (crBest.at(idSaida).regress.at(countRegress).at(i).expoente - idExpo) >= 0.5 ? 1
                            : (crBest.at(idSaida).regress.at(countRegress).at(i).expoente - idExpo) <= -0.5 ? -1 : 0;
                        if (m_state->Adj.isTipoExpo == 2) idExpo = fabs(idExpo);
                    }
                    if (idExpo) {
                        idAtraso = crBest.at(idSaida).regress.at(countRegress).at(i).vTermo.tTermo1.atraso;
                        // Monta a string versao Scilab/MatLab
                        strRegress += (idCoefic) && (idExpo)
                            ? (idVariavel >= 1
                                ? ((idExpo != 1.) && (idExpo != 0.)
                                    ? QString("(" + m_state->Adj.Dados.variaveis.nome.at(idVariavel - 1) + "(k-%1)^%2)*").arg(idAtraso).arg(idExpo)
                                    : ((idExpo == 1.)
                                        ? QString(m_state->Adj.Dados.variaveis.nome.at(idVariavel - 1) + "(k-%1)*").arg(idAtraso)
                                        : ""))
                                : ((idExpo != 1.) && (idExpo != 0.)
                                    ? QString("(E(k-%1)^%2)*").arg(idAtraso).arg(idExpo)
                                    : (idExpo == 1. ? QString("E(k-%1)*").arg(idAtraso) : "")))
                            : "";
                    }
                }
                strRegress.remove(strRegress.size() - 1, 1);
                if (idCoefic) {
                    if (crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.nd) {
                        strNum += (QString(idCoefic > 0.0f ? " +%1*" : " %1*").arg(idCoefic)) + strRegress;
                        strErrNum += " " + QString::number(crBest.at(idSaida).err.at(countRegress)) + "; ";
                    } else {
                        strDen += (QString(idCoefic > 0.0f ? " +%1*" : " %1*").arg(idCoefic)) + strRegress;
                        strErrDen += " " + QString::number(crBest.at(idSaida).err.at(countRegress)) + "; ";
                    }
                }
                strRegress = "";
            } else {
                strRegress = "";
                if (idCoefic) {
                    if (crBest.at(idSaida).regress.at(countRegress).at(0).vTermo.tTermo1.nd) {
                        strNum = QString("%1").arg(idCoefic) + strNum;
                        strErrNum = QString::number(crBest.at(idSaida).err.at(countRegress)) + "; " + strErrNum;
                    } else {
                        isFeito = true;
                        strDen = QString("%1").arg(idCoefic + 1) + strDen;
                        strErrDen = QString::number(crBest.at(idSaida).err.at(countRegress)) + "; " + strErrDen;
                    }
                }
            }
        }
        /////////////////////////////////////////////////////////////////
        const qint32 tamErro = crBest.at(idSaida).vlrsCoefic.size() - crBest.at(idSaida).regress.size();
        for (countRegress = 0; countRegress < tamErro; countRegress++) {
            idCoefic = crBest.at(idSaida).vlrsCoefic.at(crBest.at(idSaida).regress.size() + countRegress);
            strErr += (QString(idCoefic > 0.0f ? " +%1*E(k-%2)" : " %1*E(k-%2)").arg(idCoefic).arg(countRegress + 1));
        }
        /////////////////////////////////////////////////////////////////
        if (strNum == "") strNum = "1";
        if (strDen == "") strDen = "1";
        else if (!isFeito) {
            strDen = QString("+1") + strDen;
            strErrDen = QString("-1;") + strErrDen;
        }
        /////////////////////////////////////////////////////////////////
        for (index = crBest.at(idSaida).maiorAtraso + 2; index < numColuna + crBest.at(idSaida).maiorAtraso; index++) {
            aux = m_state->Adj.Dados.variaveis.valores.at(idSaida, index)
                - m_state->Adj.Dados.variaveis.valores.at(idSaida, index - 2);
            varAux += aux * aux;
        }
        jn = crBest.at(idSaida).erro;
        jnM = somaEr.at(idSaida) / (m_state->Adj.Dados.tamPop);
        rsme = (sqrt(jn)) / (sqrt(varAux / (numColuna - 2)));
        str.append(QString("\nBIC:= %1; RMSE(2):= %2; Jn(Menor):= %3; Jn(Md):= %4")
                       .arg(crBest.at(idSaida).aptidao).arg(rsme).arg(jn).arg(jnM));
        if (strNum.size())
            str.append(QString("\n%1(k) = ((%1_Num+%1_Err)/(%1_Den));\n%1_Num = " + strNum
                + ";\n%1_Err = " + strErr + ";\n%1_Den = " + strDen
                + ";\nERR_Num:=(" + strErrNum + ");\nERR_Den:=(" + strErrDen + ");")
                           .arg(m_state->Adj.Dados.variaveis.nome.at(idSaida)));
    }
    str.append(QString("\n"));
    return str;
}
