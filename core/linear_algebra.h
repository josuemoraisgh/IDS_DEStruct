#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#include "xmatriz.h"
#include "xvetor.h"

///////////////////////////////////////////////////////////////////////////////
/// LinearAlgebra — Funções template de álgebra linear.
///
/// Extraídas de destruct.cpp para respeitar SRP.
/// Algoritmos mantidos EXATAMENTE iguais ao original.
///////////////////////////////////////////////////////////////////////////////

/// Sistema Linear via eliminação de Gauss (identico ao original)
template<typename T>
XVetor<T> SistemaLinear(bool &isOK, const JMathVar<T> &mat1, const XVetor<T> &vet1)
{
    qint32 n = vet1.size(), i, j, k, l;
    JMathVar<T> matAux(mat1);
    XVetor<T> result(n);
    matAux.append('C', vet1);
    double termo, m;
    try {
        for (k = 0; k < n - 1; k++) {
            for (i = k + 1; i < n; i++) {
                m = -1 * (matAux.item(i, k) / matAux.item(k, k));
                for (j = 0; j < n + 1; j++)
                    matAux(i, j) = (matAux.item(k, j) * m) + matAux.item(i, j);
            }
        }
        for (i = 0; i < n; i++) {
            termo = 0;
            l = n - i;
            for (j = l; j < n; j++)
                termo = termo + (result.at(j) * matAux.item(n - i - 1, j));
            result[n - i - 1] = (matAux.item(n - 1 - i, n) - termo) / matAux.item(n - i - 1, n - i - 1);
        }
        isOK = true;
    } catch (char *str) {
        Q_UNUSED(str);
        isOK = false;
    }
    return result;
}

/// A'*A : sendo mat1 = A' (transposta passada)
template<typename T>
JMathVar<T> AoQuadTrans(const JMathVar<T> &mat1)
{
    JMathVar<T> result(mat1.numLinhas(), mat1.numLinhas());
    for (qint32 linha = 0; linha < mat1.numLinhas(); linha++) {
        for (qint32 coluna = 0; coluna < mat1.numLinhas(); coluna++) {
            T sum(0.0f);
            for (int aux = 0; aux < mat1.numColunas(); aux++)
                sum += mat1.at(linha, aux) * mat1.at(coluna, aux);
            result(linha, coluna) = (sum != sum) || (sum > 1e9f) ? 1e9f : sum;
        }
    }
    return result;
}

/// A'*A : sendo mat1 = A (não-transposta)
template<typename T>
JMathVar<T> AoQuad(const JMathVar<T> &mat1)
{
    JMathVar<T> result(mat1.numColunas(), mat1.numColunas());
    for (qint32 linha = 0; linha < mat1.numColunas(); linha++) {
        for (qint32 coluna = 0; coluna < mat1.numColunas(); coluna++) {
            T sum(0.0f);
            for (int aux = 0; aux < mat1.numLinhas(); aux++)
                sum += mat1.item(aux, linha) * mat1.item(aux, coluna);
            result(linha, coluna) = (sum != sum) || (sum > 1e9f) ? 1e9f : sum;
        }
    }
    return result;
}

/// Multiplica matriz transposta por vetor
template<typename T>
XVetor<T> MultMatTransVet(const JMathVar<T> &mat1, const XVetor<T> &vet1)
{
    XVetor<T> result(mat1.numColunas());
    if (mat1.numLinhas() == vet1.size()) {
        for (qint32 coluna = 0; coluna < mat1.numColunas(); coluna++) {
            T sum(0.0f);
            for (qint32 aux = 0; aux < mat1.numLinhas(); aux++)
                sum += mat1.item(aux, coluna) * vet1.at(aux);
            result[coluna] = (sum != sum) ? 1e9f : sum;
        }
    }
    return result;
}

/// Multiplica matriz por vetor
template<typename T>
XVetor<T> MultMatVet(const JMathVar<T> &mat1, const XVetor<T> &vet1)
{
    XVetor<T> result(mat1.numLinhas());
    if (mat1.numColunas() == vet1.size()) {
        for (qint32 linha = 0; linha < mat1.numLinhas(); linha++) {
            T sum(0.0f);
            for (qint32 aux = 0; aux < mat1.numColunas(); aux++)
                sum += mat1.item(linha, aux) * vet1.at(aux);
            result[linha] = (sum != sum) ? 1e9f : sum;
        }
    }
    return result;
}

#endif // LINEAR_ALGEBRA_H
