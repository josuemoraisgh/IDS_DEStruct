#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <QVector>
#include <QString>
#include <QSemaphore>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>
#include <QList>

#include "xtipodados.h"

///////////////////////////////////////////////////////////////////////////////
/// SharedState — Centraliza todo o estado compartilhado entre threads.
///
/// SRP: Responsabilidade única = manter o estado global e primitivas de sync.
/// Antes disperso como membros estáticos de DEStruct.
///////////////////////////////////////////////////////////////////////////////

#define TAMPIPELINE 4

class SharedState
{
public:
    SharedState();
    ~SharedState() = default;

    // ─── Primitivas de sincronização ───────────────────────────────────
    QSemaphore     justThread[TAMPIPELINE];
    QSemaphore     waitThread;
    QReadWriteLock lock_index[TAMPIPELINE];
    QReadWriteLock lock_Elitismo[TAMPIPELINE];
    QReadWriteLock lock_BufferSR;
    QReadWriteLock lock_modeOper_TH;
    QReadWriteLock lockLerDados;
    QWaitCondition justSync;
    QWaitCondition waitSync;
    QMutex         mutex;

    // ─── Dados do pipeline ─────────────────────────────────────────────
    QList<QList<QVector<Cromossomo> > > BufferSR;
    QList<QVector<qint32> >             idChange;
    volatile qint32                     index[TAMPIPELINE];
    volatile qint64                     tamArquivo;
    QString                             fileName;
    QList<qint32>                      &cVariaveis;
    QList<QVector<Cromossomo> >         crMut;
    QList<QVector<qreal> >              somaSSE;
    QList<qreal>                        mediaY, mediaY2;
    QList<QList<QVector<qreal> > >      residuos, vcalc;
    bool                               &isCarregar;
    bool                                idParadaJust[TAMPIPELINE];

    // ─── Configuração central (referência ao estático DEStruct::DES_Adj) ──
    Config                             &Adj;

    // ─── Thread management ─────────────────────────────────────────────
    volatile qint16                     TH_size;
    volatile qint16                     countSR;

    // ─── Helpers thread-safe ───────────────────────────────────────────
    qint16 modoOperTH();
    void   setModoOperTH(qint16 mode);
};

#endif // SHARED_STATE_H
