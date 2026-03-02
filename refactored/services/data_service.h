#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "../interfaces/i_data_service.h"
#include <QObject>

class SharedState;
class ThreadWorker;

///////////////////////////////////////////////////////////////////////////////
/// DataService — Implementa IDataService
///
/// SRP: Responsabilidade única = carregar e normalizar dados de arquivo.
/// Extrai DES_Carregar() e slot_DES_Normalizar() do DEStruct original.
/// DIP: Depende de SharedState (injetado).
///////////////////////////////////////////////////////////////////////////////
class DataService : public IDataService
{
public:
    DataService(SharedState *state, ThreadWorker *worker);

    void loadData() override;
    void normalizeData() override;

private:
    SharedState  *m_state;
    ThreadWorker *m_worker;
};

#endif // DATA_SERVICE_H
