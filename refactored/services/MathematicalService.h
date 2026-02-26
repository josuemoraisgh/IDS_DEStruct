#ifndef REFACTORED_MATHEMATICAL_SERVICE_H
#define REFACTORED_MATHEMATICAL_SERVICE_H

#include "../domain/Chromosome.h"
#include "../utils/MathMatrix.h"
#include "../utils/MathVector.h"

namespace Services {

/**
 * @brief Serviço para operações matemáticas complexas
 * 
 * Responsabilidade única: Centralizar cálculos matemáticos
 * usados no algoritmo de identificação estrutural
 */
class MathematicalService
{
public:
    MathematicalService();
    ~MathematicalService() = default;

    /**
     * @brief Resolve um sistema linear Ax = b usando eliminação de Gauss
     * @param A Matriz de coeficientes
     * @param b Vetor de termos independentes
     * @param solution Vetor solução (saída)
     * @return true se o sistema foi resolvido com sucesso
     */
    bool solveLinearSystem(const Utils::MathMatrix<qreal>& A,
                          const Utils::MathVector<qreal>& b,
                          Utils::MathVector<qreal>& solution);

    /**
     * @brief Calcula a norma de um vetor
     * @param vec Vetor
     * @param p Tipo de norma (1, 2, ou infinito)
     * @return Valor da norma
     */
    qreal vectorNorm(const Utils::MathVector<qreal>& vec, qint32 p = 2);

    /**
     * @brief Calcula o erro quadrático médio (MSE)
     * @param predicted Valores preditos
     * @param actual Valores reais
     * @return MSE
     */
    qreal meanSquaredError(const Utils::MathVector<qreal>& predicted,
                          const Utils::MathVector<qreal>& actual);

    /**
     * @brief Calcula a soma dos erros quadrados (SSE)
     * @param predicted Valores preditos
     * @param actual Valores reais
     * @return SSE
     */
    qreal sumSquaredErrors(const Utils::MathVector<qreal>& predicted,
                          const Utils::MathVector<qreal>& actual);

    /**
     * @brief Calcula o coeficiente de determinação (R²)
     * @param predicted Valores preditos
     * @param actual Valores reais
     * @return R²
     */
    qreal coefficientOfDetermination(const Utils::MathVector<qreal>& predicted,
                                    const Utils::MathVector<qreal>& actual);

    /**
     * @brief Calcula o critério de informação bayesiano (BIC)
     * @param sse Soma dos erros quadrados
     * @param n Número de observações
     * @param k Número de parâmetros
     * @return Valor do BIC
     */
    qreal bayesianInformationCriterion(qreal sse, qint32 n, qint32 k);

    /**
     * @brief Calcula o critério de informação de Akaike (AIC)
     * @param sse Soma dos erros quadrados
     * @param n Número de observações
     * @param k Número de parâmetros
     * @return Valor do AIC
     */
    qreal akaikeInformationCriterion(qreal sse, qint32 n, qint32 k);

    /**
     * @brief Normaliza um vetor para o intervalo [0, 1]
     * @param vec Vetor a ser normalizado
     * @return Vetor normalizado
     */
    Utils::MathVector<qreal> normalize(const Utils::MathVector<qreal>& vec);

    /**
     * @brief Denormaliza um vetor do intervalo [0, 1]
     * @param vec Vetor normalizado
     * @param minVal Valor mínimo original
     * @param maxVal Valor máximo original
     * @return Vetor denormalizado
     */
    Utils::MathVector<qreal> denormalize(const Utils::MathVector<qreal>& vec,
                                        qreal minVal, qreal maxVal);

    /**
     * @brief Calcula a transposta de uma matriz
     * @param matrix Matriz
     * @return Matriz transposta
     */
    Utils::MathMatrix<qreal> transpose(const Utils::MathMatrix<qreal>& matrix);

    /**
     * @brief Multiplica duas matrizes
     * @param A Primeira matriz
     * @param B Segunda matriz
     * @return Produto A * B
     */
    Utils::MathMatrix<qreal> multiply(const Utils::MathMatrix<qreal>& A,
                                     const Utils::MathMatrix<qreal>& B);

    /**
     * @brief Calcula a inversa de uma matriz
     * @param matrix Matriz a ser invertida
     * @param inverse Matriz inversa (saída)
     * @return true se a matriz foi invertida com sucesso
     */
    bool inverse(const Utils::MathMatrix<qreal>& matrix,
                Utils::MathMatrix<qreal>& inverse);

private:
    /**
     * @brief Realiza pivoteamento parcial em um sistema linear
     * @param A Matriz de coeficientes
     * @param b Vetor de termos independentes
     * @param row Linha atual
     * @return true se o pivoteamento foi bem-sucedido
     */
    bool partialPivoting(Utils::MathMatrix<qreal>& A,
                        Utils::MathVector<qreal>& b,
                        qint32 row);

    /**
     * @brief Realiza eliminação Gaussiana
     * @param A Matriz de coeficientes
     * @param b Vetor de termos independentes
     * @return true se a eliminação foi bem-sucedida
     */
    bool gaussianElimination(Utils::MathMatrix<qreal>& A,
                            Utils::MathVector<qreal>& b);

    /**
     * @brief Realiza substituição reversa
     * @param A Matriz triangular superior
     * @param b Vetor de termos independentes
     * @param solution Vetor solução (saída)
     */
    void backSubstitution(const Utils::MathMatrix<qreal>& A,
                         const Utils::MathVector<qreal>& b,
                         Utils::MathVector<qreal>& solution);
};

} // namespace Services

#endif // REFACTORED_MATHEMATICAL_SERVICE_H
