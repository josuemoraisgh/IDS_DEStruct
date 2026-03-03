#ifndef I_CONFIG_PERSISTENCE_H
#define I_CONFIG_PERSISTENCE_H

#include <QIODevice>
#include <QString>

struct Config;

///////////////////////////////////////////////////////////////////////////////
/// Interface para persistência de configuração (SRP + DIP)
///////////////////////////////////////////////////////////////////////////////
class IConfigPersistence
{
public:
    virtual ~IConfigPersistence() = default;
    virtual bool save(QIODevice *device) = 0;
    virtual bool load(QIODevice *device) = 0;
    virtual QString errorString() const = 0;
};

#endif // I_CONFIG_PERSISTENCE_H
