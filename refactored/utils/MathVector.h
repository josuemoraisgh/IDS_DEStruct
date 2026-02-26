#ifndef REFACTORED_MATH_VECTOR_H
#define REFACTORED_MATH_VECTOR_H

#include <QVector>
#include <QtGlobal>
#include <QtMath>
#include <algorithm>

namespace Utils {

/**
 * @brief Classe template para vetor matemático com operações básicas
 * 
 * Responsabilidade: Fornecer operações matemáticas vetoriais de forma encapsulada
 */
template<typename T>
class MathVector
{
public:
    MathVector() {}
    explicit MathVector(qint32 size, const T& value = T()) : m_data(size, value) {}
    MathVector(const MathVector& other) : m_data(other.m_data) {}
    MathVector(const QVector<T>& data) : m_data(data) {}
    ~MathVector() = default;

    // Operadores
    MathVector& operator=(const MathVector& other)
    {
        if (this != &other) {
            m_data = other.m_data;
        }
        return *this;
    }

    T& operator[](qint32 index) { return m_data[index]; }
    const T& operator[](qint32 index) const { return m_data[index]; }

    T& at(qint32 index) { return m_data[index]; }
    const T& at(qint32 index) const { return m_data.at(index); }

    // Acesso aos dados
    QVector<T>& data() { return m_data; }
    const QVector<T>& data() const { return m_data; }
    
    T* raw() { return m_data.data(); }
    const T* raw() const { return m_data.data(); }

    // Informações
    qint32 size() const { return m_data.size(); }
    bool isEmpty() const { return m_data.isEmpty(); }

    // Modificadores
    void resize(qint32 size) { m_data.resize(size); }
    void resize(qint32 size, const T& value) { m_data.resize(size); m_data.fill(value, size); }
    void clear() { m_data.clear(); }
    void fill(const T& value) { m_data.fill(value); }
    void append(const T& value) { m_data.append(value); }
    void remove(qint32 index) { m_data.remove(index); }

    // Operações matemáticas
    T sum() const
    {
        T result = T();
        for (const T& val : m_data) {
            result += val;
        }
        return result;
    }

    T mean() const
    {
        if (m_data.isEmpty()) return T();
        return sum() / m_data.size();
    }

    T min() const
    {
        if (m_data.isEmpty()) return T();
        return *std::min_element(m_data.begin(), m_data.end());
    }

    T max() const
    {
        if (m_data.isEmpty()) return T();
        return *std::max_element(m_data.begin(), m_data.end());
    }

    T dotProduct(const MathVector& other) const
    {
        if (m_data.size() != other.size()) {
            qWarning("MathVector::dotProduct - size mismatch");
            return T();
        }
        
        T result = T();
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result += m_data[i] * other[i];
        }
        return result;
    }

    MathVector operator+(const MathVector& other) const
    {
        if (m_data.size() != other.size()) {
            qWarning("MathVector::operator+ - size mismatch");
            return *this;
        }
        
        MathVector result(m_data.size());
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] + other[i];
        }
        return result;
    }

    MathVector operator-(const MathVector& other) const
    {
        if (m_data.size() != other.size()) {
            qWarning("MathVector::operator- - size mismatch");
            return *this;
        }
        
        MathVector result(m_data.size());
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] - other[i];
        }
        return result;
    }

    MathVector operator*(const T& scalar) const
    {
        MathVector result(m_data.size());
        for (qint32 i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] * scalar;
        }
        return result;
    }

    // Iteradores
    typename QVector<T>::iterator begin() { return m_data.begin(); }
    typename QVector<T>::const_iterator begin() const { return m_data.begin(); }
    typename QVector<T>::iterator end() { return m_data.end(); }
    typename QVector<T>::const_iterator end() const { return m_data.end(); }

private:
    QVector<T> m_data;
};

} // namespace Utils

#endif // REFACTORED_MATH_VECTOR_H
