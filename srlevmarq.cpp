#include <QDateTime>
#include "srlevmarq.h"
#include "xtipodados.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <QDebug>
#include <math.h>
#include <limits>
#include <QtScript/QtScript>
#ifndef QT_NO_SCRIPTTOOLS
#include <QScriptEngineDebugger>
#endif

#include <QtScript/QScriptClassPropertyIterator>
#include <QtScript/QScriptEngine>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Funções ////////////////////////////////////
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
    //Calcula o ganho obtido com um modelo linear sendo i o nº de regressores
    for (i = 0; i < hlm.size(); i++) res += (hlm.at(i) * (hlm.at(i) * paramAmortecimento - matQuasiGrad.item(i,0)));
    res *= (0.5);
}
//////////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************
*  Função: LMQ Resolver sistemas lineares
*
*  Descrição da função
*     Resolve sistemas de equações lineares do tipo: [a].{x} = {b}
*
*  Parâmetros
*     matCoeficientes - [a] matriz simétrica.
*     vetor           - {b} termos independentes.
*     vetor           - {x} solução do sistema.
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
        aux = matriz.item(i,i);
        for (j=i;j<ordem;++j) matriz(i,j) /= aux;
        vetor[i] /= aux;
        for (k=0;k<ordem;++k)
            if (k!=i)
            {
                for(j=i;j<ordem;++j) matriz(k,j) -= matriz.item(k,i)*matriz.item(i,j);
                vetor[k] -= matriz.item(k,i)*vetor.at(i);
            }
    }
    /************** Outro Método ********************
    // Redução da matriz [a]
    for (j = 1; j < ordem; j++)
    {
            for (i = 1; i < j; i++)
                for (k = 0; k < i; k++) matriz(j,i) -= matriz.item(i,k) * matriz.item(j,k);
            for (k = 0; k < j; k++)
            {
                    matriz(j,j) -= matriz.item(j,k) / matriz.item(k,k) * matriz.item(j,k);
                    matriz(j,k) /= matriz.item(k,k);
            }
    }

    // Redução do vetor {b}
    for (i = 1; i < ordem; i++)
        for (k = 0; k < i; k++) vetor[i] -= matriz.item(i,k) * vetor[k];

    for (i = 0; i < ordem; i++)
        vetor[i] /= matriz.item(i,i);

    //Back-substituiçao
    for (i = ordem - 1; i > 0; i--)
        for (k = 0; k < i; k++)
                vetor[k] -= matriz.item(i,k) * vetor[i];
    ************** Outro Método ********************/
}//////////////////////////////////////////////////////////////////////////////////////////
inline void SRLevMarq::LMQ_ProdVet(qreal *prodVetorial,const qint32 incProdVet,
                                   const qint32 tam,
                                   const qreal *endVet1,const qint32 incVet1,
                                   const qreal *endVet2,const qint32 incVet2
                                   ) const
{
    if((endVet1 != NULL)&&(endVet2 != NULL)&&(prodVetorial != NULL))
    {
        *prodVetorial= (*endVet1)*(*endVet1);
        for(qint32 i=1;i<tam;i++)
        {
            endVet1 += incVet1;
            endVet2 += incVet2;
            prodVetorial += incProdVet;
            *prodVetorial = (*endVet1)*(*endVet1);
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
        *prodEscalar = (*endVet1)*(*endVet1);
        for(qint32 i=1;i<tam;i++)
        {
            endVet1 += incVet1;
            endVet2 += incVet2;
            *prodEscalar += (*endVet1)*(*endVet1);
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
    /*
    //vlrRegressores - Matriz sendo linha os regressores e coluna os atrasos.
    const qint32 qtdeRegress = cr.regressores.size();
    const qreal *func,*vlrRegress,*coef;
    qreal auxND[2]={0.0f,0.0f},*res;
    qint32 i;
    if((coefic.size()==(qtdeRegress+3))&&(GL_fx.size()==residuos->size()))
    {
        for(vlrRegress = cr.vlrRegressores.begin(),res = residuos->begin(),func = GL_fx.begin();res<residuos->end();func++,res++)
        {
            //Calcula a parcela referente ao numerador e ao denominador
            auxND[1] = *(coefic.begin()+1);auxND[0] = *(coefic.end()-1);
            for(i=0,coef=coefic.begin()+2;i<qtdeRegress;vlrRegress++,coef++,i++)
            {
                auxND[i<cr.numDenom?1:0] += (*coef)*(*vlrRegress);
                //else qDebug() << "Func:LMQ_CalcResiduos-falhou";
            }
            if(!auxND[1]) auxND[1]=1.0f;if(!auxND[0]) auxND[0]=1.0f;
            //Calcula o residuo
            *res = (*func) - (coefic.first()+ValidQRealMarq(auxND[1]/auxND[0]));
        }
    }
    else if(GL_fx.isEmpty()) qDebug() << "Func:LMQ_CalcResiduos-GL_fx.isEmpty";
    else if(coefic.isEmpty()) qDebug() << "Func:LMQ_CalcResiduos-coefic.isEmpty";
    else qDebug() << "Func:LMQ_CalcResiduos-coefic.size():=" << coefic.size() << "qtdeRegress:=" <<qtdeRegress+3;
    */
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_CalcHessGrad(XMatriz<qreal>* matHessi,
                                 XMatriz<qreal> *matQuasiGrad,
                                 const XMatriz<qreal> &jacobiana,
                                 const XVetor<qreal>  &residuos,
                                 const Cromossomo     &cr)
{
    //vlrRegressores - linha = regressores; coluna = atrasos.
    //JACOBIANA = d(fx)/d(x);
    //HESSIANA  = Transposta(jacobiana)*jacobiana
    //QuasiGRADIENTE = Transposta(jacobiana)*F(X)

    const qint32 qtdeRegress = matHessi->numLinhas(),
                 qtdeAtrasos = matHessi->numColunas();
    qint32 mudaNumDenum;
    qreal *hessi    = matHessi->begin(),
          *quasiGrad= matQuasiGrad->begin();
    //Encontra o ponto do vetor onde passa a ser denominador
    for(mudaNumDenum=0;(mudaNumDenum < cr.regressores.size())&&(cr.termos.at(cr.regressores.at(mudaNumDenum)).tTermo1.nd);mudaNumDenum++);
    mudaNumDenum+=2;//Incrementa os termos constantes.
    //Preenche a matriz hessiana com o quadrado da matriz jacobiana.    
    for (qint32 linha0 = 0;linha0 < qtdeRegress; linha0++)
    {
        hessi       = linha0+(matHessi->begin());
        for (qint32 linha1 = 0;linha1 < qtdeRegress; linha1++)
        {
            if(!linha0)
            {
                quasiGrad += linha1 ? 1 : 0;
                LMQ_ProdEsc(quasiGrad,qtdeAtrasos,residuos.begin(),1,linha1+jacobiana.begin(),qtdeRegress);
            }
            hessi += linha1 ? qtdeRegress : 0; //(linha0,linha1)
            LMQ_ProdEsc(hessi,qtdeAtrasos,linha0+jacobiana.begin(),qtdeRegress,linha1+jacobiana.begin(),qtdeRegress);
       }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
void SRLevMarq::LMQ_MinimizarLevMarq(Cromossomo *cr,
                                     const XVetor<qreal> &GL_fx,
                                     const bool &isRefino)
{
    /////////////////////Inicializaçao das constantes////////////
    const qint32 numMaxIteracoes = 100;
    const qreal  tolerancia      = 1.0e-5f,//1.0e-8f,
                 xProximo        = 1.0e-3f;
    const qint32 qtdeRegress = cr->vlrRegressores.numLinhas()+3,
                 qtdeAtrasos = cr->vlrRegressores.numColunas();
    ////////////////////////Declaracao de variaveis//////////////
    qint32 i=0, v = 2, numIteracoes = 0, achou = 0;
    /////////////////////Inicializaçao de variaveis//////////////
    qreal normaInfG = 1.0f,
          maxDiag   = 1.0f,
          paramAmortecimento = 1.0f,
          normaHlm = 1.0f,
          normaX = 1.0f,
          ganho = 1.0f,
          fx = 1.0f,
          fxnovo = 1.0f,
          soma = 1.0f;
    ////////////////////////////////////////////////////////////
    XVetor<qreal> coefic(qtdeRegress),
                  xnovo(qtdeRegress),
                  hlm(qtdeRegress);
    ////////////////////////////////////////////////////////////
    if(isRefino)
    {
        ////////////////////////////////////////////////////////////
        //valores - Matriz onde Linha é as variaveis (sendo a linha 0 a variavel de saida) e coluna os atrasos.
        //vlrRegressores - Matriz onde linha é os regressores e coluna é os atrasos.
        if(hessiana->numColunas() != qtdeAtrasos)
        {
            //residuos - Vetor com o valor do residuo (F(X)Real-F(X)Calculado) para cada atraso
            residuos  = new XVetor<qreal>(qtdeAtrasos);
            //Jacobiana - Matriz Derivada cuja linha = nº de atrasos e coluna = nº de Regressores
            jacobiana = new XMatriz<qreal>(qtdeAtrasos,qtdeRegress);
            //hessiana - Transposta(Jacobiana)*Jacobiana - linha = nº de atrasos e coluna = nº de atrasos
            hessiana    = new XMatriz<qreal>(qtdeRegress,qtdeRegress);
            matauxiliar = new XMatriz<qreal>(qtdeRegress,qtdeRegress);
            //Gradiente - Transposta(Jacobiana) * F(X) + Hessiana * H(x) - linha = nº de atrasos; coluna = 1
            //quasiGradi - Transposta(Jacobiana) * F(X) - linha = nº de atrasos; coluna = 1
            quasiGradi   = new XMatriz<qreal>(qtdeRegress,1);
        }

        ////////////////////////////////////////////////////////////
        //Adquiri o ponto inicial
        qCopy(cr->coefic.begin(),cr->coefic.end(),coefic.begin());//coefic = cr->coefic;
        ////////////////////////////////////////////////////////////
        LMQ_CalcJacobResiduos(residuos, jacobiana, coefic, GL_fx, *cr);
        LMQ_CalcHessGrad(hessiana, quasiGradi, *jacobiana, *residuos, *cr);
        ////////////////////////////////////////////////////////////
        //Se o maior crescimento da função no ponto x for menor que e1 então encontrou o mínimo local procurado
        normaInfG = quasiGradi->NormaInf();
        if (normaInfG <= tolerancia) achou = 1;
        else
        {
            LMQ_CalcF(fx,*residuos);
            //O parâmetro de amortecimento é dado baseando-se na matriz hessiana
            maxDiag = hessiana->MaiorElemDiagPrin();
            paramAmortecimento = xProximo * maxDiag;
        }
        //Procura o ponto crítico até encontrá-lo ou até atingir o número máximo de iterações
        while ((achou == 0) && (numIteracoes < numMaxIteracoes))
        {
            //Incrementa a iteração
            numIteracoes++;
            //Faz uma copia da matriz hessiana para uma matriz auxiliar
            qCopy(hessiana->begin(),hessiana->end(),matauxiliar->begin());
            //A soma do parâmetro de amortecimento na matriz hessiana garante que a matriz é positiva definida e seja invertível
            for(i = 0; i < coefic.size(); i++)
            {
                    (*matauxiliar)(i,i) = matauxiliar->item(i,i) + paramAmortecimento;
                    hlm[i] = -1.0 * quasiGradi->item(i,0);
            }
            //Calcula o passo em uma direção máxima de descida
            LMQ_ResolverSistLinear(hlm,*matauxiliar);
            normaHlm = hlm.normal();
            normaX   = coefic.normal();
            //Se a direção máxima de descida for menor do que e2 * (normaX + e2) então encontrou-se o ponto crítico procurado
            if (normaHlm <= (tolerancia * (normaX + tolerancia))) achou = 1;
            else
            {
                //Se não achou incrementa o passo e continua procurando
                for (i = 0; i < coefic.size(); i++) xnovo[i] = coefic.at(i) + hlm.at(i);
                //Calcula o ganho ao dar o passo
                LMQ_CalcJacobResiduos(residuos, jacobiana, xnovo, GL_fx, *cr);
                LMQ_CalcF(fxnovo,*residuos);
                LMQ_CalcGanhoLinear(soma, hlm, *quasiGradi, paramAmortecimento);
                ganho    = (fx - fxnovo) / (soma);
                //Se o ganho for maior do que 0 então vê se encontra x ao dar o passo e diminue o parâmetro de amortecimento
                if (ganho > 0)
                {
                    qCopy(xnovo.begin(),xnovo.end(),coefic.begin());//coefic = xnovo;
                    fx = fxnovo;
                    LMQ_CalcHessGrad(hessiana, quasiGradi, *jacobiana, *residuos, *cr);
                    normaInfG = quasiGradi->NormaInf();
                    //Se o maior crescimento da função no ponto x for menor que e1 então encontrou o mínimo local procurado
                    if(normaInfG <= tolerancia) achou = 1;
                    else
                    {
                        //LMQ_CalcularHessiana (hessiana, *jacobiana);
                        paramAmortecimento = paramAmortecimento * (MAX(1/3.0, 1 - pow(((2 * ganho) - 1), 3)));
                        v = 2;
                    }
                }
                //Se não for, aumenta o parâmetro de amortecimento e calcula novamente.
                else{paramAmortecimento = paramAmortecimento * v;v = 2* v;}
            }
        }
        //Imforma os resultados
        qCopy(coefic.begin(),coefic.end(),cr->coefic.begin());//cr->coefic = coefic;
    }
    else
    {
        if(hessiana->numColunas() != qtdeAtrasos) residuos  = new XVetor<qreal>(qtdeAtrasos);
        LMQ_CalcJacobResiduos(residuos, jacobiana, cr->coefic, GL_fx, *cr);
    }
    //Calcula o erro
    LMQ_CalcErro(cr->erro,*residuos);
}
