#ifndef I_DATA_SERVICE_H
#define I_DATA_SERVICE_H

#include <QtCore/QObject>

///////////////////////////////////////////////////////////////////////////////
/// Interface para servi\u00e7o de dados (SRP: s\u00f3 carregar/padronizar)
/// DIP: UI e Engine dependem desta abstra\u00e7\u00e3o, n\u00e3o da implementa\u00e7\u00e3o concreta.
///////////////////////////////////////////////////////////////////////////////
class IDataService
{
public:
    virtual ~IDataService() = default;

    /// Carrega os dados do arquivo para o shared state (multi-thread)
    virtual void loadData() = 0;

    /// Padroniza os dados carregados usando z-score (multi-thread)
    virtual void normalizeData() = 0;
};

#endif // I_DATA_SERVICE_H
