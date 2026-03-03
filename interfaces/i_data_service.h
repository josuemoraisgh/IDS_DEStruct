#ifndef I_DATA_SERVICE_H
#define I_DATA_SERVICE_H

#include <QtCore/QObject>

///////////////////////////////////////////////////////////////////////////////
/// Interface para serviço de dados (SRP: só carregar/normalizar)
/// DIP: UI e Engine dependem desta abstração, não da implementação concreta.
///////////////////////////////////////////////////////////////////////////////
class IDataService
{
public:
    virtual ~IDataService() = default;

    /// Carrega os dados do arquivo para o shared state (multi-thread)
    virtual void loadData() = 0;

    /// Normaliza os dados carregados (multi-thread)
    virtual void normalizeData() = 0;
};

#endif // I_DATA_SERVICE_H
