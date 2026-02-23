#ifndef JMathVar_H
#define JMathVar_H

#include <math.h>
#include <QGlobal.h>
#include <QDebug>
#include <QtMath>
#include <stdio.h>
#include <stdlib.h>
#include <QVector>

//Inser??????o de constantes reais com nota??????o cientifica apenas pelo argReal n???o podendo ser inserida na string.
class JStrSet
{
 public:
    const char *JSS_str;
    qint32 JSS_id[12],JSS_indexId,JSS_indexRd;
    qreal JSS_rd[2];
    JStrSet(){JSS_indexId=0;JSS_indexRd=0;}
    JStrSet(const char *str){JSS_str=str;JSS_indexId=0;JSS_indexRd=0;}
    inline JStrSet &set(const char *str) {JSS_str=str;JSS_indexId=0;JSS_indexRd=0;return *this;}
    inline JStrSet &argInt(const qint32 &ai1) {JSS_id[JSS_indexId++] = ai1;return *this;}
    inline JStrSet &argReal(const qreal &ar1) {JSS_rd[JSS_indexRd++] = ar1;return *this;}
    inline JStrSet &operator=(const char *ch){return set(ch);}
    bool inline operator==(const char *ch2) const
    {
        const char *ch1 = JSS_str;
        while(*ch1!='\0'&&*ch2!='\0'&&*ch1++==*ch2++);
        return *ch1==*ch2?true:false;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //id[0]{iniLinha1}=0,id[1]{stepLinha1}=0,id[2]{fimLinha1}=this->numLinhas(),
    //id[3]{iniColuna1}=0,id[4]{stepColuna1}=1,id[5]{fimColuna1}=this->numColunas(),
    //id[6]{isTransposta1}=0;
    //id[7]{iniLinha2}=0,id[8]{stepLinha2}=0,id[9]{fimLinha2}=mt.numLinhas(),
    //id[10]{iniColuna2}=0,id[11]{stepColuna2}=1,id[12]{fimColuna2}=mt.numColunas(),
    //id[13]{isTransposta2}=0;
    //id[14]{operador}=0,
    //////////////////////////////////////////////////////////////////////////////////////////
    //rd[0]{multiplicador}=1,rd[1]{expoente}=1;
    //////////////////////////////////////////////////////////////////////////////////////////
    void extract(qint32 id[15],qreal rd[2]) const
    {
        qreal numberReal=0.,multNumber=1;
        qint32 j=0,tipoDado=0,number=0; //Para tipoDados = 0 numero inteiro, 1 numero real, 2 indice do array inteiro , 3 indice do array real;
        const char *ch = JSS_str;
        bool isExp=false;
        while(*ch!='\0')
        {
            switch(*ch)
            {
                case '=': id[14]=0;tipoDado=0;multNumber=1;break;
                case '+':
                    if(*(ch+1)=='=') {ch++; id[14]=1;}
                    else if((*(ch+1)=='(')&&((*(ch-1)==')')||(*(ch-1)=='\''))) id[14]=1;
                    tipoDado=0;multNumber=1;break;
                case '-':
                    if(*(ch+1)=='=') {ch++;id[14]=2;tipoDado=0;multNumber=1;} //Subtra??????o de Matrizes
                    else if((*(ch+1)=='(')&&((*(ch-1)==')')||(*(ch-1)=='\''))) {id[14]=2;tipoDado=0;multNumber=1;} //Subtra??????o de Matrizes
                    else multNumber=-1; //Caso o numero lido tenha o sinal negativo
                    break;
                case '*':
                    if(*(ch+1)=='=') {ch++;id[14]=3;}//Multiplica??????o de Matrizes
                    else if((*(ch+1)=='(')&&((*(ch-1)==')')||(*(ch-1)=='\''))) id[14]=3;//Multiplica??????o de Matrizes
                    else
                    {
                        if((multNumber>1)||(multNumber<-1)) {rd[0]=(tipoDado==3?JSS_rd[number-1]:tipoDado==2?JSS_id[number-1]:(tipoDado==1?numberReal:number));} else tipoDado=1;
                    } //Multiplica??????o de uma Matriz por uma constante
                    tipoDado=0;multNumber=1;break;
                case '/':
                    if(*(ch+1)=='=') {ch++;id[14]=4;}
                    else if((*(ch+1)=='(')&&((*(ch-1)==')')||(*(ch-1)=='\''))) id[14]=4;
                    tipoDado=0;multNumber=1;break;
                case '%': if(*(ch+1)=='f') {ch++;tipoDado=3;} else tipoDado=2; break;
                case '^': isExp=true;tipoDado=1;break;
                case '\'':
                    id[j++]=1;tipoDado=0;multNumber=1;break;
                //case ';': id[j++]=2;str.clear();tipoDado=0;idNumber=1;break; //Operador Inversor
                case '(':
                    if(j==6) j++;tipoDado=0;multNumber=1;break;
                case ':':
                    if((multNumber>1)||(multNumber<-1)) id[j++]=tipoDado?JSS_id[number-1]:number;
                    tipoDado=0;
                    multNumber=1;
                    break;
                case ')':
                    if((multNumber>1)||(multNumber<-1))
                    {
                        number = tipoDado?JSS_id[number-1]:number;
                        switch(j)
                        {
                            case 0: id[0]=0;id[1]=1;id[2]=1;id[3]=number;id[4]=1;id[5]=id[3]+1;j=6;break;
                            case 1: id[3]=id[0];id[5]=number;id[0]=0;id[1]=1;id[2]=1;j=6;break;
                            case 2: id[3]=id[0];id[4]=id[1];id[5]=number;id[0]=0;id[1]=1;id[2]=1;j=6;break;
                            case 3: id[j++]=number;id[j++]=1;id[j++]=id[3]+1;break;
                            case 4: j++;id[j++]=number;break;
                            case 5: id[j++]=number;break;
                            case 7: id[7]=0;id[8]=1;id[9]=1;id[10]=number;id[11]=1;id[12]=id[10]+1;j=13;break;
                            case 8: id[10]=id[7];id[12]=number;id[7]=0;id[8]=1;id[9]=1;j=13;break;
                            case 9: id[10]=id[7];id[11]=id[8];id[12]=number;id[7]=0;id[8]=1;id[9]=1;j=13;break;
                            case 10: id[j++]=number;id[j++]=1;id[j++]=id[10]+1;break;
                            case 11: j++;id[j++]=number;break;
                            case 12: id[j++]=number;
                        }
                    }
                    else
                    {
                        switch(j)
                        {
                            case 0: j=6;break;
                            case 1: id[3]=id[0];id[0]=0;j=6;break;
                            case 3: j=6;break;
                            case 4: j=6;break;
                            case 7: j=13;break;
                            case 8: id[10]=id[7];id[7]=0;j=13;break;
                            case 10:j=13;break;
                            case 11:j=13;break;
                        }
                    }
                    tipoDado=0;
                    multNumber=1;
                    break;
                case ',':
                    if((multNumber>1)||(multNumber<-1))
                    {
                        number = tipoDado?JSS_id[number-1]:number;
                        switch(j)
                        {
                            case 0: id[j++]=number;id[j++]=1;id[j++]=id[0]+1;break;
                            case 1: j++;id[j++]=number;break;
                            case 2: id[j++]=number;break;
                            case 7: id[j++]=number;id[j++]=1;id[j++]=id[7]+1;break;
                            case 8: j++;id[j++]=number;break;
                            case 9: id[j++]=number;
                        }
                    }
                    else
                    {
                        switch(j)
                        {
                            case 0: j=3;break;
                            case 1: j=3;break;
                            case 7: j=10;break;
                            case 8: j=10;break;
                        }
                    }
                    tipoDado=0;
                    multNumber=1;
                    break;
                case '.': tipoDado=1;numberReal=(qreal) number;number>=0?multNumber=0.1:multNumber=-0.1;break;
                default:
                    if((multNumber==1)||(multNumber==-1)){numberReal=0;number=0;}//Zera na leitura do primeiro caracter de um numero.
                    if(tipoDado==1)//Numero Real
                    {
                        if(*ch>='0'&&*ch<='9') {numberReal += ((qreal) (*ch-'0'))*multNumber;multNumber*=multNumber<1?0.1:10;}
                        else multNumber=1;
                    }
                    else //Numero Inteiro
                    {
                        if(*ch>='0'&&*ch<='9') {number += ((qint32) (*ch-'0'))*multNumber;multNumber*=10;}
                        else multNumber=1;
                    }
           }
           ch++;
        }
        if((j==6)||(j==7)) {id[7]=0;id[8]=0;id[9]=0;if(multNumber!=1) rd[0]=tipoDado==3?JSS_rd[number-1]:numberReal;} //Faz uma marca de que n???o tem os dados da outra matriz.
        if(isExp) rd[1]=tipoDado==3?JSS_rd[number-1]:numberReal;
        multNumber=1;
    }
};
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class JMathVar : public QVector<T>
{
 public:
    /*C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..*/
    volatile qint32 JMV_numLinhas;
    volatile qint32 JMV_numColunas;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //M???todos B???sicos
    JMathVar():QVector<T>(1) {JMV_numLinhas=1;JMV_numColunas=1;}
    JMathVar(const JMathVar<T> &v): QVector<T>((QVector<T> ) v) {JMV_numLinhas  = v.numLinhas();JMV_numColunas = v.numColunas();}
    JMathVar(const qint32 &qtdeElementos,const T &valor = T(0.0f)):QVector<T>(qtdeElementos,valor) {JMV_numLinhas  = 1;JMV_numColunas = qtdeElementos;}
    JMathVar(const qint32 &qtdeLinha,const qint32 &qtdeColuna,const T &valor = T(0.0f)):QVector<T>((qtdeLinha*qtdeColuna),valor) {JMV_numLinhas  = qtdeLinha;JMV_numColunas = qtdeColuna;}
    void clear();
    bool setNumLinhas(const qint32 &numLinhas);
    bool setNumColunas(const qint32 &numColunas);
    const qint32 numLinhas() const {return((const qint32) JMV_numLinhas); }
    const qint32 numColunas() const {return((const qint32) JMV_numColunas);}
    void append(const QList<T> &list, const qint32 &linha, const qint32 &coluna);
    void append(const char &ch, const JMathVar<T> &mat);
    void append(const char &ch,const T &vlr);
    void prepend(const char &ch, const T &vlr);
    void remove(const char &ch, const qint32 &posElementos) {if(ch=='C'||ch=='c') JMV_numColunas--;else JMV_numLinhas--;((QVector<T> *) this)->remove(posElementos);}
    const bool fill(const T &valor, const qint32 &qtdeLinha, const qint32 &qtdeColuna); //Tem que fazer o fill para n???o perder a posi??????o quando insere linhas do que ja existe
    const bool fill(const T &valor, const qint32 &qtdeElementos) {return fill(valor,1,qtdeElementos);}
    void resize(const qint32 &qtdeLinha ,const qint32 &qtdeColuna); //Tem que fazer o resize para n???o perder a posi??????o quando insere linhas do que ja existe
    inline void resize(const qint32 &qtdeElementos) {resize(1,qtdeElementos);}
    inline T& operator()(const qint32 &posLinha, const qint32 &posColuna) {return (*this)[(JMV_numLinhas*posColuna)+posLinha];}
    inline T& operator()(const qint32 &posElementos) { return (*this)(0,posElementos);}
    inline const T& at(const qint32 &posLinha, const qint32 &posColuna) const {return ((QVector<T> *) this)->at((JMV_numLinhas*posColuna)+posLinha);}
    inline const T& at(const qint32 &posElementos) const { return ((QVector<T> *) this)->at(posElementos);}
    const bool fill(const qint32 id[15]);
    void swap(const qint32 id[15]);
    const bool copy(const JMathVar<T> &mt,const qint32 id[15],const qreal rd[2]);
    JMathVar<T> opMat(const JMathVar<T> &mt,const qint32 id[15],const qreal rd[2]) const;
    JMathVar<T> Inv() const; //Este M???todo vai no futuro chamar o m???todo opMat
    JMathVar<T> Trans() const;  //Este M???todo vai no futuro chamar o m???todo opMat
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Fazer uma unica fun??????o do tipo operador para vai receber um vetor de caracter informando o tipo
    //Dai ela vai fazer todo o resto numa unica fun??????o usando o recurso va_arg onde posso passar varios tipo de dados
    //Ver http://www.cplusplus.com/reference/cstdarg/va_start/
    //
    //const JMathVar<T> operator()(const char ch); //Faz todas as opera??????es com quantas matrizes desejar
    //const JMathVar<T> operator()(va_arg d[]);//Faz apenas opera??????es de duas em duas matrizes
    //
    // TIPO(:,:) = %M1(:,%2) + %M3(%4,%5)*%f6*%M7(:,:)
    //
    //Onde os TIPOs podem ser:
    //
    //  F[Fill]    = Preenche a matriz this conforme uma m???trica.
    //  S[Swap]    = Troca elementos da matriz this podendo fazer opera??????es com eles na troca.
    //  A[Append]  = Adiciona elementos na matriz this deslocando a matriz em uma dada regi???o.
    //  E[Extract] = Extrai o conteudo obedecendo uma metrica da matriz this e a retorna.
    //  C[Copy]    = Copia para a matriz this os dados de outra ou dela mesma podendo aumetar a matriz caso seja necess???rio.
    //  R[Repalce] = Copia para a matriz this os dados de outra ou dela mesma e apaga na matrix this tudo o que tiver a mais pro final do que foi copiado.
    //  D[Delete]  = Deleta removendo uma dada regi???o conforme uma metrica
    //  sem tipo   = C[Copy]
    //
    //Se n???o tiver o igual retorna uma matriz qualquer com o igual retorna a matriz que chamou o m???todo.
    //A matriz %M0 e considerada a matriz da chamada
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //M???todos com parsing que ser???o removidos no futuro
    JMathVar<T> operator()(const JStrSet &parsing) const; //Extrai o conteudo obedecendo uma metrica de uma matriz
    JMathVar<T> operator()(const JMathVar<T> &mt, const JStrSet &parsing) const;
    const bool copy(const JStrSet &parsing);
    const bool copy(const JMathVar<T> &mt, const JStrSet &parsing);
    const bool replace(const JMathVar<T> &mt, const JStrSet &parsing);
    const bool fill(const JStrSet &parsing);
    void swap(const JStrSet &parsing);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //M???todos gerais  
    T Norma(const qint32 &tipo) const;
    T NormaInf() const;
    T MaiorElem(const JStrSet &parsing,qint32 &posLinha,qint32 &posColuna) const;
    T MaiorElemDiagPrin() const;
    qreal Det();
    JMathVar<T> SistemaLinear(const JMathVar<T> &vet1,bool &isOK=false);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Operadores Matem???ticos.
    JMathVar<T>&operator=(const T &valor);
    //JMathVar<T>&operator+=(const T &valor);
    //JMathVar<T>&operator-=(const T &valor);
    //JMathVar<T>&operator*=(const T &valor);
    //JMathVar<T>&operator/=(const T &valor);
    //JMathVar<T>&operator^(const T &valor);
    JMathVar<T>&operator=(const JMathVar<T> &mt);
    //JMathVar<T>&operator+=(const JMathVar<T> &mt);
    //JMathVar<T>&operator-=(const JMathVar<T> &mt);
    JMathVar<T> operator*(const JMathVar<T> &mt) const {qreal rd[2]={1.,1.};qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};return opMat(mt,id,rd);}
    JMathVar<T> operator/(const JMathVar<T> &mt) const {qreal rd[2]={1.,1.};qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};return opMat(mt,id,rd);}
    JMathVar<T> operator+(const JMathVar<T> &mt) const {qreal rd[2]={1.,1.};qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};return opMat(mt,id,rd);}
    JMathVar<T> operator-(const JMathVar<T> &mt) const {qreal rd[2]={1.,1.};qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};return opMat(mt,id,rd);}
};
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//M???todos B???sicos
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void JMathVar<T>::clear()
{
    JMV_numLinhas = 1;
    JMV_numColunas = 1;
    ((QVector<T> *) this)->clear();
    ((QVector<T> *) this)->resize(1);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline bool JMathVar<T>::setNumLinhas(const qint32 &numLinhas)
{
    if(numLinhas&&!(this->size()%numLinhas))
    {
        JMV_numLinhas = numLinhas;
        JMV_numColunas = (this->size()/numLinhas);
        return true;
    }
    else return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline bool JMathVar<T>::setNumColunas(const qint32 &numColunas)
{
    if(numColunas&&!(this->size()%numColunas))
    {
        JMV_numColunas = numColunas;
        JMV_numLinhas = (this->size()/numColunas);
        return true;
    }
    else return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline const bool JMathVar<T>::fill(const T &valor, const qint32 &qtdeLinha, const qint32 &qtdeColuna)
{
    JMV_numLinhas  = qtdeLinha;
    JMV_numColunas = qtdeColuna;
    ((QVector<T> *) this)->fill(valor,qtdeLinha*qtdeColuna);
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void JMathVar<T>::resize(const qint32 &qtdeLinha,const qint32 &qtdeColuna)
{
    JMV_numLinhas  = qtdeLinha;
    JMV_numColunas = qtdeColuna;
    ((QVector<T> *) this)->resize(qtdeLinha*qtdeColuna);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
const bool JMathVar<T>::fill(const qint32 id[15])
{
    const qint32 sizeAnt = this->size();
    qint32 iL,jL,iP,jP,valorPonto,
           pos,posEscrita,posDesejadaMenor,posDesejadaMaior;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 qtdadeElem = id[2]-JMV_numLinhas,
                 qtdadeNVetor = id[5]-JMV_numColunas;
    //////////////////////////////////////////////////////////////////////////////////////////
    //Se houver a necessidade altera o tamanho da matriz.
    if((id[2]>JMV_numLinhas)&&(id[5]>JMV_numColunas)){((QVector<T> *) this)->resize(id[2]*id[5]);JMV_numLinhas=id[2];JMV_numColunas=id[5];}
    else if(id[5]>JMV_numColunas){((QVector<T> *) this)->resize(JMV_numLinhas*id[5]);JMV_numColunas=id[5];}
    else if(id[2]>JMV_numLinhas){((QVector<T> *) this)->resize(id[2]*JMV_numColunas);JMV_numLinhas=id[2];}
    //////////////////////////////////////////////////////////////////////////////////////////
    posDesejadaMenor = this->size() - (qtdadeNVetor*JMV_numLinhas) - qtdadeElem;
    posDesejadaMaior = this->size() - (qtdadeNVetor*JMV_numLinhas) - 1;
    //////////////////////////////////////////////////////////////////////////////////////////
    const T *bE = this->begin() + sizeAnt,//Final do vetor Antigo de leitura
            copy(0.0f);
    T *vi = this->end();//Final do vetor Atual
    //////////////////////////////////////////////////////////////////////////////////////////
    pos = this->size() -1;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 iniiP = id[10]+(((id[12]-id[10])/id[11])-1)*id[11];iP=iniiP;
    const qint32 inijP = id[7]+(((id[9]-id[7])/id[8])-1)*id[8];jP=inijP;
    valorPonto = ((iP*JMV_numLinhas)+jP);//Valor do ponto.
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 iniiL = id[3]+(((id[5]-id[3])/id[4])-1)*id[4];iL = iniiL;
    const qint32 inijL = id[0]+(((id[2]-id[0])/id[1])-1)*id[1];jL = inijL;
    posEscrita = ((iL*JMV_numLinhas)+jL);
    //////////////////////////////////////////////////////////////////////////////////////////
    while(pos>=0)
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        if(((pos >= posDesejadaMenor)&&(pos <= posDesejadaMaior))||(pos >= (this->size() - qtdadeNVetor*JMV_numLinhas)))
            *--vi = copy;
        else *--vi = *--bE;
        if((pos == posDesejadaMenor)&&(pos <= posDesejadaMaior))
        {
            posDesejadaMenor -= JMV_numLinhas,
            posDesejadaMaior -= JMV_numLinhas;
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        if((pos == posEscrita)&&(iL>=id[3]))
        {
            *vi = valorPonto;
            if(id[13])
            {
                iP -= id[11];
                if(iP<id[10]){jP -= id[8];iP = iniiP;}
            }
            else
            {
                jP -= id[8];
                if(jP<id[7]) {iP -= id[11];jP = inijP;}
            }
            valorPonto = ((iP*this->JMV_numLinhas)+jP);
            jL -= id[1];
            if(jL<id[0]) {iL -= id[4];jL = inijL;}
            posEscrita = ((iL*this->JMV_numLinhas)+jL);
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        if(((*vi)!=(*vi))||((*vi)>=1e199)||((*vi)<=-1e199))
            return false;
        pos--;
        //////////////////////////////////////////////////////////////////////////////////////////
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>/*C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..*/
void JMathVar<T>::swap(const qint32 id[15])
{
    qint32 pos1,pos2,i1,j1,i2,j2;
    //////////////////////////////////////////////////////////////////////////////////////////
    T *i = this->begin(),aux;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 inii1 = id[3]+(((id[5]-id[3])/id[4])-1)*id[4]; i1 = inii1;
    const qint32 inij1 = id[0]+(((id[2]-id[0])/id[1])-1)*id[1]; j1 = inij1;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 inii2 = id[10]+(((id[12]-id[10])/id[11])-1)*id[11]; i2 = inii2;
    const qint32 inij2 = id[7]+(((id[9]-id[7])/id[8])-1)*id[8]; j2 = inij2;
    //////////////////////////////////////////////////////////////////////////////////////////
    while(id[6]?j1>=id[0]:i1>=id[3])
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        pos1 = ((i1*this->JMV_numLinhas)+j1);
        if(id[6])
        {
            i1 -= id[4];
            if(i1<id[3]){j1 -= id[1];i1 = inii1;}
        }
        else
        {
            j1 -= id[1];
            if(j1<id[0]) {i1 -= id[4];j1 = inij1;}
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        pos2 = ((i2*this->JMV_numLinhas)+j2);
        if(id[13])
        {
            i2 -= id[11];
            if(i2<id[10]){j2 -= id[8];i2 = inii2;}
        }
        else
        {
            j2 -= id[8];
            if(j2<id[7]){i2 -= id[11];j2 = inij2;}
        }
        //////////////////////////////////////////////////////////////////////////////////////////
        aux       = *(i+pos1);
        *(i+pos1) = *(i+pos2);
        *(i+pos2) = aux;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
}
//////////////////////////////////////////////////////////////////////////////////////////
//+=3*(ch,ch)^exp onde ch = ([x:y:z] ou [x:y] ou :)
template<typename T>/*C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..*/
const bool JMathVar<T>::copy(const JMathVar<T> &mt,const qint32 id[15],const qreal rd[2])
{
    bool isOk = true;
    const qint32 sizeAntE = this->size();
    qreal auxReal;
    qint32 pos,posEscrita,posLeitura,iL,jL,iE,jE,posDesejadaMenor,posDesejadaMaior;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 iniiL = id[10]+(((id[12]-id[10])/id[11])-1)*id[11];iL = iniiL;
    const qint32 inijL = id[7]+(((id[9]-id[7])/id[8])-1)*id[8];jL = inijL;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 qtdadeElem = id[2]-JMV_numLinhas,
                 qtdadeNVetor = id[5]-JMV_numColunas;
    //////////////////////////////////////////////////////////////////////////////////////////
    const qint32 iniiE = id[3]+(((id[5]-id[3])/id[4])-1)*id[4];iE = iniiE;
    const qint32 inijE = id[0]+(((id[2]-id[0])/id[1])-1)*id[1];jE = inijE;
    //////////////////////////////////////////////////////////////////////////////////////////
    const T *bE = this->begin() + sizeAntE,//Final do vetor Antigo de leitura
            *bL = mt.begin(),//Inicio do vetor Antigo de escrita
            copy(0.0f);
    T *i = this->end();//Final do vetor Atual
    //////////////////////////////////////////////////////////////////////////////////////////
    pos = this->size() -1;
    //////////////////////////////////////////////////////////////////////////////////////////
    posEscrita = ((iE*this->JMV_numLinhas)+jE);
    posLeitura = ((iL*mt.JMV_numLinhas)+jL);
    //////////////////////////////////////////////////////////////////////////////////////////
    posDesejadaMenor = this->size() - (qtdadeNVetor*JMV_numLinhas) - qtdadeElem;
    posDesejadaMaior = this->size() - (qtdadeNVetor*JMV_numLinhas) - 1;
    //////////////////////////////////////////////////////////////////////////////////////////
    while(pos>=0)
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        if(((pos >= posDesejadaMenor)&&(pos <= posDesejadaMaior))||(pos >= (this->size() - qtdadeNVetor*JMV_numLinhas))) *--i = copy;
        else *--i = *--bE;
        if((pos == posDesejadaMenor)&&(pos <= posDesejadaMaior))
        {
            posDesejadaMenor -= JMV_numLinhas,
            posDesejadaMaior -= JMV_numLinhas;
        }
        //if(iL==id[10])
            //qDebug()<<"Teste";
        if((pos == posEscrita)&&(iE>=id[3])&&((id[13]!=id[6])?(jL>=id[7]):(iL>=id[10])))
        {
            auxReal = rd[0]*qPow(*(bL+posLeitura),rd[1]);
            //////////////////////////////////////////////////////////////////////////////////////////
            jE -= id[1];
            if(jE<id[0]) {iE -= id[4];jE = inijE;}
            posEscrita = ((iE*this->JMV_numLinhas)+jE);
            if(id[13]!=id[6])
            {
                iL -= id[11];
                if(iL<id[10]){jL -= id[8];iL = iniiL;}
            }
            else
            {
                jL -= id[8];
                if(jL<id[7]){iL -= id[11];jL = inijL;}
            }
            posLeitura = ((iL*mt.JMV_numLinhas)+jL);
            //////////////////////////////////////////////////////////////////////////////////////////
            switch(id[14])
            {
                case 0: *i  = auxReal;break;
                case 1: *i += auxReal;break;
                case 2: *i -= auxReal;break;
                case 3: *i *= auxReal;break;
                case 4: *i /= auxReal;break;
            }
            //////////////////////////////////////////////////////////////////////////////////////////
        }
        if(((*i)!=(*i))||((*i)>=1e199)||((*i)<=-1e199)) isOk=false;
        pos--;
        //////////////////////////////////////////////////////////////////////////////////////////
    }
    return isOk;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////Faz as opera??????es de matriz com uma matriz/////////////////////
template<typename T>
JMathVar<T> JMathVar<T>::opMat(const JMathVar<T> &mt,const qint32 id[15],const qreal rd[2]) const
{
    JMathVar<T> result;
    qint32 pos;
    T const *mc = mt.end(); //Final do vetor de leitura
    T *mi,*mb;
    //Considera uma das matrizes como um ponto.
    if((mt.JMV_numLinhas==1)&&(mt.JMV_numColunas==1))//Se o valor do dividendo for uma constante.
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        const T *vi = this->end(),//Final do vetor Atual
                valor = mt.at(0);
        result.resize(numLinhas(),numColunas());
        T *re = result.end();
        pos = this->size() -1;
        //////////////////////////////////////////////////////////////////////////////////////////
        switch(id[14])
        {
            case 0: while(pos-->=0) *--re = rd[0]*qPow(valor,rd[1]); break;
            case 1: while(pos-->=0) *--re = rd[0]*qPow((*--vi),rd[1]) + valor; break;
            case 2: while(pos-->=0) *--re = rd[0]*qPow((*--vi),rd[1]) - valor; break;
            case 3: while(pos-->=0) *--re = rd[0]*qPow((*--vi),rd[1]) * valor; break;
            case 4: while(pos-->=0) *--re = rd[0]*qPow((*--vi),rd[1]) / valor; break;
            case 5: while(pos-->=0) *--re = qPow(rd[0]*qPow((*--vi),rd[1]),valor);
        }
        //////////////////////////////////////////////////////////////////////////////////////////
    }
    else //Considera as matrizes como vetores ou matrizes mesmo.
    {
        result = *this;
        mi = result.end(); //Final do vetor de escrita
        mb = result.begin();//Inicio do vetor de escrita
        switch(id[14]) //Falta fazer quando apenas para uma faixa;
        {
            case 0: while(mb<mi) *--mi  = rd[0]*qPow(*--mc,rd[1]); break; //Para id[14]= 0{=}
            case 1: while(mb<mi) *--mi += rd[0]*qPow(*--mc,rd[1]); break; //Para id[14]= 1{+=}
            case 2: while(mb<mi) *--mi -= rd[0]*qPow(*--mc,rd[1]); break; //Para id[14]= 2{-=}
            case 3: //Para id2[6]= 3{*} <M1, M2> = <M1,N> * <N, M2>
                qint32 iniLinhas,stepLinhas,fimLinhas,iniColunas,stepColunas,fimColunas,iniAux,stepAux,fimAux;
                if(id[6]) {iniLinhas = id[3];stepLinhas = id[4];fimLinhas = id[5];iniAux = id[0];stepAux = id[1];fimAux = id[2];}
                else {iniLinhas = id[0];stepLinhas = id[1];fimLinhas = id[2];iniAux = id[3];stepAux = id[4];fimAux = id[5];}
                if(id[13]) {iniColunas  = id[7];stepColunas = id[8];fimColunas  = id[9];}
                else {iniColunas  = id[10];stepColunas = id[11];fimColunas  = id[12];}
                if(fimAux==(id[13]?id[12]:id[9]))
                {
                    T sum;
                    result.resize((fimLinhas-iniLinhas)/stepLinhas,(fimColunas-iniColunas)/stepColunas);
                    for (qint32 linha = iniLinhas; linha < fimLinhas;linha+=stepLinhas)
                    {
                        for (qint32 coluna = iniColunas; coluna < fimColunas;coluna+=stepColunas)
                        {
                            sum = 0;
                            for (qint32 aux = iniAux; aux < fimAux;aux+=stepAux)
                                sum += (id[6]?this->at(aux,linha):this->at(linha,aux)) * (id[13]?mt.at(coluna,aux):mt.at(aux,coluna));
                            result(linha-iniLinhas,coluna-iniColunas) = rd[0]*qPow(sum,rd[1]);
                        }
                    }
                }
                else
                {
                    qWarning() << "Erro na operacao de multiplicacao de matrizes: dimensoes incompativeis."
                               << "linhas(A)=" << ((fimLinhas-iniLinhas)/stepLinhas)
                               << "colunas(A)=" << ((fimAux-iniAux)/stepAux)
                               << "linhas(B)=" << ((fimAux-iniAux)/stepAux)
                               << "colunas(B)=" << ((fimColunas-iniColunas)/stepColunas);
                    // Evita encerramento abrupto da aplicacao; retorna matriz nula com dimensao esperada.
                    result.fill(0, (fimLinhas-iniLinhas)/stepLinhas, (fimColunas-iniColunas)/stepColunas);
                    return result;
                }
                //break;
            //case 4: //Divis???o: <M1, M2> = <M1,N> * inv(<M2, N>)
                //break;
            //case 5: //Exp valendo apenas quando for um ponto elevado a outro ponto
            //case 6: //Inv
        }
    }
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//M???todos com parsing
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T> //fazer inversa e transpota pertencendo aos operadores
JMathVar<T> JMathVar<T>::operator()(const JStrSet &parsing) const
{
    qreal rd[2]={1.,1.};
    //lInicial:Incremento:lFinal,
    qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,numLinhas(),0,1,numColunas(),0,0},
           pos,posLeitura,iL,jL;
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(id,rd);
    if(id[7]==0&&id[8]==0&&id[9]==0)
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        if(id[2]>numLinhas()) id[2]=numLinhas();//So vai ate o maximo da matriz
        if(id[5]>numColunas()) id[5]=numColunas();
        //////////////////////////////////////////////////////////////////////////////////////////
        JMathVar<T> result(id[6]?((id[5]-id[3])/id[4]):((id[2]-id[0])/id[1]),id[6]?((id[2]-id[0])/id[1]):((id[5]-id[3])/id[4]));
        //////////////////////////////////////////////////////////////////////////////////////////
        const T *bL = this->begin();//Inicio do vetor de leitura
        T *i = result.end();        //Final do vetor de escrita
        //////////////////////////////////////////////////////////////////////////////////////////
        pos = this->size() -1;
        //////////////////////////////////////////////////////////////////////////////////////////
        const qint32 iniiL = id[3]+(((id[5]-id[3])/id[4])-1)*id[4];iL = iniiL; //colunas
        const qint32 inijL = id[0]+(((id[2]-id[0])/id[1])-1)*id[1];jL = inijL; //Linhas
        posLeitura = ((iL*this->JMV_numLinhas)+jL);
        //////////////////////////////////////////////////////////////////////////////////////////
        while((pos>=0)&&(id[6]?jL>=id[0]:iL>=id[3]))
        {
            if(pos == posLeitura)
            {
                *--i = rd[0]*qPow(*(bL+posLeitura),rd[1]);
                //////////////////////////////////////////////////////////////////////////////////////////
                if(id[6])
                {
                    iL -= id[4];
                    if(iL<id[3]){jL -= id[1];iL = iniiL;}
                }
                else
                {
                    jL -= id[1];
                    if(jL<id[0]){iL -= id[4];jL = inijL;}
                }
                posLeitura = ((iL*this->JMV_numLinhas)+jL);
                //////////////////////////////////////////////////////////////////////////////////////////
            }
            pos--;
            //////////////////////////////////////////////////////////////////////////////////////////
        }
        return result;
    }
    else
    {
        return this->opMat(*this,id,rd);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T> //fazer inversa e transpota pertencendo aos operadores
JMathVar<T> JMathVar<T>::operator()(const JMathVar<T> &mt, const JStrSet &parsing) const
{
    qreal rd[2]={1.,1.};
    qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(id,rd);
    //////////////////////////////////////////////////////////////////////////////////////////
    return this->opMat(mt,id,rd);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
const bool JMathVar<T>::fill(const JStrSet &parsing)
{
    qreal rd[2]={1.,1.};
    qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,numLinhas(),0,1,numColunas(),0,0};
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(id,rd);
    //////////////////////////////////////////////////////////////////////////////////////////
    this->fill(id);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>/*C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..*/
void JMathVar<T>::swap(const JStrSet &parsing)
{
    qreal rd[2]={1.,1.};
    qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,numLinhas(),0,1,numColunas(),0,0};
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(id,rd);
    //////////////////////////////////////////////////////////////////////////////////////////
    if(id[2]>numLinhas()) id[2]=numLinhas();//So vai ate o maximo da matriz
    if(id[5]>numColunas()) id[5]=numColunas();
    //////////////////////////////////////////////////////////////////////////////////////////
    this->swap(id);
}
//////////////////////////////////////////////////////////////////////////////////////////
///
//Existe um erro nesta fun??????o tenho que corrigir quando usa a multiplica??????o.
template<typename T>
const bool JMathVar<T>::copy(const JStrSet &parsing) //Ou faz com ele mesmo ou com uma constante.
{
    qreal rd[2]={1.,1.};
    qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,numLinhas(),0,1,numColunas(),0,0};
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(id,rd);
    //////////////////////////////////////////////////////////////////////////////////////////
    if(id[7]==0&&id[8]==0&&id[9]==0)
    {
        bool isOk = true;
        qint32 posEscrita,iE,jE;
        //////////////////////////////////////////////////////////////////////////////////////////
        if((id[2]>JMV_numLinhas)&&(id[5]>JMV_numColunas)) resize(id[2], id[5]);
        else if(id[2]>JMV_numLinhas) resize(id[2],(const qint32) JMV_numColunas);
        else if(id[5]>JMV_numColunas) resize((const qint32) JMV_numLinhas, id[5]);
        //////////////////////////////////////////////////////////////////////////////////////////
        const qint32 iniiE = id[3]+(((id[5]-id[3])/id[4])-1)*id[4];iE = iniiE;
        const qint32 inijE = id[0]+(((id[2]-id[0])/id[1])-1)*id[1];jE = inijE;
        //////////////////////////////////////////////////////////////////////////////////////////
        T *i = this->begin();//Final do vetor Atual
        //////////////////////////////////////////////////////////////////////////////////////////
        while(iE>=id[3])
        {
            //////////////////////////////////////////////////////////////////////////////////////////
            posEscrita = ((iE*JMV_numLinhas)+jE);
            jE -= id[1];
            if(jE<id[0]) {iE -= id[4];jE = inijE;}
            //////////////////////////////////////////////////////////////////////////////////////////
            if(!id[14]) *(i+posEscrita)  = rd[0];
            else *(i+posEscrita)  = rd[0]*qPow(*(i+posEscrita),rd[1]);
            /*switch(id[14])
            {
                case 0: *(i+posEscrita)  = rd[0];break;
                //case 1: *(i+posEscrita) += rd[0];break;
                //case 2: *(i+posEscrita) -= rd[0];break;
                case 3: *(i+posEscrita) *= rd[0];break;
                case 4: *(i+posEscrita) /= rd[0];break;
                case 5: *(i+posEscrita)  = rd[0]*qPow(*(i+posEscrita),rd[1]);break;
            }*/
            //////////////////////////////////////////////////////////////////////////////////////////
            if(((*(i+posEscrita))!=(*(i+posEscrita)))||((*(i+posEscrita))>=1e199)||((*(i+posEscrita))<=-1e199)) isOk = false;
            //////////////////////////////////////////////////////////////////////////////////////////
        }
        return isOk;
    }
    else
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        const qint32 qtdadeLinhaThis  = id[6]?((id[5]-id[3])/id[4]):((id[2]-id[0])/id[1]),
                     qtdadeColunaThis = id[6]?((id[2]-id[0])/id[1]):((id[5]-id[3])/id[4]),
                     qtdadeLinhaMt    = id[13]?((id[12]-id[10])/id[11]):((id[9]-id[7])/id[8]),
                     qtdadeColunaMt   = id[13]?((id[9]-id[7])/id[8]):((id[12]-id[10])/id[11]),
                     minLinhaE = id[6]?id[3]+qtdadeColunaMt*id[4]:id[0]+qtdadeLinhaMt*id[1],
                     minColunaE = id[6]?id[0]+qtdadeLinhaMt*id[1]:id[3]+qtdadeColunaMt*id[4];
        //////////////////////////////////////////////////////////////////////////////////////////
        //Se houver a necessidade altera o tamanho da matriz.
        if(((qtdadeLinhaThis != qtdadeLinhaMt)&&(qtdadeColunaThis != qtdadeColunaMt))||((JMV_numLinhas<minLinhaE)&&(JMV_numColunas<minColunaE))) {this->resize(minLinhaE,minColunaE);id[2]=minLinhaE;id[5]=minColunaE;}
        else if((qtdadeLinhaThis != qtdadeLinhaMt)||(JMV_numLinhas<minLinhaE)) {this->resize(minLinhaE,(const qint32) JMV_numColunas);id[2]=minLinhaE;}
        else if((qtdadeColunaThis != qtdadeColunaMt)||(JMV_numColunas<minColunaE)) {this->resize((const qint32) JMV_numLinhas,minColunaE);id[5]=minColunaE;}
        //////////////////////////////////////////////////////////////////////////////////////////
        return this->copy(*this,id,rd);
    }

}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
const bool JMathVar<T>::copy(const JMathVar<T> &mt, const JStrSet &parsing)
{
    if((parsing=="(:,:)=(:,:)")||(parsing=="(:)=(:)"))
    {
        this->resize(mt.numLinhas(),mt.numColunas());
        qCopy(mt.begin(),mt.end(),this->begin());
        return true;
    }
    else
    {
        qreal rd[2]={1.,1.};
        qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};
        //////////////////////////////////////////////////////////////////////////////////////////
        parsing.extract(id,rd);
        //////////////////////////////////////////////////////////////////////////////////////////
        const qint32 qtdadeLinhaMt    = id[13]?((id[12]-id[10])/id[11]):((id[9]-id[7])/id[8]),
                     qtdadeColunaMt   = id[13]?((id[9]-id[7])/id[8]):((id[12]-id[10])/id[11]),
                     minLinhaE = id[6]?id[3]+qtdadeColunaMt*id[4]:id[0]+qtdadeLinhaMt*id[1],
                     minColunaE = id[6]?id[0]+qtdadeLinhaMt*id[1]:id[3]+qtdadeColunaMt*id[4];
        //////////////////////////////////////////////////////////////////////////////////////////
        //Se houver a necessidade altera o tamanho da matriz.
        if((JMV_numLinhas<minLinhaE)&&(JMV_numColunas<minColunaE)) {resize(minLinhaE,minColunaE);id[2]=minLinhaE;id[5]=minColunaE;}
        else if(JMV_numLinhas<minLinhaE) {resize(minLinhaE,(const qint32) JMV_numColunas);id[2]=minLinhaE;}
        else if(JMV_numColunas<minColunaE) {resize((const qint32) JMV_numLinhas,minColunaE);id[5]=minColunaE;}
        //////////////////////////////////////////////////////////////////////////////////////////
        return this->copy(mt,id,rd);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
const bool JMathVar<T>::replace(const JMathVar<T> &mt, const JStrSet &parsing)
{
    if((parsing=="(:,:)=(:,:)")||(parsing=="(:)=(:)"))
    {
        this->resize(mt.numLinhas(),mt.numColunas());
        qCopy(mt.begin(),mt.end(),this->begin());
        return true;
    }
    else
    {
        qreal rd[2]={1.,1.};
        qint32 id[15]={0,1,numLinhas(),0,1,numColunas(),0,0,1,mt.numLinhas(),0,1,mt.numColunas(),0,0};
        //////////////////////////////////////////////////////////////////////////////////////////
        parsing.extract(id,rd);
        //////////////////////////////////////////////////////////////////////////////////////////
        const qint32 qtdadeLinhaThis  = id[6]?((id[5]-id[3])/id[4]):((id[2]-id[0])/id[1]),
                     qtdadeColunaThis = id[6]?((id[2]-id[0])/id[1]):((id[5]-id[3])/id[4]),
                     qtdadeLinhaMt    = id[13]?((id[12]-id[10])/id[11]):((id[9]-id[7])/id[8]),
                     qtdadeColunaMt   = id[13]?((id[9]-id[7])/id[8]):((id[12]-id[10])/id[11]),
                     minLinhaE = id[6]?id[3]+qtdadeColunaMt*id[4]:id[0]+qtdadeLinhaMt*id[1],
                     minColunaE = id[6]?id[0]+qtdadeLinhaMt*id[1]:id[3]+qtdadeColunaMt*id[4];
        //////////////////////////////////////////////////////////////////////////////////////////
        //Se houver a necessidade altera o tamanho da matriz.
        if(((qtdadeLinhaThis < qtdadeLinhaMt)&&(qtdadeColunaThis < qtdadeColunaMt))||((JMV_numLinhas<minLinhaE)&&(JMV_numColunas<minColunaE))) {resize(minLinhaE,minColunaE);id[2]=minLinhaE;id[5]=minColunaE;}
        else if((qtdadeLinhaThis != qtdadeLinhaMt)||(JMV_numLinhas<minLinhaE)) {resize(minLinhaE,(const qint32) JMV_numColunas);id[2]=minLinhaE;}
        else if((qtdadeColunaThis != qtdadeColunaMt)||(JMV_numColunas<minColunaE)) {resize((const qint32) JMV_numLinhas,minColunaE);id[5]=minColunaE;}
        //////////////////////////////////////////////////////////////////////////////////////////
        return this->copy(mt,id,rd);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void JMathVar<T>::append(const QList<T> &list, const qint32 &linha, const qint32 &coluna)
{
    if((JMV_numLinhas == linha)||(JMV_numLinhas==1))
    {
        if(JMV_numColunas==1) this->remove('C',0);
        JMV_numColunas += coluna;
        JMV_numLinhas = linha;
        *((QVector<T> *) this) += list.toVector();
    }
    else
        qDebug()<< "JMathVar<T>::append: O tamanho das Linhas n???o s???o iguais";
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void JMathVar<T>::append(const char &ch, const JMathVar<T> &mat)
{//Fazer se ??? para linha ou coluna
    (void)ch;
    if(JMV_numLinhas == mat.numLinhas() || this->isEmpty() ||JMV_numLinhas==1)
    {
        if((JMV_numColunas==1)&&(!at(0))) this->remove('C',0);
        JMV_numColunas += mat.numColunas();
        JMV_numLinhas  = mat.numLinhas();
        (*this) += (QVector<T> ) mat;
    }
    else qDebug()<< "JMathVar<T>::append(const JMathVar<T> &mat): O tamanho das Linhas n???o s???o iguais";
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void JMathVar<T>::append(const char &ch, const T &vlr)
{
    if(ch=='C'||ch=='c') JMV_numColunas++;
    else JMV_numLinhas++;
    (*this) += vlr;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void JMathVar<T>::prepend(const char &ch,const T &vlr)
{
    if(ch=='C'||ch=='c') JMV_numColunas++;
    else JMV_numLinhas++;
    ((QVector<T> *) this)->prepend(vlr);
}
//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////Operadores de Matrizes/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
JMathVar<T> &JMathVar<T>::operator=(const T &v)
{
    this->resize(1,1);
    *this->begin() = v;
    return (*this);
}
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
JMathVar<T> &JMathVar<T>::operator=(const JMathVar<T> &v)
{
    this->resize(v.numLinhas(),v.numColunas());
    T *i = this->end();  //Final do vetor
    T *b = this->begin();//Inicio do vetor
    T const *c = v.begin();
    while(b<i) {*b++ = *c++;}
    return (*this);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
JMathVar<T> JMathVar<T>::Trans() const
{
    JMathVar<T> result(numColunas(),numLinhas());
    for (qint32 linha = 0; linha < numLinhas(); linha++)
        for (qint32 coluna = 0; coluna < numColunas(); coluna++)
            result(coluna,linha) = this->at(linha,coluna);
    return result;
}//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T JMathVar<T>::Norma(const qint32 &tipo) const
{
    //////////////////////////////////////////////////////////////////////////////////////////
    const T *vi = this->end();//Final do vetor Atual
    T norma(0.0f);
    qint32 pos = this->size() -1;
    //////////////////////////////////////////////////////////////////////////////////////////
    while(pos>=0)
    {
        norma += qPow(fabs(*--vi),tipo);
        pos--;
        //////////////////////////////////////////////////////////////////////////////////////////
    }
    return qPow(norma,((qreal) 1)/tipo);
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T JMathVar<T>::NormaInf()  const
{
    T normaInf(0.0f),somaL;
    // Calcula a norma infinita
    for (qint32 i = 0; i < numLinhas(); i++)
    {
            somaL = T(0.0f);
            for (qint32 j = 0; j < numColunas(); j++) somaL += fabs(this->at(i,j));
            if (somaL > normaInf) normaInf = somaL;
    }
    return normaInf;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T JMathVar<T>::MaiorElem(const JStrSet &parsing,qint32 &posLinha,qint32 &posColuna) const
{
    qreal rdL[2]={1.,1.};
    qint32 linha,coluna,idL[15]={0,1,this->numLinhas(),0,1,this->numColunas(),0,0,1,this->numLinhas(),0,1,this->numColunas(),0,0};
    //////////////////////////////////////////////////////////////////////////////////////////
    parsing.extract(idL,rdL);
    //////////////////////////////////////////////////////////////////////////////////////////
    posLinha=idL[0];
    posColuna=idL[3];
    T maiorElem = this->at(posLinha,posColuna);
    //////////////////////////////////////////////////////////////////////////////////////////
    for(linha = posLinha; linha < idL[2];linha+=idL[1])
    {
        for(coluna = posColuna; coluna < idL[5];coluna+=idL[4])
        {
            if(this->at(linha,coluna)>maiorElem) {maiorElem=this->at(linha,coluna);posLinha=linha;posColuna=coluna;}
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    return maiorElem;
}
//////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
T JMathVar<T>::MaiorElemDiagPrin()  const
{
    T maxElem(this->at(0,0));
    qint32 i;

    /* percorre os elementos da diagonal buscando o maior */
    for (i = 0; i < numLinhas(); i++)
    {
        if(i < numColunas())
        {
            if (this->at(i,i) > maxElem)
                maxElem = this->at(i,i);
        }
        else return maxElem;
    }
    return maxElem;
}
//////////////////////////////////////////////////////////////////////////////////////////
//    a00 a01 a02 | a00 a01   Det = a00*a11*a22 - a00*a12*a21
//    a10 a11 a12 | a10 a11       + a01*a12*a20 - a01*a10*a22
//    a20 a21 a22 | a20 a21       + a02*a10*a21 - a02*a11*a20
template <typename T>
qreal JMathVar<T>::Det()
{
    const qint32 ordem = numLinhas();
    qint32 i,j;
    qreal determ,fMais,fMenus;

    for (determ=0.0,i=0;i < ordem; i++)
    {
        for (j=0,fMais=1.0,fMenus=1.0;j < ordem; j++)
        {
            fMais *= this->at(j,(i+j)%ordem);
            fMenus *= this->at(j,(i-j)<0?ordem+(i-j):(i-j));
        }
        determ += fMais-fMenus;
    }
    return determ;
}
//////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
JMathVar<T> JMathVar<T>::Inv() const
{
    const qint32 ordem = numLinhas();
    JMathVar<T> inversa(ordem,ordem,0.0);
    if((ordem <= 0) || (ordem != numColunas())) return inversa;

    JMathVar<T> a(*this);
    for (qint32 i = 0; i < ordem; ++i) inversa(i,i) = 1.0;

    for (qint32 i = 0; i < ordem; ++i)
    {
        qint32 pivoLinha = i;
        qreal maxAbs = qFabs((qreal)a.item(i,i));
        for (qint32 r = i + 1; r < ordem; ++r)
        {
            const qreal valor = qFabs((qreal)a.item(r,i));
            if (valor > maxAbs) { maxAbs = valor; pivoLinha = r; }
        }

        if (maxAbs <= 1e-15) return JMathVar<T>(ordem,ordem,0.0);

        if (pivoLinha != i)
        {
            for (qint32 c = 0; c < ordem; ++c)
            {
                qSwap(a(i,c), a(pivoLinha,c));
                qSwap(inversa(i,c), inversa(pivoLinha,c));
            }
        }

        const qreal pivo = (qreal)a.item(i,i);
        for (qint32 c = 0; c < ordem; ++c)
        {
            a(i,c) = (qreal)a.item(i,c) / pivo;
            inversa(i,c) = (qreal)inversa.item(i,c) / pivo;
        }

        for (qint32 r = 0; r < ordem; ++r)
        {
            if (r == i) continue;
            const qreal fator = (qreal)a.item(r,i);
            if (fator == 0.0) continue;
            for (qint32 c = 0; c < ordem; ++c)
            {
                a(r,c) = (qreal)a.item(r,c) - fator * (qreal)a.item(i,c);
                inversa(r,c) = (qreal)inversa.item(r,c) - fator * (qreal)inversa.item(i,c);
            }
        }
    }

    return inversa;
}//////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////Calcula o sistema linear//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//     Resolve sistemas de equa??????es lineares do tipo: [this].{Solu??????o} = {vet1}         //
//     this    - [Matriz] matriz sim???trica.                                             //
//     vet1    - {vetor}  termos independentes.                                         //
//     Solu??????o - {vetor}  solu??????o do sistema.                                           //
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//C0[L0 L1 L2 ..] C1[L0 L1 L2 ..] C2[L0 L1 L2 ..] C3[L0 L1 L2 ..] C..
template<typename T>
JMathVar<T> JMathVar<T>::SistemaLinear(const JMathVar<T> &vet1,bool &isOK)
{   
    JStrSet jst;
    double termo,m;
    qint32 n=vet1.numLinhas(),i,j,k,l;
    JMathVar<T> matAux(*this),result(n);
    if((n==JMV_numLinhas)&&(n==JMV_numColunas))
    {
        isOK=true;
        matAux.copy(vet1,jst.set("(:,%1)=(:)").argInt(matAux.numColunas()));//Matriz expandida (n,n+1)
        if((JMV_numLinhas==1)&&(JMV_numColunas==1)&&(vet1.JMV_numLinhas==1)&&(vet1.JMV_numColunas==1)) result[0] = vet1.at(0)/this->at(0);
        else
        {
            if(this->Det()>=10e-15)
            {
                //Implementando o M???todo de Gauss transformando em uma matriz triangular superior
                for (k=0;k<n-1;k++) //Varre todas as colunas com exce??????o da ultima coluna da matriz e da coluna expandida
                {
                    for (i=k+1;i<n;i++) //Varre todas a linhas das colunas em avalia??????o
                    {
                        m=(matAux.at(i,k)/matAux.at(k,k)); //Multiplicadores
                        for (j=0;j<n+1;j++) matAux(i,j)=matAux.at(i,j)-(m*matAux.at(k,j)); //Matriz com a transformada em Gauss
                    }
                }
                //Resolvendo o sistema
                for (i=0;i<n;i++)
                {
                    termo=0;
                    l=n-i;
                    for (j=l;j<n;j++) termo=termo+(result.at(j)*matAux.at(n-i-1,j));
                    result[n-i-1]=(matAux.at(n-1-i,n)-termo)/matAux.at(n-i-1,n-i-1);
                    if(result.at(n-i-1)!=result.at(n-i-1))
                    {
                        isOK=false;
                        break;
                    }
                }
            }
            else
                isOK=false;
        }
    }
    else
        isOK=false;
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////
inline qreal sum(JMathVar<qreal> mat)
{
    qreal soma=0;
    for(qint32 i=0;i<mat.size();i++) soma += ((QVector<qreal> ) mat).at(i);
    return soma;
}
//////////////////////////////////////////////////////////////////////////////////////////
inline qreal cov(JMathVar<qreal> mat)
{
    const qreal media=sum(mat)/mat.size();
    qreal soma=0;
    for(qint32 i=0;i<mat.size();i++)
        soma += qPow((((QVector<qreal> ) mat).at(i)-media),2);
    return soma/mat.size();
}
//////////////////////////////////////////////////////////////////////////////////////////
inline bool compara(JMathVar<qreal> mat1,JMathVar<qreal> mat2,qreal valor)
{
    if((mat1.size()==mat2.size())&&(mat1.JMV_numColunas==mat2.JMV_numColunas)&&(mat1.JMV_numLinhas==mat2.JMV_numLinhas))
    {
        qreal *var=mat1.begin(),*var1=mat2.begin();
        for(;var<mat1.end();var++,var1++)
            if(!((*var-*var1)==0?true:(*var-*var1)>0?(*var-*var1)<valor:(*var-*var1)>(-1*valor))) return false;
        return true;
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
//  Macros para compatibilidade - XMatriz ?? apenas um alias de JMathVar
#define XMatriz JMathVar
//////////////////////////////////////////////////////////////////////////////////////////
#endif //JMathVar


