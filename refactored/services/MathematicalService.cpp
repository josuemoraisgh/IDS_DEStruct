#include "MathematicalService.h"
#include <QtMath>
#include <algorithm>

namespace Services {

MathematicalService::MathematicalService()
{
}

bool MathematicalService::solveLinearSystem(const Utils::MathMatrix<qreal>& A,
                                           const Utils::MathVector<qreal>& b,
                                           Utils::MathVector<qreal>& solution)
{
    if (A.numRows() != A.numCols() || A.numRows() != b.size()) {
        qWarning("MathematicalService::solveLinearSystem - dimension mismatch");
        return false;
    }

    // Cria cópias para trabalhar (não modificar originais)
    Utils::MathMatrix<qreal> Acopy = A;
    Utils::MathVector<qreal> bcopy = b;

    // Eliminação Gaussiana
    if (!gaussianElimination(Acopy, bcopy)) {
        return false;
    }

    // Substituição reversa
    solution.resize(b.size());
    backSubstitution(Acopy, bcopy, solution);

    return true;
}

bool MathematicalService::gaussianElimination(Utils::MathMatrix<qreal>& A,
                                             Utils::MathVector<qreal>& b)
{
    qint32 n = A.numRows();

    // Faithful to original SistemaLinear — Gauss WITHOUT partial pivoting
    for (qint32 k = 0; k < n - 1; ++k) {
        for (qint32 i = k + 1; i < n; ++i) {
            qreal m = -1.0 * (A.at(i, k) / A.at(k, k));
            for (qint32 j = 0; j < n; ++j) {
                A.at(i, j) = A.at(k, j) * m + A.at(i, j);
            }
            b[i] = b[k] * m + b[i];
        }
    }

    return true;
}

bool MathematicalService::partialPivoting(Utils::MathMatrix<qreal>& A,
                                         Utils::MathVector<qreal>& b,
                                         qint32 row)
{
    qint32 n = A.numRows();
    qint32 maxRow = row;
    qreal maxVal = qAbs(A.at(row, row));

    // Encontra a linha com o maior elemento
    for (qint32 i = row + 1; i < n; ++i) {
        qreal val = qAbs(A.at(i, row));
        if (val > maxVal) {
            maxVal = val;
            maxRow = i;
        }
    }

    // Verifica se o pivô é muito pequeno
    if (maxVal < 1e-10) {
        qWarning("MathematicalService::partialPivoting - singular matrix");
        return false;
    }

    // Troca as linhas
    if (maxRow != row) {
        for (qint32 j = 0; j < A.numCols(); ++j) {
            qSwap(A.at(row, j), A.at(maxRow, j));
        }
        qSwap(b[row], b[maxRow]);
    }

    return true;
}

void MathematicalService::backSubstitution(const Utils::MathMatrix<qreal>& A,
                                          const Utils::MathVector<qreal>& b,
                                          Utils::MathVector<qreal>& solution)
{
    qint32 n = A.numRows();

    for (qint32 i = n - 1; i >= 0; --i) {
        qreal sum = 0.0;
        for (qint32 j = i + 1; j < n; ++j) {
            sum += A.at(i, j) * solution[j];
        }
        solution[i] = (b[i] - sum) / A.at(i, i);
    }
}

qreal MathematicalService::vectorNorm(const Utils::MathVector<qreal>& vec, qint32 p)
{
    if (vec.isEmpty()) {
        return 0.0;
    }

    if (p == 1) {
        qreal sum = 0.0;
        for (qint32 i = 0; i < vec.size(); ++i) {
            sum += qAbs(vec[i]);
        }
        return sum;
    }
    else if (p == 2) {
        qreal sum = 0.0;
        for (qint32 i = 0; i < vec.size(); ++i) {
            sum += vec[i] * vec[i];
        }
        return qSqrt(sum);
    }
    else { // norma infinito
        qreal maxVal = 0.0;
        for (qint32 i = 0; i < vec.size(); ++i) {
            maxVal = qMax(maxVal, qAbs(vec[i]));
        }
        return maxVal;
    }
}

qreal MathematicalService::meanSquaredError(const Utils::MathVector<qreal>& predicted,
                                           const Utils::MathVector<qreal>& actual)
{
    if (predicted.size() != actual.size() || predicted.isEmpty()) {
        return 0.0;
    }

    qreal sum = 0.0;
    for (qint32 i = 0; i < predicted.size(); ++i) {
        qreal diff = predicted[i] - actual[i];
        sum += diff * diff;
    }

    return sum / predicted.size();
}

qreal MathematicalService::sumSquaredErrors(const Utils::MathVector<qreal>& predicted,
                                           const Utils::MathVector<qreal>& actual)
{
    if (predicted.size() != actual.size()) {
        return 0.0;
    }

    qreal sum = 0.0;
    for (qint32 i = 0; i < predicted.size(); ++i) {
        qreal diff = predicted[i] - actual[i];
        sum += diff * diff;
    }

    return sum;
}

qreal MathematicalService::coefficientOfDetermination(const Utils::MathVector<qreal>& predicted,
                                                     const Utils::MathVector<qreal>& actual)
{
    if (predicted.size() != actual.size() || predicted.isEmpty()) {
        return 0.0;
    }

    qreal meanActual = actual.mean();
    qreal ssRes = 0.0;  // Soma dos quadrados dos resíduos
    qreal ssTot = 0.0;  // Soma total dos quadrados

    for (qint32 i = 0; i < predicted.size(); ++i) {
        qreal residual = actual[i] - predicted[i];
        qreal deviation = actual[i] - meanActual;
        ssRes += residual * residual;
        ssTot += deviation * deviation;
    }

    if (qAbs(ssTot) < 1e-10) {
        return 1.0;
    }

    return 1.0 - (ssRes / ssTot);
}

qreal MathematicalService::bayesianInformationCriterion(qreal sse, qint32 n, qint32 k)
{
    if (n <= 0 || sse < 0) {
        return 0.0;
    }

    return n * qLn(sse / n) + k * qLn(n);
}

qreal MathematicalService::akaikeInformationCriterion(qreal sse, qint32 n, qint32 k)
{
    if (n <= 0 || sse < 0) {
        return 0.0;
    }

    return n * qLn(sse / n) + 2 * k;
}

Utils::MathVector<qreal> MathematicalService::normalize(const Utils::MathVector<qreal>& vec)
{
    if (vec.isEmpty()) {
        return Utils::MathVector<qreal>();
    }

    qreal minVal = vec.min();
    qreal maxVal = vec.max();
    qreal range = maxVal - minVal;

    if (qAbs(range) < 1e-10) {
        return Utils::MathVector<qreal>(vec.size(), 0.5);
    }

    Utils::MathVector<qreal> result(vec.size());
    for (qint32 i = 0; i < vec.size(); ++i) {
        result[i] = (vec[i] - minVal) / range;
    }

    return result;
}

Utils::MathVector<qreal> MathematicalService::denormalize(const Utils::MathVector<qreal>& vec,
                                                         qreal minVal, qreal maxVal)
{
    if (vec.isEmpty()) {
        return Utils::MathVector<qreal>();
    }

    qreal range = maxVal - minVal;
    Utils::MathVector<qreal> result(vec.size());
    
    for (qint32 i = 0; i < vec.size(); ++i) {
        result[i] = vec[i] * range + minVal;
    }

    return result;
}

Utils::MathMatrix<qreal> MathematicalService::transpose(const Utils::MathMatrix<qreal>& matrix)
{
    return matrix.transpose();
}

Utils::MathMatrix<qreal> MathematicalService::multiply(const Utils::MathMatrix<qreal>& A,
                                                      const Utils::MathMatrix<qreal>& B)
{
    if (A.numCols() != B.numRows()) {
        qWarning("MathematicalService::multiply - dimension mismatch");
        return Utils::MathMatrix<qreal>();
    }

    Utils::MathMatrix<qreal> result(A.numRows(), B.numCols(), 0.0);

    for (qint32 i = 0; i < A.numRows(); ++i) {
        for (qint32 j = 0; j < B.numCols(); ++j) {
            qreal sum = 0.0;
            for (qint32 k = 0; k < A.numCols(); ++k) {
                sum += A.at(i, k) * B.at(k, j);
            }
            result.at(i, j) = sum;
        }
    }

    return result;
}

bool MathematicalService::inverse(const Utils::MathMatrix<qreal>& matrix,
                                 Utils::MathMatrix<qreal>& inverse)
{
    if (matrix.numRows() != matrix.numCols()) {
        qWarning("MathematicalService::inverse - matrix must be square");
        return false;
    }

    qint32 n = matrix.numRows();
    
    // Cria matriz aumentada [A | I]
    Utils::MathMatrix<qreal> augmented(n, 2 * n);
    for (qint32 i = 0; i < n; ++i) {
        for (qint32 j = 0; j < n; ++j) {
            augmented.at(i, j) = matrix.at(i, j);
            augmented.at(i, j + n) = (i == j) ? 1.0 : 0.0;
        }
    }

    // Eliminação de Gauss-Jordan
    for (qint32 i = 0; i < n; ++i) {
        // Encontra pivô
        qint32 maxRow = i;
        qreal maxVal = qAbs(augmented.at(i, i));
        for (qint32 k = i + 1; k < n; ++k) {
            if (qAbs(augmented.at(k, i)) > maxVal) {
                maxVal = qAbs(augmented.at(k, i));
                maxRow = k;
            }
        }

        if (maxVal < 1e-10) {
            qWarning("MathematicalService::inverse - singular matrix");
            return false;
        }

        // Troca linhas
        if (maxRow != i) {
            for (qint32 j = 0; j < 2 * n; ++j) {
                qSwap(augmented.at(i, j), augmented.at(maxRow, j));
            }
        }

        // Normaliza linha do pivô
        qreal pivot = augmented.at(i, i);
        for (qint32 j = 0; j < 2 * n; ++j) {
            augmented.at(i, j) /= pivot;
        }

        // Elimina coluna
        for (qint32 k = 0; k < n; ++k) {
            if (k != i) {
                qreal factor = augmented.at(k, i);
                for (qint32 j = 0; j < 2 * n; ++j) {
                    augmented.at(k, j) -= factor * augmented.at(i, j);
                }
            }
        }
    }

    // Extrai matriz inversa
    inverse.resize(n, n);
    for (qint32 i = 0; i < n; ++i) {
        for (qint32 j = 0; j < n; ++j) {
            inverse.at(i, j) = augmented.at(i, j + n);
        }
    }

    return true;
}

} // namespace Services
