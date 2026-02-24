#ifndef AGSTRUCT_H
#define AGSTRUCT_H

#include <QVector>
#include <QString>
#include <QThread>
#include <QSemaphore>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>

#include <QtCore/QObject>

#include "mtrand.h"
#include "xtipodados.h"

#define TAMPIPELINE 4
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
class QTextStream;
class QFile;

class DEStruct : public QThread
{

    Q_OBJECT

public:
    ////////////////////////////////////////////////////////////////////////////
    DEStruct();
    ////////////////////////////////////////////////////////////////////////////
    ~DEStruct();
    ////////////////////////////////////////////////////////////////////////////
    static QSemaphore DES_justThread[TAMPIPELINE],DES_waitThread;
    static QReadWriteLock lock_DES_index[TAMPIPELINE],
                          lock_DES_Elitismo[TAMPIPELINE],
                          lock_DES_BufferSR,
                          lock_DES_modeOper_TH,
                          LerDados;
                          //lock_DES_pop[TAMPIPELINE],
                          //lock_DES_Pior[TAMPIPELINE],
    static QWaitCondition justSync,waitSync;
    static QMutex         mutex;
    ////////////////////////////////////////////////////////////////////////////
    static QList<QList<QVector<Cromossomo> > > DES_BufferSR;
    static QList<QVector<qint32 > > DES_idChange;
    static volatile qint32 DES_index[TAMPIPELINE];
    static volatile qint64 tamArquivo;//,DES_iteracoes;
    static QString DES_fileName;
    static QList<qint32> DES_cVariaveis;
    static QList<QVector<Cromossomo > > DES_crMut;
    static QList<QVector<qreal> > DES_somaSSE;
    static QList<qreal > DES_mediaY,DES_mediaY2;
    static QList<QList<QVector<qreal> > > DES_residuos,
                                          DES_vcalc;
    static bool DES_isCarregar,DES_idParadaJust[TAMPIPELINE];//DES_isDesenhar,
    //static QTime DES_tp;
    static Config DES_Adj;
    MTRand DES_RG;
    ////////////////////////////////////////////////////////////////////////////
    void qSortPop(qint32 *start, qint32 *end, const qint32 &idSaida) const;
    ////////////////////////////////////////////////////////////////////////////
    const Cromossomo DES_criaCromossomo(const qint32 &idSaida) const;
    void DES_CruzMut(Cromossomo &crAvali,  const Cromossomo &cr0, const Cromossomo &crNew, const Cromossomo &cr1, const Cromossomo &cr2) const;
    void DES_CalcERR(Cromossomo &cr,const qreal &metodoSerr) const;
    void DES_MontaVlrs(Cromossomo &cr,JMathVar<qreal> &vlrsRegress,JMathVar<qreal> &vlrsMedido,const bool &isValidacao=false,const bool &isLinearCoef=true) const;
    void DES_CalcVlrsEstRes(const Cromossomo &cr,const JMathVar<qreal> &vlrsRegress,const JMathVar<qreal> &vlrsCoefic,const JMathVar<qreal> &vlrsMedido,JMathVar<qreal> &vlrsResiduo,JMathVar<qreal> &vlrsEstimado) const;
    ////////////////////////////////////////////////////////////////////////////
    void DES_calAptidao(Cromossomo &cr, const quint32 &tamErro=1) const;
    void DES_MontaSaida(Cromossomo &cr, QVector<qreal> &vplotar, QVector<qreal> &resid) const;
    ////////////////////////////////////////////////////////////////////////////
    void DES_removeTermo(Cromossomo &cr,const qint32 &indexTermo) const;
    void DES_removeRegress(Cromossomo &cr,const qint32 &indRegress) const;
    ////////////////////////////////////////////////////////////////////////////
    void run();
    ////////////////////////////////////////////////////////////////////////////
    static qint16 DES_modo_Oper_TH()
    {
        qint16 resp;
        lock_DES_modeOper_TH.lockForWrite();
        resp = DES_Adj.modeOper_TH;
        lock_DES_modeOper_TH.unlock();
        return resp;
    }
signals:
    void signal_DES_EscreveEquacao();
    void signal_DES_closed();
    void signal_DES_Finalizado();
    void signal_DES_Parado();
    void signal_DES_SetStatus(const volatile qint64 &iteracoes,const QVector<qreal> *somaEr,const QList<QVector<qreal> > *resObtido,const QList<QVector<qreal> > *residuo,const QVector<Cromossomo> *crBest) const;
    void signal_DES_Desenha() const;
    void signal_DES_Finalizar() const;
    void signal_DES_Status(const quint16 index) const;
    void signal_DES_Tam();
    ////////////////////////////////////////////////////////////////////////////
private slots:
    void slot_DES_Normalizar();
    void slot_DES_EquacaoEscrita();
    void slot_DES_StatusSetado();
    void slot_DES_Estado(const quint16 &std);
    ////////////////////////////////////////////////////////////////////////////
private:
    //QWaitCondition waitSync;
    XMatriz<qreal> *DES_vlrRegressores;
    volatile bool DES_isEquacaoEscrita,DES_isStatusSetado;
    void DES_Carregar();
    void DES_AlgDiffEvol();
    const JMathVar<qreal> MultMatrizResiduo(Cromossomo &cr,JMathVar<qreal> &VetResiduo, const JMathVar<qreal> &matrizRegress, const JMathVar<qreal> &vetorCoefic, const JMathVar<qreal> &vetorMedido, const QVector<QVector<qreal> > &matResiduo) const;
    static volatile qint16 DES_TH_size,DES_countSR;//,DES_countPipe;
    qint32 DES_TH_id;
    bool DES_idParada_Th[TAMPIPELINE];
};
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#endif // AGSTRUCT_H
