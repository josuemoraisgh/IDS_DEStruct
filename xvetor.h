#ifndef XVETOR_H
#define XVETOR_H

#include <QGlobal.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <QVector>

template<typename T>
class XMatriz;

template<typename T>
class XVetor : public QVector<T>
{
public:
    XVetor():QVector<T>(){}
    XVetor(const qint32 &size):QVector<T>(size){}
    XVetor(const qint32 &size,const T &valor):QVector<T>(size,valor){}
    XVetor(const XMatriz<T> &mat):QVector<T>(mat.size()){*((QVector<T> *) this) = ((QVector<T> ) mat);}

    void append(const XVetor<T> &vet);
    void append(const T &vlr);

    void insereElementos(const qint32 &posicao,const T *begin, const T *end, const qint32 &step);
    void Copy(const qint32 &posIni, const qint32 &stepLinha,const T *begin, const T *end, const qint32 &step);

    qreal ProdEsc(const XVetor<T> &vet);
    XVetor<T> ProdVet(const XVetor<T> &vet);
    XVetor<T> operator*(const qreal &val);
    XVetor<T> operator*(const XVetor<T> &vet);
    XMatriz<T> operator*(const XMatriz<T> &mat);
    XMatriz<T> Trans();
    XVetor<T> &operator=(const XVetor<T> &v);
    T soma();
    T normal();
};
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void XVetor<T>::append(const T &vlr)
{
    ((QVector<T> *) this)->append(vlr);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void XVetor<T>::append(const XVetor<T> &vet)
{
    *this += vet;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T XVetor<T>::soma()
{
    double soma = 0.0;
    //Soma os elementos
    for (qint32 i = 0; i < this->size(); i++) soma += this->at(i);
    return soma;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T XVetor<T>::normal()
{
    T norma(0.0f);
    //Calcula a norma do vetor
    for (qint32 i = 0; i < this->size(); i++)
        norma += (this->at(i) * this->at(i));
    return sqrt(norma);
}
//////////////////////////////////////////////////////////////////////////////////////////
//Altera os termos (posicao de inicio, tamanho, Endereco do inicio do outro vetor , passo para ser dado no outro vetor);
template<typename T>
void XVetor<T>::Copy(const qint32 &posIni, const qint32 &stepLinha,const T *begin, const T *end, const qint32 &step)
{
    if((begin!=NULL)&&(end!=NULL)&&(posIni>=0)&&(posIni<this->size())&&(stepLinha>0)&&(step>0))
    {
        qint32 dest = posIni;
        while (begin < end)
        {
            if(dest >= this->size()) break;
            (*this)[dest] = *begin;
            dest+=stepLinha;
            begin+=step;
        }
    }
#ifdef _DEBUG
    else qDebug() << "Func:XVetor<T>::Copy-falhou";
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>/*C1[L1 L2 L3 ..] C2[L1 L2 L3 ..] C3[L1 L2 L3 ..] C4[L1 L2 L3 ..] C..*/
void XVetor<T>::insereElementos(const qint32 &posicao,const T *begin, const T *end, const qint32 &step)
{
    if((begin!=NULL)&&(end!=NULL)&&(posicao>=0)&&(posicao<=this->size())&&(step>0))
    {
        qint32 pos = posicao;
        while (begin < end)
        {
            ((QVector<T> *) this)->insert(pos, *begin);
            begin += step;
            ++pos;
        }
    }
#ifdef _DEBUG
    else qDebug() << "Func:XVetor<T>::insereElementos-falhou";
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
XVetor<T> &XVetor<T>::operator=(const XVetor<T> &v)
{
    ((QVector<T> *) this)->operator=(v);
    return *this;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
qreal XVetor<T>::ProdEsc(const XVetor<T> &vet)
{
    qreal result=0.0;
    //Multiplica os vetores
    for (qint32 i = 0; i < this->size() && i < vet.size(); i++)
        result += this->at(i) * vet.at(i);
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
XVetor<T> XVetor<T>::ProdVet(const XVetor<T> &vet)
{
    XVetor<T> vetRes(this->size());
    //Multiplica os vetores
    for (qint32 i = 0; i < this->size() && i < vet.size(); i++)
    {
        const qint32 thisI1 = (i + 1) % this->size();
        const qint32 thisI2 = (i + 2) % this->size();
        const qint32 vetI1 = (i + 1) % vet.size();
        const qint32 vetI2 = (i + 2) % vet.size();
        vetRes[i] = this->at(thisI1) * vet.at(vetI2) - this->at(thisI2) * vet.at(vetI1);
    }
    return vetRes;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
XVetor<T> XVetor<T>::operator*(const qreal &val)
{
    XVetor<T> vetRes(this->size());
    //Multiplica o vetor por uma constante
    for (qint32 i = 0; i < this->size(); i++)
        vetRes[i] = this->at(i) * val;
    return vetRes;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
XVetor<T> XVetor<T>::operator*(const XVetor<T> &vet)
{
    XVetor<T> vetRes(this->size());
    //Multiplica os vetores
    for (int i = 0; i < this->size() && i < vet.size(); i++)
        vetRes[i] = this->at(i) * vet.at(i);
    return vetRes;
}
//////////////////////////////////////////////////////////////////////////////////////////
//<1, M1> = <1, M2> * <M2,M1>
template <typename T>
XMatriz<T> XVetor<T>::operator*(const XMatriz<T> &mat)
{
    XMatriz<T> result(1,mat.numColunas());
    for (int coluna = 0; coluna < mat.numColunas(); ++coluna)
    {
        T sum(0.0f);
        for (int aux = 0; aux < this->size() && aux < mat.numLinhas(); ++aux) sum += this->at(aux) * mat.item(aux,coluna);
        result(0,coluna) = sum;
    }
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
XMatriz<T> XVetor<T>::Trans()
{
    XMatriz<T> result(this->size(),1);
    *((QVector<T> *) &result) = *((QVector<T> *) this);
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////
#endif // XVETOR

