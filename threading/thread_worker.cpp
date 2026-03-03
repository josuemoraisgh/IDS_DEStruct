#include "thread_worker.h"
#include "shared_state.h"
#include "../interfaces/i_data_service.h"
#include "../interfaces/i_evolution_engine.h"
#include <QTime>
#include <QDebug>

///////////////////////////////////////////////////////////////////////////////
/// ThreadWorker — Implementação
///
/// O ciclo run() replica EXATAMENTE a máquina de estados do DEStruct::run()
/// original, mas delega cada operação ao serviço correspondente.
///////////////////////////////////////////////////////////////////////////////

ThreadWorker::ThreadWorker(SharedState *state, QObject *parent)
    : QThread(parent)
    , m_state(state)
    , m_dataService(nullptr)
    , m_evolutionEngine(nullptr)
    , m_isEquacaoEscrita(true)
    , m_isStatusSetado(true)
{
    m_state->mutex.lock();
    m_threadId = m_state->TH_size;

    if (m_threadId) {
        // Cria semáforos para threads adicionais (mesmo padrão original)
        for (qint32 c = 0; c < TAMPIPELINE; c++)
            m_state->justThread[c].release();
        m_state->waitThread.release();
    }
    m_state->TH_size++;
    m_state->mutex.unlock();

    m_rng.seed(QTime::currentTime().msec());

    for (int i = 0; i < TAMPIPELINE; ++i)
        m_idParada_Th[i] = false;

    start(); // Inicia thread (mesmo que original)
}

///////////////////////////////////////////////////////////////////////////////
ThreadWorker::~ThreadWorker()
{
    m_state->mutex.lock();
    m_state->Adj.modeOper_TH = 0;
    m_state->TH_size--;
    m_state->mutex.unlock();
    quit();
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::setDataService(IDataService *ds)
{
    m_dataService = ds;
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::setEvolutionEngine(IEvolutionEngine *engine)
{
    m_evolutionEngine = engine;
}

///////////////////////////////////////////////////////////////////////////////
/// Máquina de estados — idêntica ao DEStruct::run() original
///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::run()
{
    bool isOk = false;
    while (m_state->Adj.modeOper_TH) {
        switch (m_state->Adj.modeOper_TH) {
        case 0: // Termina a Thread
            break;

        case 1: // Thread espera próximo comando
            m_state->mutex.lock();
            if (m_state->waitThread.tryAcquire())
                m_state->waitSync.wait(&m_state->mutex);
            else {
                m_state->waitThread.release(m_state->TH_size - 1);
                emit signal_Finalizado();
                m_state->waitSync.wait(&m_state->mutex);
            }
            m_state->mutex.unlock();
            break;

        case 2: // Thread pausa
            m_state->mutex.lock();
            if (m_state->waitThread.tryAcquire())
                m_state->waitSync.wait(&m_state->mutex);
            else {
                m_state->waitThread.release(m_state->TH_size - 1);
                emit signal_Parado();
                m_state->waitSync.wait(&m_state->mutex);
            }
            m_state->mutex.unlock();
            break;

        case 3: // Executa o DE (delega ao engine)
            if (m_evolutionEngine)
                m_evolutionEngine->run();
            break;

        case 4: // Carrega dados (delega ao data service)
            if (m_dataService)
                m_dataService->loadData();
            break;

        case 5: // Normaliza dados (delega ao data service)
            if (m_dataService)
                m_dataService->normalizeData();
            break;
        }
    }

    // Apenas uma thread roda para fechar a tela principal (mesmo padrão)
    m_state->mutex.lock();
    m_state->waitSync.wakeAll();
    if (!m_state->waitThread.tryAcquire())
        isOk = true;
    m_state->mutex.unlock();
    if (isOk)
        emit signal_closed();
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::slot_Normalizar()
{
    if (m_dataService)
        m_dataService->normalizeData();
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::slot_EquacaoEscrita()
{
    m_isEquacaoEscrita = true;
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::slot_StatusSetado()
{
    m_isStatusSetado = true;
}

///////////////////////////////////////////////////////////////////////////////
void ThreadWorker::slot_Estado(const quint16 &std)
{
    m_state->setModoOperTH(std);
    m_state->waitSync.wakeAll();
}
