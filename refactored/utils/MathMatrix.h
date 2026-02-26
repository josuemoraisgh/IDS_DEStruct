#ifndef REFACTORED_MATH_MATRIX_H
#define REFACTORED_MATH_MATRIX_H

#include "MathVector.h"
#include <QVector>
#include <QtGlobal>
#include <QtMath>

namespace Utils {

/**
 * @brief Classe template para matriz matemática com operações básicas
 * 
 * Responsabilidade: Fornecer operações matriciais de forma encapsulada
 * Organização: Dados armazenados row-major (linhas contíguas)
 */
template<typename T>
class MathMatrix
{
public:
    MathMatrix() : m_rows(0), m_cols(0) {}
    
    MathMatrix(qint32 rows, qint32 cols, const T& value = T())
        : m_rows(rows), m_cols(cols), m_data(rows * cols, value)
    {}
    
    MathMatrix(const MathMatrix& other)
        : m_rows(other.m_rows), m_cols(other.m_cols), m_data(other.m_data)
    {}
    
    ~MathMatrix() = default;

    // Operadores
    MathMatrix& operator=(const MathMatrix& other)
    {
        if (this != &other) {
            m_rows = other.m_rows;
            m_cols = other.m_cols;
            m_data = other.m_data;
        }
        return *this;
    }

    // Acesso aos elementos
    T& at(qint32 row, qint32 col)
    {
        return m_data[row * m_cols + col];
    }

    const T& at(qint32 row, qint32 col) const
    {
        return m_data.at(row * m_cols + col);
    }

    // Informações
    qint32 numRows() const { return m_rows; }
    qint32 numCols() const { return m_cols; }
    qint32 size() const { return m_data.size(); }
    bool isEmpty() const { return m_data.isEmpty(); }

    // Modificadores
    void resize(qint32 rows, qint32 cols)
    {
        m_rows = rows;
        m_cols = cols;
        m_data.resize(rows * cols);
    }

    void resize(qint32 rows, qint32 cols, const T& value)
    {
        m_rows = rows;
        m_cols = cols;
        m_data.resize(rows * cols);
        m_data.fill(value);
    }

    void clear()
    {
        m_rows = 0;
        m_cols = 0;
        m_data.clear();
    }

    void fill(const T& value)
    {
        m_data.fill(value);
    }

    // Extração de vetores
    MathVector<T> getRow(qint32 row) const
    {
        MathVector<T> result(m_cols);
        for (qint32 col = 0; col < m_cols; ++col) {
            result[col] = at(row, col);
        }
        return result;
    }

    MathVector<T> getColumn(qint32 col) const
    {
        MathVector<T> result(m_rows);
        for (qint32 row = 0; row < m_rows; ++row) {
            result[row] = at(row, col);
        }
        return result;
    }

    void setRow(qint32 row, const MathVector<T>& vec)
    {
        if (vec.size() != m_cols) {
            qWarning("MathMatrix::setRow - size mismatch");
            return;
        }
        for (qint32 col = 0; col < m_cols; ++col) {
            at(row, col) = vec[col];
        }
    }

    void setColumn(qint32 col, const MathVector<T>& vec)
    {
        if (vec.size() != m_rows) {
            qWarning("MathMatrix::setColumn - size mismatch");
            return;
        }
        for (qint32 row = 0; row < m_rows; ++row) {
            at(row, col) = vec[row];
        }
    }

    // Operações matriciais
    MathMatrix transpose() const
    {
        MathMatrix result(m_cols, m_rows);
        for (qint32 row = 0; row < m_rows; ++row) {
            for (qint32 col = 0; col < m_cols; ++col) {
                result.at(col, row) = at(row, col);
            }
        }
        return result;
    }

    MathMatrix operator+(const MathMatrix& other) const
    {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            qWarning("MathMatrix::operator+ - dimension mismatch");
            return *this;
        }

        MathMatrix result(m_rows, m_cols);
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] + other.m_data[i];
        }
        return result;
    }

    MathMatrix operator-(const MathMatrix& other) const
    {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            qWarning("MathMatrix::operator- - dimension mismatch");
            return *this;
        }

        MathMatrix result(m_rows, m_cols);
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] - other.m_data[i];
        }
        return result;
    }

    MathMatrix operator*(const T& scalar) const
    {
        MathMatrix result(m_rows, m_cols);
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] * scalar;
        }
        return result;
    }

    MathVector<T> operator*(const MathVector<T>& vec) const
    {
        if (m_cols != vec.size()) {
            qWarning("MathMatrix::operator* - dimension mismatch");
            return MathVector<T>();
        }

        MathVector<T> result(m_rows, T());
        for (qint32 row = 0; row < m_rows; ++row) {
            T sum = T();
            for (qint32 col = 0; col < m_cols; ++col) {
                sum += at(row, col) * vec[col];
            }
            result[row] = sum;
        }
        return result;
    }

    // Acesso direto aos dados
    QVector<T>& data() { return m_data; }
    const QVector<T>& data() const { return m_data; }

private:
    qint32 m_rows;
    qint32 m_cols;
    QVector<T> m_data;  // Row-major storage
};

} // namespace Utils

#endif // REFACTORED_MATH_MATRIX_H
