#include "chromosome_service.h"
#include "../threading/shared_state.h"
#include "linear_algebra.h"
#include <QTime>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <qmath.h>

///////////////////////////////////////////////////////////////////////////////
/// qSortDuplo — QuickSort duplo (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void qSortDuplo(compTermo *start, compTermo *end,
                qint32 *posStart, qint32 *posEnd,
                bool (*compare)(const compTermo &, const compTermo &))
{
top:
    int span = int(end - start);
    if (span < 2) return;
    --end;
    --posEnd;

    compTermo *low = start, *high = end - 1;
    qint32 *posLow = posStart, *posHigh = posEnd - 1;
    compTermo *pivot = start + span / 2;
    qint32 *posPivot = posStart + span / 2;

    if (compare(*end, *start)) { qSwap(*end, *start); qSwap(*posEnd, *posStart); }
    if (span == 2) return;
    if (compare(*pivot, *start)) { qSwap(*pivot, *start); qSwap(*posPivot, *posStart); }
    if (compare(*end, *pivot)) { qSwap(*end, *pivot); qSwap(*posEnd, *posPivot); }
    if (span == 3) return;

    qSwap(*pivot, *end);
    qSwap(*posPivot, *posEnd);

    while (low < high) {
        while ((low < high) && (compare(*low, *end))) { ++low; ++posLow; }
        while ((high > low) && (compare(*end, *high))) { --high; --posHigh; }
        if (low < high) {
            qSwap(*low, *high); ++low; --high;
            qSwap(*posLow, *posHigh); ++posLow; --posHigh;
        } else break;
    }

    if (compare(*low, *end)) { ++low; ++posLow; }

    qSwap(*end, *low);
    qSwap(*posEnd, *posLow);

    qSortDuplo(start, low, posStart, posLow, compare);

    start = low + 1;
    posStart = posLow + 1;
    ++end;
    ++posEnd;
    goto top;
}

///////////////////////////////////////////////////////////////////////////////
/// CmpMaiorApt para qSortPop (acessa shared state via ponteiro)
///////////////////////////////////////////////////////////////////////////////
static SharedState *s_sortState = nullptr;

static bool CmpMaiorAptForSort(const qint32 &countCr1, const qint32 &countCr2, const qint32 &idSaida)
{
    s_sortState->lock_BufferSR.lockForRead();
    const Cromossomo cr1 = s_sortState->Adj.Pop.at(idSaida).at(countCr1);
    const Cromossomo cr2 = s_sortState->Adj.Pop.at(idSaida).at(countCr2);
    s_sortState->lock_BufferSR.unlock();
    return (cr1.aptidao == cr2.aptidao ? cr1.erro < cr2.erro : cr1.aptidao < cr2.aptidao);
}

