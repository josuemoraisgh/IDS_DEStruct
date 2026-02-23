#include <QDateTime>
#include "srlevmarq.h"
#include "xtipodados.h"
#include "xmatriz.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <QDebug>
#include <limits>
#include <algorithm>
#include <QtScript/QtScript>
// QScriptEngineDebugger not available in Qt 5.15.2
// #ifndef QT_NO_SCRIPTTOOLS
//   #include <QtScript/QScriptEngineDebugger>
// #endif

#include <QtScript/QScriptClassPropertyIterator>
#include <QtScript/QScriptEngine>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Funcoes ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
inline qreal ValidQRealMarq(const qreal &value)
{
    const qreal max = std::numeric_limits<qreal>::max(),
                min = std::numeric_limits<qreal>::min();
    if(!(value==value)) return max;
    else
    {
        if(value >= 0.0f)
        {
            if (value > max) return max;
            else if(value < min) return 0.0f;
            else return value;
        }
        else
        {
            if ((-value) > max) return -max;
            else if ((-value) < min) return 0.0f;
            else return value;
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////INICIO da CLASSE/////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
SRLevMarq::SRLevMarq()
{
    residuos    = new XVetor<qreal>();
    jacobiana   = new XMatriz<qreal>();
    hessiana    = new XMatriz<qreal>();
    quasiGradi  = new XMatriz<qreal>();
    matauxiliar = new XMatriz<qreal>();
}
//////////////////////////////////////////////////////////////////////////////////////////
SRLevMarq::~SRLevMarq()
{
    delete residuos;
    delete jacobiana;
    delete hessiana;
    delete quasiGradi;
    delete matauxiliar;
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcF(qreal &erro, const XVetor<qreal> &fx)
{
    erro = 0.0f;
    //Calcula F(x)
    for (const qreal *fx1 = fx.begin(); fx1 < fx.end(); fx1++) erro += (*fx1) * (*fx1);
    erro /= 2.0;
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcErro(qreal &erro, const XVetor<qreal> &residuos)
{
    erro = 0.0f;
    if(!residuos.empty())
    {
        for(const qreal *res = residuos.begin(); res < residuos.end(); res++) erro += fabs(*res); //residuos.at(i)*residuos.at(i);
        erro /= residuos.size();
    }
    else qDebug() << "Func:LMQ_CalcErro-falhou";
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcGanhoLinear(qreal &res,
                                    const XVetor<qreal>  &hlm,
                                    const XMatriz<qreal> &matQuasiGrad,
                                    const qreal          &paramAmortecimento)
{
    qint32 i;
    res = 0.0f;
    //Calcula o ganho obtido com um modelo linear sendo i o n de regressores
    for (i = 0; i < hlm.size(); i++) res += (hlm.at(i) * (hlm.at(i) * paramAmortecimento - matQuasiGrad.at(i,0)));
    res *= (0.5);
}
//////////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************
*  Funcao: LMQ Resolver sistemas lineares
*
*  Descricao da funcao
*     Resolve sistemas de equacoes lineares do tipo: [a].{x} = {b}
*
*  Parametros
*     matCoeficientes - [a] matriz simetrica.
*     vetor           - {b} termos independentes.
*     vetor           - {x} solucao do sistema.
************************************************************************/
void SRLevMarq::LMQ_ResolverSistLinear(XVetor<qreal>  &vetor,
                                       XMatriz<qreal> &matriz)
{
    qint32 i ,j ,k;
    qreal aux;
    const qint32 ordem=vetor.size();
    /**************Metodo de Gaus Jordan ********************/
    for (i=0;i<ordem;++i)
    {
        aux = matriz.at(i,i);
        for (j=i;j<ordem;++j) matriz(i,j) /= aux;
        vetor[i] /= aux;
        for (k=0;k<ordem;++k)
            if (k!=i)
            {
                for(j=i;j<ordem;++j) matriz(k,j) -= matriz.at(k,i)*matriz.at(i,j);
                vetor[k] -= matriz.at(k,i)*vetor.at(i);
            }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
inline void SRLevMarq::LMQ_ProdVet(qreal *prodVetorial,const qint32 incProdVet,
                                   const qint32 tam,
                                   const qreal *endVet1,const qint32 incVet1,
                                   const qreal *endVet2,const qint32 incVet2
                                   ) const
{
    if((endVet1 != NULL)&&(endVet2 != NULL)&&(prodVetorial != NULL))
    {
        *prodVetorial= (*endVet1)*(*endVet2);
        for(qint32 i=1;i<tam;i++)
        {
            endVet1 += incVet1;
            endVet2 += incVet2;
            prodVetorial += incProdVet;
            *prodVetorial = (*endVet1)*(*endVet2);
        }
    }
    else qDebug() << "Func:LMQ_ProdVet-falhou";
}
//////////////////////////////////////////////////////////////////////////////////////////
inline void SRLevMarq::LMQ_ProdEsc(qreal *prodEscalar, const qint32 tam,
                                   const qreal *endVet1,const qint32 incVet1,
                                   const qreal *endVet2,const qint32 incVet2
                                   ) const
{
    if((endVet1 != NULL)&&(endVet2 != NULL))
    {
        *prodEscalar = (*endVet1)*(*endVet2);
        for(qint32 i=1;i<tam;i++)
        {
            endVet1 += incVet1;
            endVet2 += incVet2;
            *prodEscalar += (*endVet1)*(*endVet2);
        }
    }
    else qDebug() << "Func:LMQ_ProdEsc-falhou";
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcJacobResiduos(XVetor<qreal>* residuos,
                                      XMatriz<qreal>* jacobiana,
                                      const XVetor<qreal> &coefic,
                                      const XVetor<qreal> &GL_fx,
                                      const Cromossomo    &cr)
{
    (void)residuos; (void)jacobiana; (void)coefic; (void)GL_fx; (void)cr;  // Stub function - interface nao compativel
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcHessGrad(XMatriz<qreal>* matHessi,
                                 XMatriz<qreal> *matQuasiGrad,
                                 const XMatriz<qreal> &jacobiana,
                                 const XVetor<qreal>  &residuos,
                                 const Cromossomo     &cr)
{
    (void)matHessi; (void)matQuasiGrad; (void)jacobiana; (void)residuos; (void)cr;  // Stub function - interface nao compativel
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_MinimizarLevMarq(Cromossomo *cr,
                                     const XVetor<qreal> &GL_fx,
                                     const bool &isRefino)
{
    (void)cr; (void)GL_fx; (void)isRefino;  // Stub function - interface incompatible
}
