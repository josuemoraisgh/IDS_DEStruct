#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H

#include <QThread>
#include "mtrand.h"
#include "xtipodados.h"

#ifndef TAMPIPELINE
#define TAMPIPELINE 4
#endif

class SharedState;
class IDataService;
class IEvolutionEngine;

///////////////////////////////////////////////////////////////////////////////
/// ThreadWorker — Thread de trabalho (herda QThread como o DEStruct original).
///
/// SRP: Responsabilidade única = gerenciar o ciclo de vida da thread.
///      DELEGA carga de dados para IDataService.
///      DELEGA algoritmo DE para IEvolutionEngine.
///
/// OCP: Novas operações podem ser adicionadas sem modificar esta classe,
///      bastando registrar novos serviços.
///////////////////////////////////////////////////////////////////////////////
class ThreadWorker : public QThread
{
    Q_OBJECT

public:
    ThreadWorker(SharedState *state, QObject *parent = nullptr);
    ~ThreadWorker();

    /// Injeta dependências (DIP) — deve ser chamado após construção
    void setDataService(IDataService *ds);
    void setEvolutionEngine(IEvolutionEngine *engine);

    SharedState *sharedState() const { return m_state; }
    qint32 threadId() const { return m_threadId; }

signals:
    void signal_EscreveEquacao();
    void signal_closed();
    void signal_Finalizado();
    void signal_Parado();
    void signal_SetStatus(const volatile qint64 &iteracoes,
                          const QVector<qreal> *somaEr,
                          const QList<QVector<qreal> > *resObtido,
                          const QList<QVector<qreal> > *residuo,
                          const QVector<Cromossomo> *crBest) const;
    void signal_Desenha() const;
    void signal_Finalizar() const;
    void signal_Status(const quint16 index) const;
    void signal_Tam();

private slots:
    void slot_Normalizar();
    void slot_EquacaoEscrita();
    void slot_StatusSetado();
    void slot_Estado(const quint16 &std);

protected:
    void run() override;

private:
    SharedState        *m_state;
    IDataService       *m_dataService;
    IEvolutionEngine   *m_evolutionEngine;
    qint32              m_threadId;
    bool                m_idParada_Th[TAMPIPELINE];
    volatile bool       m_isEquacaoEscrita;
    volatile bool       m_isStatusSetado;
    MTRand              m_rng;
};

#endif // THREAD_WORKER_H