///////////////////////////////////////////////////////////////////////////////
static qint32 configuredDecimation(const SharedState *state, qint32 idSaida)
{
    if ((idSaida >= 0) && (idSaida < state->Adj.decimacao.size()))
        return qMax(1, state->Adj.decimacao.at(idSaida));
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
static bool hasDecimatedCacheForOutput(const SharedState *state, qint32 idSaida)
{
    if ((idSaida < 0)
        || (idSaida >= state->dadosFiltradosPorSaida.size())
        || (idSaida >= state->dadosFiltradosDecimacao.size()))
        return false;

    const qint32 dCfg = configuredDecimation(state, idSaida);
    const JMathVar<qreal> &cache = state->dadosFiltradosPorSaida.at(idSaida);
    return (dCfg > 1)
        && (state->dadosFiltradosDecimacao.at(idSaida) == dCfg)
        && (cache.numLinhas() == state->Adj.Dados.variaveis.valores.numLinhas())
        && (cache.numColunas() > 0);
}

///////////////////////////////////////////////////////////////////////////////
static const JMathVar<qreal> *selectDataForOutput(const SharedState *state, qint32 idSaida)
{
    if (hasDecimatedCacheForOutput(state, idSaida))
        return &state->dadosFiltradosPorSaida.at(idSaida);
    return &state->Adj.Dados.variaveis.valores;
}

///////////////////////////////////////////////////////////////////////////////
static qint32 effectiveDecimationStep(const SharedState *state, qint32 idSaida)
{
    return hasDecimatedCacheForOutput(state, idSaida) ? 1 : configuredDecimation(state, idSaida);
}

///////////////////////////////////////////////////////////////////////////////
ChromosomeService::ChromosomeService(SharedState *state)
    : m_state(state)
{
    m_rng.seed(QTime::currentTime().msec());
}

///////////////////////////////////////////////////////////////////////////////
/// qSortPop — EXATAMENTE igual ao original
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::qSortPop(qint32 *start, qint32 *end, qint32 idSaida) const
{
top:
    int span = int(end - start);
    if (span < 2) return;
    --end;

    qint32 *low = start, *high = end - 1;
    qint32 *pivot = start + span / 2;

    // Usa acesso thread-safe ao shared state
    s_sortState = m_state;

    if (CmpMaiorAptForSort(*end, *start, idSaida)) qSwap(*end, *start);
    if (span == 2) return;
    if (CmpMaiorAptForSort(*pivot, *start, idSaida)) qSwap(*pivot, *start);
    if (CmpMaiorAptForSort(*end, *pivot, idSaida)) qSwap(*end, *pivot);
    if (span == 3) return;

    qSwap(*pivot, *end);

    while (low < high) {
        while (low < high && CmpMaiorAptForSort(*low, *end, idSaida))++low;
        while (high > low && CmpMaiorAptForSort(*end, *high, idSaida))--high;
        if (low < high) { qSwap(*low, *high); ++low; --high; }
        else break;
    }

    if (CmpMaiorAptForSort(*low, *end, idSaida))++low;

    qSwap(*end, *low);

    qSortPop(start, low, idSaida);

    start = low + 1;
    ++end;
    goto top;
}

///////////////////////////////////////////////////////////////////////////////
/// createRandom — DES_criaCromossomo (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
Cromossomo ChromosomeService::createRandom(qint32 idSaida) const
{
    Cromossomo cr;
    QVector<compTermo> vetTermo;
    MTRand RG(QTime::currentTime().msec());
    qint32 tamCrom, tamRegress, i;
    compTermo vlrTermo;
    const JMathVar<qreal> *dadosRef = selectDataForOutput(m_state, idSaida);
    const qint32 decEff = effectiveDecimationStep(m_state, idSaida);
    const quint32 numVariaveis = dadosRef->numLinhas(),
                  numAtrasos = qMax<qint32>(1, (dadosRef->numColunas()) / (2 * decEff)),
                  vlrMaxAtras = numAtrasos < 30 ? numAtrasos : 30;
    cr.idSaida = idSaida;

    // Inicializa os coeficientes constantes
    vlrTermo.vTermo.tTermo1.atraso = 0;
    vlrTermo.vTermo.tTermo1.nd = 1;
    vlrTermo.vTermo.tTermo1.reg = 0;
    vlrTermo.vTermo.tTermo1.var = 1;
    vlrTermo.expoente = 1;
    vetTermo.append(vlrTermo);
    cr.regress.append(vetTermo);
    vetTermo.clear();

    // Inicializa os coeficientes normais
    tamCrom = RG.randInt(3);
    while (tamCrom >= 0) {
        vlrTermo.vTermo.tTermo1.nd = m_state->Adj.isRacional ? RG.randInt(0, 1) : 1;
        vlrTermo.vTermo.tTermo1.reg = RG.randInt(1, (MASKREG / 2) - 1);
        tamRegress = RG.randInt(3);
        while (tamRegress >= 0) {
            vlrTermo.vTermo.tTermo1.var = RG.randInt(1, numVariaveis);
            vlrTermo.vTermo.tTermo1.atraso = RG.randInt(1, vlrMaxAtras);
            if ((qint32)vlrTermo.vTermo.tTermo1.atraso > cr.maiorAtraso)
                cr.maiorAtraso = vlrTermo.vTermo.tTermo1.atraso;
            vlrTermo.expoente = (qreal)RG.randInt(1, 10);
            if (!vlrTermo.expoente) vlrTermo.expoente = 1;
            vetTermo.append(vlrTermo);
            tamRegress--;
        }
        std::sort(vetTermo.begin(), vetTermo.end(), CmpMaiorTerm);
        for (i = 1; i < vetTermo.size(); i++)
            if (vetTermo.at(i).vTermo.tTermo0 == vetTermo.at(i - 1).vTermo.tTermo0)
                vetTermo.remove(i--);
        cr.regress.append(vetTermo);
        vetTermo.clear();
        tamCrom--;
    }
    cr.err.fill(-1, cr.regress.size());
    evaluate(cr);
    return cr;
}

///////////////////////////////////////////////////////////////////////////////
/// montaVlrs — DES_MontaVlrs (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::montaVlrs(Cromossomo &cr, JMathVar<qreal> &vlrsRegress,
                                   JMathVar<qreal> &vlrsMedido,
                                   bool isValidacao,
                                   bool isLinearCoef) const
{
    JStrSet jst;
    qint32 i = 0, j = 0, countRegress = 0, variavel = 0, atraso = 0;
    qreal expo = 0.;
    JMathVar<qreal> matAux;
    const JMathVar<qreal> *dadosRef = selectDataForOutput(m_state, cr.idSaida);
    const qint32 decStep = effectiveDecimationStep(m_state, cr.idSaida);
    const qint32 numColsRef = dadosRef->numColunas();

    for (cr.maiorAtraso = 0, i = 0; i < cr.regress.size(); i++) {
        if (cr.regress.at(i).size()) {
            for (j = 0; j < cr.regress.at(i).size(); j++)
                if ((qint32)cr.regress.at(i).at(j).vTermo.tTermo1.atraso > cr.maiorAtraso)
                    cr.maiorAtraso = cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
        } else {
            cr.regress.remove(i); i--;
        }
    }

    const qint32 qtdeAtrasos = numColsRef / ((isValidacao ? 1 : 2) * decStep),
                 posIniAtrasos = cr.maiorAtraso * decStep,
                 tam = qtdeAtrasos - cr.maiorAtraso;

    vlrsMedido.replace(*dadosRef,
                       jst.set("(:,0)=(0,%1:%2:%3)'")
                           .argInt(posIniAtrasos)
                           .argInt(decStep)
                           .argInt(posIniAtrasos + tam * decStep));

    for (countRegress = 0; (countRegress < cr.regress.size()) && (countRegress < (tam - 2)); countRegress++) {
        for (i = 0; i < cr.regress.at(countRegress).size(); i++) {
            variavel = cr.regress.at(countRegress).at(i).vTermo.tTermo1.var;
            if (!m_state->Adj.isTipoExpo)
                expo = cr.regress.at(countRegress).at(i).expoente;
            else {
                expo = (qint32)cr.regress.at(countRegress).at(i).expoente;
                expo += ((cr.regress.at(countRegress).at(i).expoente - expo) >= 0.5) ? 1
                    : (((cr.regress.at(countRegress).at(i).expoente - expo) <= -0.5) ? -1 : 0);
                if (m_state->Adj.isTipoExpo == 2) expo = fabs(expo);
            }
            if ((expo > 1e-5) || (expo < -1e-5) || (!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg)) {
                atraso = cr.regress.at(countRegress).at(i).vTermo.tTermo1.atraso;
                if (!cr.regress.at(countRegress).at(i).vTermo.tTermo1.reg
                        ? matAux.fill(1, tam, 1)
                        : matAux.replace(*dadosRef,
                                         jst.set("(:,:)=(%1,%2:%3:%4)'^%f1")
                                             .argInt(variavel - 1)
                                             .argInt(posIniAtrasos - atraso * decStep)
                                             .argInt(decStep)
                                             .argInt(posIniAtrasos + (tam - atraso) * decStep)
                                             .argReal(expo))
                    || isValidacao) {
                    if ((i <= 0) && (!cr.regress.at(countRegress).at(i).vTermo.tTermo1.nd) && isLinearCoef)
                        matAux.replace(vlrsMedido, jst.set("(:,:)*=-1*(:,:)"));
                    vlrsRegress.replace(matAux, jst.set((QString("(:,%1)") + QString((i == 0) ? "=" : "*=") + QString("(:,:)")).toLatin1()).argInt(countRegress));
                } else
                    cr.regress[countRegress].remove(i--);
            } else
                cr.regress[countRegress].remove(i--);
        }
        if (!cr.regress.at(countRegress).size()) cr.regress.remove(countRegress--);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// calcVlrsEstRes — DES_CalcVlrsEstRes (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::calcVlrsEstRes(const Cromossomo &cr,
                                        const JMathVar<qreal> &vlrsRegress,
                                        const JMathVar<qreal> &vlrsCoefic,
                                        const JMathVar<qreal> &vlrsMedido,
                                        JMathVar<qreal> &vlrsResiduo,
                                        JMathVar<qreal> &vlrsEstimado) const
{
    JStrSet jst;
    JMathVar<qreal> vlrsRegressNum, vlrsRegressDen, vlrsCoeficNum, vlrsCoeficDen, a, b;
    qint32 i = 0, atraso = 0, tamNum = 0, tamDen = 0;
    qreal *estimado, *residuo;
    const qreal *medido;

    for (tamNum = 0, tamDen = 0, i = 0; i < cr.regress.size(); i++) {
        if (cr.regress.at(i).at(0).vTermo.tTermo1.nd) {
            vlrsCoeficNum.copy(vlrsCoefic, jst.set("(:,%1)=(:,%2)").argInt(tamNum).argInt(i));
            vlrsRegressNum.copy(vlrsRegress, jst.set("(:,%1)=(:,%2)").argInt(tamNum++).argInt(i));
        } else {
            vlrsCoeficDen.copy(vlrsCoefic, jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            vlrsRegressDen.copy(vlrsRegress, jst.set("(:,%1)=(:,%2)").argInt(tamDen++).argInt(i));
        }
    }

    vlrsEstimado = vlrsRegressNum(vlrsCoeficNum, jst.set("(:,:)*(:,:)'"));
    b = vlrsRegressDen(vlrsCoeficDen, jst.set("(:,:)*(:,:)'"));
    a.fill(1, vlrsEstimado.numLinhas(), vlrsEstimado.numColunas());
    if (b.numLinhas() > 0)
        a.copy(b, jst.set("(:,:)+=(:,:)"));

    const qint32 tamErro = vlrsCoefic.numColunas() - (tamNum + tamDen);
    if (tamErro) {
        vlrsResiduo.fill(0, vlrsMedido.numLinhas(), 1);
        const qint32 tamvlrsRegress = (tamNum + tamDen);
        const qreal *denVal = a.begin();
        for (atraso = 0, estimado = vlrsEstimado.begin(), residuo = vlrsResiduo.begin(),
             medido = vlrsMedido.begin();
             medido < vlrsMedido.end();
             medido++, residuo++, estimado++, atraso++, denVal++) {
            for (i = 0; i < tamErro; i++)
                *estimado += vlrsCoefic.at(tamvlrsRegress + i) * ((atraso - i) >= 0 ? *(residuo - i) : 0);
            *estimado /= *denVal;
            *residuo = *medido - *estimado;
        }
    } else {
        vlrsEstimado.copy(a, jst.set("(:,:)/=(:,:)"));
        vlrsResiduo = vlrsMedido(vlrsEstimado, jst.set("(:)-(:)"));
    }
}

///////////////////////////////////////////////////////////////////////////////
/// calcERR — DES_CalcERR (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::calcERR(Cromossomo &cr, qreal metodoSerr) const
{
    JStrSet jst;
    qint32 i = 0, j = 0, k = 0, aux, start = 0;
    qreal vlrsMedidoQuad = 0., serr = 0., u = 0.;
    JMathVar<qreal> vlrsMedido, vlrsRegress, A, a, c, x, v;

    montaVlrs(cr, vlrsRegress, vlrsMedido, true);

    if (cr.regress.size()) {
        A = vlrsRegress;
        A.replace(vlrsMedido, jst.set("(:,%1)=(:,:)").argInt(A.numColunas()));
        const qint32 n = A.numColunas() - 1;
        vlrsMedidoQuad = vlrsMedido(vlrsMedido, jst.set("(:)'*(:)")).at(0);
        c.resize(n);
        cr.err.fill(0, n);

        for (j = 0; j < n; j++) {
            for (k = j; k < n; k++) {
                x = (A(A, jst.set("(%1:,%2)'*(%1:,%3)^2").argInt(j).argInt(k).argInt(n)));
                v = (A(A, jst.set("%f1*(%1:,%2)'*(%1:,%2)").argReal(vlrsMedidoQuad).argInt(j).argInt(k)));
                c(k) = x.at(0) / v.at(0);
            }
            cr.err(j) = c.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n), start, aux);
            if (aux != j) {
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j], cr.regress[aux]);
            }
            v = A(jst.set("(%1:,%1)'").argInt(j));
            u = v.Norma(2);
            if (u != 0) {
                u = ((v.at(0) > 1e-10) || (v.at(0) < -1e-10) ? v.at(0)
                     : (v.at(0) > 0) ? 1e-10 : -1e-10) + sign(v.at(0)) * u;
                v.replace(v, jst.set("(1:)=%f1*(1:)").argReal(1 / u));
            }
            v(0) = 1;
            a = A(jst.set("(%1:,%1:)").argInt(j));
            u = -2 / v(v, jst.set("(:)*(:)'")).at(0);
            x = a(a, jst.set("(0:,0:)=%f1*(0:,0:)").argReal(u));
            x = x(v, jst.set("(:,:)'*(:,:)'"));
            x = v(x, jst.set("(:,:)'*(:,:)'"));
            a.replace(x, jst.set("(:,:)+=(:,:)"));
            A.replace(a, jst.set("(%1,%1)=(:,:)").argInt(j));
        }

        for (j = 0; j < n - 1; j++) {
            cr.err.MaiorElem(jst.set("(%1:1:%2)").argInt(j).argInt(n), start, aux);
            if (aux != j) {
                cr.err.swap(jst.set("(%1)=(%2)").argInt(j).argInt(aux));
                A.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                vlrsRegress.swap(jst.set("(:,%1)=(:,%2)").argInt(j).argInt(aux));
                qSwap(cr.regress[j], cr.regress[aux]);
            }
        }

        a.clear();
        for (j = 0, serr = 0.0;
             (j < cr.err.size()) && ((serr < metodoSerr) || (cr.err.at(j) > 0.001)) && (cr.err.at(j) > 0.0009) && (cr.err.at(j) == cr.err.at(j));
             j++)
            serr += cr.err.at(j);
        vlrsRegress = vlrsRegress(jst.set("(:,0:1:%1)").argInt(j));
        cr.err = cr.err(jst.set("(:,0:1:%1)").argInt(j));
        for (; j < cr.regress.size();) cr.regress.remove(j);

        for (cr.maiorAtraso = 0, i = 0; i < cr.regress.size(); i++)
            for (j = 0; j < cr.regress.at(i).size(); j++)
                if ((qint32)cr.regress.at(i).at(j).vTermo.tTermo1.atraso > cr.maiorAtraso)
                    cr.maiorAtraso = cr.regress.at(i).at(j).vTermo.tTermo1.atraso;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// evaluate — DES_calAptidao (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::evaluate(Cromossomo &cr, quint32 tamErro) const
{
    bool isOk1, isOk2, isOk = false;
    JStrSet jst;
    qreal var = 0, var1 = 0, erroDepois = 9e99;
    QVector<QVector<compTermo> > regressNum, regressDen;
    JMathVar<qreal> vlrsRegress, vlrsRegress1, vlrsRegressNum, vlrsRegressDen, vlrsRegressDenAux,
        vlrsCoefic, vlrsCoefic1, vlrsEstimado, vlrsResiduo, vlrsMedido,
        errNum, errDen, auxDen, sigma1, sigma2, aux1, aux2, x, v;
    qint32 i, tamNum = 0, tamDen = 0, count1 = 0, count2 = 0, size = 0;

    cr.erro = 9e99;
    cr.aptidao = 9e99;
    errNum.remove('C', 0);
    errDen.remove('C', 0);

    montaVlrs(cr, vlrsRegress, vlrsMedido, true, false);
    const qint32 qtdeAtrasos = vlrsMedido.numLinhas();

    if (qtdeAtrasos <= 0 || cr.regress.size() == 0) return;

    while (cr.regress.size() > vlrsRegress.numColunas()) {
        cr.regress.removeLast();
        cr.err.remove('C', cr.err.numColunas() - 1);
    }

    for (i = 0; i < cr.regress.size(); i++) size += cr.regress.at(i).size();

    for (tamNum = 0, tamDen = 0, i = 0; i < cr.regress.size(); i++) {
        if (cr.regress.at(i).at(0).vTermo.tTermo1.nd) {
            regressNum.append(cr.regress.at(i));
            errNum.append('C', cr.err.at(i));
            vlrsRegressNum.copy(vlrsRegress, jst.set("(:,%1)=(:,%2)").argInt(tamNum++).argInt(i));
        } else {
            regressDen.append(cr.regress.at(i));
            errDen.append('C', cr.err.at(i));
            vlrsRegressDen.copy(vlrsRegress, jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            auxDen.copy(vlrsRegress, jst.set("(:,%1)=(:,%2)").argInt(tamDen).argInt(i));
            auxDen.replace(vlrsMedido, jst.set("(:,%1)*=-1*(:,:)").argInt(tamDen++));
        }
    }

    if (!tamNum) {
        tamNum++;
        compTermo vlrTermo;
        vlrTermo.vTermo.tTermo1.atraso = 0;
        vlrTermo.vTermo.tTermo1.nd = 1;
        vlrTermo.vTermo.tTermo1.reg = 0;
        vlrTermo.vTermo.tTermo1.var = 1;
        vlrTermo.expoente = 1;
        QVector<compTermo> vetTermo;
        vetTermo.append(vlrTermo);
        regressNum.prepend(vetTermo);
        errNum.prepend('C', -1);
        vlrsRegressNum.fill(1, qtdeAtrasos, 1);
    }

    cr.regress.clear(); cr.regress += regressNum; cr.regress += regressDen;
    cr.err.clear(); cr.err.append('c', errNum); cr.err.append('c', errDen);
    cr.err.setNumColunas(errNum.numColunas() + errDen.numColunas());
    cr.vlrsCoefic.fill(0, tamNum + tamDen);
    vlrsRegress = vlrsRegressNum;
    vlrsRegress1 = vlrsRegressNum;

    if ((vlrsRegressDen.numLinhas() == qtdeAtrasos) || (vlrsRegress.numLinhas() == 1)) {
        vlrsRegress1.copy(vlrsRegressDen, jst.set("(:,%1)=(:,:)").argInt(tamNum));
        vlrsRegress.copy(auxDen, jst.set("(:,%1)=(:,:)").argInt(tamNum));
        vlrsRegressDenAux.replace(auxDen, jst.set("(:,:)=-1*(:,:)"));
    }

    if (vlrsRegressDenAux.numLinhas() == qtdeAtrasos) {
        aux1 = vlrsRegressDenAux(vlrsRegressDenAux, jst.set("(:,:)'*(:,:)"));
        sigma1.copy(aux1, jst.set("(%1,%1)=(:,:)").argInt(tamNum));
        aux2.resize(vlrsRegressDenAux.numColunas(), 1);
        for (i = 0; i < vlrsRegressDenAux.numColunas(); i++)
            aux2(i, 0) = -1 * sum(vlrsRegressDenAux(jst.set("(:,%1)'").argInt(i)));
        sigma2.copy(aux2, jst.set("(%1,:)=(:,:)").argInt(tamNum));
    } else {
        sigma1.fill(0, vlrsRegress.numColunas(), vlrsRegress.numColunas());
        sigma2.fill(0, vlrsRegress.numColunas(), 1);
    }

    count1 = 0;
    do {
        if (count1 && isOk && (erroDepois < cr.erro)) {
            const qint32 nBic = qMax<qint32>(1, qtdeAtrasos);
            const qreal sqrtN = qSqrt((qreal)nBic);
            // k_eff: cada regressor pesa (1 + maxDelay_i / sqrt(n))
            qreal kEff = 0.0;
            for (qint32 r = 0; r < cr.regress.size(); r++) {
                qint32 maxDelayR = 0;
                for (qint32 t = 0; t < cr.regress.at(r).size(); t++) {
                    const qint32 d = cr.regress.at(r).at(t).vTermo.tTermo1.atraso;
                    if (d > maxDelayR) maxDelayR = d;
                }
                kEff += 1.0 + (qreal)maxDelayR / sqrtN;
            }
            kEff += (count1 - 1); // termos de erro (delay 0, peso = 1.0)
            cr.vlrsCoefic = vlrsCoefic;
            cr.erro = erroDepois;
            cr.aptidao = nBic * qLn(cr.erro)
                + kEff * qLn(nBic);
        }

        for (i = 0; (i < count1) && (vlrsResiduo.numLinhas() > 1); i++) {
            vlrsResiduo.prepend('L', 0);
            vlrsResiduo.remove('L', vlrsResiduo.numLinhas() - 1);
            vlrsRegress.copy(vlrsResiduo, jst.set("(:,%1)=(:,:)").argInt(tamNum + tamDen + i));
            vlrsRegress1.copy(vlrsResiduo, jst.set("(:,%1)=(:,:)").argInt(tamNum + tamDen + i));
        }

        count2 = 0;
        do {
            if (count2) {
                var = var1;
                vlrsCoefic = vlrsCoefic1;
            }
            v = vlrsRegress(vlrsRegress, jst.set("(:,:)'*(:,:)"));
            v.copy(sigma1, jst.set("(:,:)-=%f1*(:,:)").argReal(var));
            x = vlrsRegress(vlrsMedido, jst.set("(:,:)'*(:,:)"));
            x.copy(sigma2, jst.set("(:,:)-=%f1*(:,:)").argReal(var));
            vlrsCoefic1 = v.SistemaLinear(x, isOk);

            if (isOk) {
                calcVlrsEstRes(cr, vlrsRegress1, vlrsCoefic1, vlrsMedido, vlrsResiduo, vlrsEstimado);
                var1 = cov(vlrsResiduo);
                isOk1 = compara(vlrsCoefic, vlrsCoefic1, 1e-3);
                isOk2 = (var - var1) == 0 ? true : (var - var1) > 0 ? (var - var1) < 1e-3 : (var - var1) > -1e-3;
                count2++;
            }
        } while (isOk && (!(isOk1 && isOk2)) && count2 <= 20);
        erroDepois = vlrsResiduo(vlrsResiduo, jst.set("(:,:)'*(:,:)")).at(0) / qtdeAtrasos;
        count1++;
    } while ((quint32)count1 <= tamErro);

    for (isOk = false, i = 0; i < cr.regress.size(); i++)
        if (cr.vlrsCoefic.at(i) != 0.0
            ? ((cr.vlrsCoefic.at(i) <= 1e-3) && (cr.vlrsCoefic.at(i) >= -1e-3))
                || (cr.vlrsCoefic.at(i) >= 1e+3) || (cr.vlrsCoefic.at(i) <= -1e+3)
            : false) {
            cr.regress.remove(i);
            cr.err.remove('C', i);
            isOk = true;
        }
    if (isOk) evaluate(cr);
}

///////////////////////////////////////////////////////////////////////////////
/// generateTrial — DES_GenerateTrial (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
Cromossomo ChromosomeService::generateTrial(const Cromossomo &target,
                                             const Cromossomo &pbest,
                                             const Cromossomo &r1,
                                             const Cromossomo &r2,
                                             double F, double CR) const
{
    if (target.regress.isEmpty()) return target;
    if (target.idSaida < 0 || target.idSaida >= m_state->Adj.decimacao.size()) {
        qDebug() << "[DE] generateTrial: idSaida out of range" << target.idSaida;
        return target;
    }

    MTRand RG(QTime::currentTime().msec());
    Cromossomo trial;
    trial.idSaida = target.idSaida;
    trial.maiorAtraso = 0;

    // Phase 1: Coleta termos
    QVector<compTermo> termosAnalisados;
    QVector<qint32> posTermosAnalisados;
    qint32 i, j;

    for (i = 0; i < target.regress.size(); i++) {
        termosAnalisados += target.regress.at(i);
        posTermosAnalisados += QVector<qint32>(target.regress.at(i).size(), 0);
    }
    for (i = 0; i < pbest.regress.size(); i++) {
        termosAnalisados += pbest.regress.at(i);
        posTermosAnalisados += QVector<qint32>(pbest.regress.at(i).size(), 1);
    }
    for (i = 0; i < r1.regress.size(); i++) {
        termosAnalisados += r1.regress.at(i);
        posTermosAnalisados += QVector<qint32>(r1.regress.at(i).size(), 2);
    }
    for (i = 0; i < r2.regress.size(); i++) {
        termosAnalisados += r2.regress.at(i);
        posTermosAnalisados += QVector<qint32>(r2.regress.at(i).size(), 3);
    }

    if (termosAnalisados.isEmpty()) { trial = target; return trial; }

    // Phase 2: Ordena e computa expoentes mutados
    qSortDuplo(termosAnalisados.begin(), termosAnalisados.end(),
               posTermosAnalisados.begin(), posTermosAnalisados.end(), CmpMaiorTerm);

    QVector<compTermo> mutTermos, tgtTermos;
    QVector<compTermo> termoAv(5, compTermo());
    compTermo auxTermo;

    termoAv[4] = termosAnalisados.at(0);
    termoAv[posTermosAnalisados.at(0)] = termosAnalisados.at(0);

    qint32 trIdx = 1;
    const qint32 maxIdx = termosAnalisados.size();

    for (; trIdx <= maxIdx; trIdx++)
    for (; trIdx <= maxIdx; trIdx++) {
        if (trIdx < maxIdx ? termoAv.at(4).vTermo.tTermo0 != termosAnalisados.at(trIdx).vTermo.tTermo0 : true) {
            if (termoAv.at(0).vTermo.tTermo0 || termoAv.at(1).vTermo.tTermo0 ||
                termoAv.at(2).vTermo.tTermo0 || termoAv.at(3).vTermo.tTermo0) {
                if (termoAv.at(0).vTermo.tTermo0) auxTermo.vTermo.tTermo0 = termoAv.at(0).vTermo.tTermo0;
                else if (termoAv.at(1).vTermo.tTermo0) auxTermo.vTermo.tTermo0 = termoAv.at(1).vTermo.tTermo0;
                else if (termoAv.at(2).vTermo.tTermo0) auxTermo.vTermo.tTermo0 = termoAv.at(2).vTermo.tTermo0;
                else auxTermo.vTermo.tTermo0 = termoAv.at(3).vTermo.tTermo0;

                auxTermo.expoente = termoAv.at(0).expoente
                    + F * (termoAv.at(1).expoente - termoAv.at(0).expoente)
                    + F * (termoAv.at(2).expoente - termoAv.at(3).expoente);

                if (auxTermo.expoente != 0.0) mutTermos.append(auxTermo);
            }
            if (termoAv.at(0).vTermo.tTermo0) tgtTermos.append(termoAv.at(0));

            termoAv[0].vTermo.tTermo0 = 0; termoAv[0].expoente = 0.0;
            termoAv[1].vTermo.tTermo0 = 0; termoAv[1].expoente = 0.0;
            termoAv[2].vTermo.tTermo0 = 0; termoAv[2].expoente = 0.0;
            termoAv[3].vTermo.tTermo0 = 0; termoAv[3].expoente = 0.0;
        }
        if (trIdx < maxIdx) {
            termoAv[4] = termosAnalisados.at(trIdx);
            qint32 pos = posTermosAnalisados.at(trIdx);
            if (termoAv.at(pos).vTermo.tTermo0 == termosAnalisados.at(trIdx).vTermo.tTermo0)
                termoAv[pos].expoente += termosAnalisados.at(trIdx).expoente;
            else termoAv[pos] = termosAnalisados.at(trIdx);
        }
    }

    // Phase 3: Crossover binomial
    std::sort(mutTermos.begin(), mutTermos.end(), CmpMaiorTerm);
    std::sort(tgtTermos.begin(), tgtTermos.end(), CmpMaiorTerm);

    QVector<quint32> uKey;
    QVector<qreal> uMutExp, uTgtExp;
    QVector<qint32> uHasMut, uHasTgt;
    qint32 mi = 0, ti = 0;

    while (mi < mutTermos.size() || ti < tgtTermos.size()) {
        quint32 mk = (mi < mutTermos.size()) ? mutTermos.at(mi).vTermo.tTermo0 : 0;
        quint32 tk = (ti < tgtTermos.size()) ? tgtTermos.at(ti).vTermo.tTermo0 : 0;
        if (mi >= mutTermos.size()) {
            uKey.append(tk); uMutExp.append(0.0); uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(0); uHasTgt.append(1); ti++;
        } else if (ti >= tgtTermos.size()) {
            uKey.append(mk); uMutExp.append(mutTermos.at(mi).expoente); uTgtExp.append(0.0);
            uHasMut.append(1); uHasTgt.append(0); mi++;
        } else if (mk > tk) {
            uKey.append(mk); uMutExp.append(mutTermos.at(mi).expoente); uTgtExp.append(0.0);
            uHasMut.append(1); uHasTgt.append(0); mi++;
        } else if (tk > mk) {
            uKey.append(tk); uMutExp.append(0.0); uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(0); uHasTgt.append(1); ti++;
        } else {
            uKey.append(mk); uMutExp.append(mutTermos.at(mi).expoente);
            uTgtExp.append(tgtTermos.at(ti).expoente);
            uHasMut.append(1); uHasTgt.append(1); mi++; ti++;
        }
    }

    if (uKey.isEmpty()) { trial = target; return trial; }

    const qint32 jrand = RG.randInt(0, uKey.size() - 1);
    QVector<compTermo> trialTermos;
    for (i = 0; i < uKey.size(); i++) {
        bool fromMutant = (RG.randReal(0.0, 1.0) < CR) || (i == jrand);
        compTermo t;
        t.vTermo.tTermo0 = uKey.at(i);
        if (fromMutant) {
            if (uHasMut.at(i)) { t.expoente = uMutExp.at(i); trialTermos.append(t); }
        } else {
            if (uHasTgt.at(i)) { t.expoente = uTgtExp.at(i); trialTermos.append(t); }
        }
    }

    if (trialTermos.isEmpty()) {
        compTermo t;
        t.vTermo.tTermo0 = uKey.at(jrand);
        t.expoente = uHasMut.at(jrand) ? uMutExp.at(jrand) : uTgtExp.at(jrand);
        trialTermos.append(t);
    }

    // Phase 4: Agrupa por idReg
    std::sort(trialTermos.begin(), trialTermos.end(), CmpMaiorTerm);
    trial.regress.clear();
    QVector<compTermo> currentGroup;
    quint32 currentIdReg = trialTermos.at(0).vTermo.tTermo2.idReg;

    for (i = 0; i < trialTermos.size(); i++) {
        quint32 thisIdReg = trialTermos.at(i).vTermo.tTermo2.idReg;
        if (thisIdReg != currentIdReg) {
            if (!currentGroup.isEmpty()) {
                for (j = 1; j < currentGroup.size(); j++)
                    if (currentGroup.at(j).vTermo.tTermo0 == currentGroup.at(j - 1).vTermo.tTermo0)
                        currentGroup.remove(j--);
                trial.regress.append(currentGroup);
            }
            currentGroup.clear();
            currentIdReg = thisIdReg;
        }
        currentGroup.append(trialTermos.at(i));
        if ((qint32)trialTermos.at(i).vTermo.tTermo1.atraso > trial.maiorAtraso)
            trial.maiorAtraso = trialTermos.at(i).vTermo.tTermo1.atraso;
    }
    if (!currentGroup.isEmpty()) {
        for (j = 1; j < currentGroup.size(); j++)
            if (currentGroup.at(j).vTermo.tTermo0 == currentGroup.at(j - 1).vTermo.tTermo0)
                currentGroup.remove(j--);
        trial.regress.append(currentGroup);
    }

    // Phase 5: Limites, poda, aptidão
    const qint32 decVal = effectiveDecimationStep(m_state, trial.idSaida);
    const qint32 numColsRef = selectDataForOutput(m_state, trial.idSaida)->numColunas();
    const qint32 qtdeAtrasos = (decVal > 0)
        ? (numColsRef / (2 * decVal)) - trial.maiorAtraso
        : 1;
    while (trial.regress.size() > qtdeAtrasos && trial.regress.size() > 1)
        trial.regress.removeLast();

    trial.err.fill(-1, trial.regress.size());
    calcERR(trial, m_state->Adj.serr);
    evaluate(trial);

    // Poda probabilística por importância (apenas para estruturas maiores)
    qint32 totalTerms = 0;
    for (qint32 rr = 0; rr < trial.regress.size(); ++rr)
        totalTerms += trial.regress.at(rr).size();
    if (m_state->Adj.iteracoes >= ModelPruningConfig::kPruningStartCycle
        && totalTerms > ModelPruningConfig::kMinTermsToEnablePruning)
        probabilisticTermPruning(trial,
                                 ModelPruningConfig::kDefaultImportanceThreshold,
                                 ModelPruningConfig::kTrialRemovalRate);

    const qint32 size1 = trial.regress.size();
    if (size1) {
        for (i = 0; i < size1; i++) {
            const qint32 size2 = trial.regress.at(i).size();
            for (j = 0; j < size2; j++)
                if (trial.regress.at(i).at(j).vTermo.tTermo1.reg)
                    trial.regress[i][j].vTermo.tTermo1.reg = (size1 - i);
        }
    }
    return trial;
}

///////////////////////////////////////////////////////////////////////////////
/// buildOutput — DES_MontaSaida (EXATAMENTE igual ao original)
///////////////////////////////////////////////////////////////////////////////
void ChromosomeService::buildOutput(Cromossomo &cr,
                                     QVector<qreal> &vplotar,
                                     QVector<qreal> &resid) const
{
    JMathVar<qreal> vlrsRegress, vlrsEstimado, vlrsResiduo, vlrsMedido;
    montaVlrs(cr, vlrsRegress, vlrsMedido, true, false);
    calcVlrsEstRes(cr, vlrsRegress, cr.vlrsCoefic, vlrsMedido, vlrsResiduo, vlrsEstimado);
    vplotar.clear(); vplotar += (QVector<qreal>)vlrsEstimado;
    resid.clear(); resid += (QVector<qreal>)vlrsResiduo;
}

///////////////////////////////////////////////////////////////////////////////
/// Análise de Importância de Termos e Poda Probabilística
///////////////////////////////////////////////////////////////////////////////

bool ChromosomeService::calculateTermImportance(const Cromossomo &cr,
                                                 QVector<QVector<qreal>> &termImportance) const
{
    termImportance.clear();

    // Validação
    if (cr.regress.isEmpty()) {
        return false;
    }

    // MSE com todos os termos (baseline)
    const qreal mseBaseline = cr.erro;
    if (mseBaseline <= 0.0) {
        return false;
    }

    // Para cada regressor
    for (qint32 regIdx = 0; regIdx < cr.regress.size(); ++regIdx) {
        QVector<qreal> regTermImportance;
        const QVector<compTermo> &regressor = cr.regress.at(regIdx);

        // Para cada termo no regressor
        for (qint32 termIdx = 0; termIdx < regressor.size(); ++termIdx) {
            // Avaliar MSE sem este termo
            qreal mseWithoutTerm = evaluateWithoutTerm(cr, regIdx, termIdx);

            // Calcular importância (normalizada [0,1])
            qreal importance = 0.0;
            if (mseWithoutTerm > mseBaseline) {
                // Termo contribui positivamente
                importance = (mseWithoutTerm - mseBaseline) / mseBaseline;
                importance = qMin(1.0, qMax(0.0, importance)); // Clamp [0,1]
            } else {
                // Termo não piora (ou melhora) — baixa importância
                importance = 0.0;
            }

            regTermImportance.append(importance);
        }

        termImportance.append(regTermImportance);
    }

    return true;
}

qreal ChromosomeService::evaluateWithoutTerm(const Cromossomo &cr,
                                              qint32 regIdx,
                                              qint32 termIdx) const
{
    // Validação de índices
    if (regIdx < 0 || regIdx >= cr.regress.size()) {
        return -1.0;
    }
    if (termIdx < 0 || termIdx >= cr.regress.at(regIdx).size()) {
        return -1.0;
    }

    // Copiar cromossomo e remover termo
    Cromossomo crModified = cr;
    crModified.regress[regIdx].remove(termIdx);

    // Se regressor ficou vazio, retornar alto erro (penalidade)
    if (crModified.regress.at(regIdx).isEmpty()) {
        return ModelPruningConfig::kEmptyRegressorPenaltyMse;  // Penalidade alta
    }

    // Re-avaliar
    evaluate(crModified, 1);

    return crModified.erro;
}

qint32 ChromosomeService::probabilisticTermPruning(Cromossomo &cr,
                                                    qreal importanceThreshold,
                                                    qreal removalRate) const
{
    // Calcular importância de todos os termos
    QVector<QVector<qreal>> termImportance;
    if (!calculateTermImportance(cr, termImportance)) {
        return 0;
    }

    qint32 totalTermsRemoved = 0;

    // Para cada regressor
    for (qint32 regIdx = 0; regIdx < termImportance.size(); ++regIdx) {
        const QVector<qreal> &regTerms = termImportance.at(regIdx);
        qint32 maxRemovalsInReg = qMax(1, (qint32)qFloor(regTerms.size() * removalRate));
        qint32 removalsInReg = 0;

        // Iterar de trás para frente (evitar problemas de índice ao remover)
        for (qint32 termIdx = regTerms.size() - 1; termIdx >= 0; --termIdx) {
            qreal importance = regTerms.at(termIdx);

            // Probabilidade de remoção inversamente proporcional à importância
            qreal removalProbability = 0.0;
            if (importance < importanceThreshold) {
                // Termo pouco importante: probabilidade de remoção alta
                removalProbability = 1.0 - (importance / importanceThreshold);
            }

            // Roleta: sortear se remove
            if (m_rng.rand() < removalProbability && removalsInReg < maxRemovalsInReg) {
                if (cr.regress.at(regIdx).size() <= ModelPruningConfig::kMinTermsPerRegressor)
                    break;
                // Remover termo
                cr.regress[regIdx].remove(termIdx);
                removalsInReg++;
                totalTermsRemoved++;

                qDebug() << "Termo removido: Reg" << regIdx << "Term" << termIdx
                         << "Importância:" << importance;
            }
        }
    }

    // Re-avaliar cromossomo após remoções
    if (totalTermsRemoved > 0) {
        evaluate(cr, 1);
    }

    return totalTermsRemoved;
}
