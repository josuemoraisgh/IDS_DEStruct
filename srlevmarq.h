#ifndef SRLEVMARQ_H
#define SRLEVMARQ_H

#include <QtCore/QObject>
#include <QtScript/QScriptClass>
#include <QtScript/QScriptString>

#include "mtrand.h"
#include "xmatriz.h"
#include "xvetor.h"
#include "xtipodados.h"

class SRLevMarq
{
public:
    MTRand LMQ_RG();
    SRLevMarq();
    ~SRLevMarq();
    XVetor<qreal>  *residuos;
    XMatriz<qreal> *jacobiana,
                   *hessiana,
                   *quasiGradi,
                   *matauxiliar;
    void LMQ_ProdEsc(qreal *prodEscalar, const qint32 tam,
                     const qreal *endVet1,const qint32 incVet1,
                     const qreal *endVet2,const qint32 incVet2
                     ) const;
    void LMQ_ProdVet(qreal *prodVetorial,const qint32 incProdVet,
                     const qint32 tam,
                     const qreal *endVet1,const qint32 incVet1,
                     const qreal *endVet2,const qint32 incVet2
                     ) const;
    void LMQ_CalcF(qreal &erro,
                   const XVetor<qreal> &fx);
    void LMQ_CalcErro(qreal &erro,
                      const XVetor<qreal> &residuosIn);
    void LMQ_CalcGanhoLinear(qreal &res,
                             const XVetor<qreal>  &hlm,
                             const XMatriz<qreal> &matQuasiGrad,
                             const qreal          &paramAmortecimento);
    void LMQ_CalcJacobResiduos(XVetor<qreal> *residuosOut,
                               XMatriz<qreal> *jacobianaOut,
                               const XVetor<qreal> &coefic,
                               const XVetor<qreal> &GL_fx,
                               const Cromossomo    &cr);
    void LMQ_CalcHessGrad(XMatriz<qreal>* matHessiOut,
                          XMatriz<qreal> *matQuasiGradOut,
                          const XMatriz<qreal> &jacobianaIn,
                          const XVetor<qreal> &residuosIn,
                          const Cromossomo    &cr);
    void LMQ_ResolverSistLinear(XVetor<qreal>  &vetor,
                                XMatriz<qreal> &matriz);
    void LMQ_MinimizarLevMarq(Cromossomo *cr,
                              const XVetor<qreal> &GL_fx,
                              const bool &isRefino);
};
#endif  //SRLEVMARQ
