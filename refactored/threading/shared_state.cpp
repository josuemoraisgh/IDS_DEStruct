#include "shared_state.h"

///////////////////////////////////////////////////////////////////////////////
SharedState::SharedState()
    : tamArquivo(0)
    , isCarregar(false)
    , TH_size(0)
    , countSR(0)
{
    for (int i = 0; i < TAMPIPELINE; ++i) {
        index[i] = 0;
        idParadaJust[i] = false;
    }

    // Inicializa listas do pipeline (mesmo padrão do original)
    crMut     = QVector<QVector<Cromossomo> >(TAMPIPELINE).toList();
    somaSSE   = QVector<QVector<qreal> >(TAMPIPELINE).toList();
    idChange  = QVector<QVector<qint32> >(TAMPIPELINE).toList();
    vcalc     = QVector<QList<QVector<qreal> > >(TAMPIPELINE).toList();
    residuos  = QVector<QList<QVector<qreal> > >(TAMPIPELINE).toList();
    BufferSR  = QVector<QList<QVector<Cromossomo> > >(TAMPIPELINE).toList();

    // Config defaults
    Adj.modeOper_TH  = 1;
    Adj.vetPop       = QVector<QList<qint32> >(TAMPIPELINE).toList();
    Adj.vetElitismo  = QVector<QList<QVector<qint32> > >(TAMPIPELINE).toList();
    Adj.isSR         = QVector<QList<QList<bool> > >(TAMPIPELINE).toList();
    Adj.isCriado     = false;

    Adj.Dados.variaveis.nome.clear();
    Adj.Dados.variaveis.valores.clear();
    Adj.Dados.variaveis.Vmaior.clear();
    Adj.Dados.variaveis.Vmenor.clear();
}

///////////////////////////////////////////////////////////////////////////////
qint16 SharedState::modoOperTH()
{
    qint16 resp;
    lock_modeOper_TH.lockForWrite();
    resp = Adj.modeOper_TH;
    lock_modeOper_TH.unlock();
    return resp;
}

///////////////////////////////////////////////////////////////////////////////
void SharedState::setModoOperTH(qint16 mode)
{
    lock_modeOper_TH.lockForWrite();
    Adj.modeOper_TH = mode;
    lock_modeOper_TH.unlock();
}
