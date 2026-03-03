#ifndef XML_CONFIG_PERSISTENCE_H
#define XML_CONFIG_PERSISTENCE_H

#include "../interfaces/i_config_persistence.h"

class SharedState;

///////////////////////////////////////////////////////////////////////////////
/// Adapter que encapsula XmlReaderWriter e implementa IConfigPersistence
///////////////////////////////////////////////////////////////////////////////
class XmlConfigPersistence : public IConfigPersistence
{
public:
    explicit XmlConfigPersistence(SharedState *state);
    ~XmlConfigPersistence() override = default;

    bool save(QIODevice *device) override;
    bool load(QIODevice *device) override;
    QString errorString() const override;

private:
    SharedState *m_state;
    QString m_errorString;
};

#endif // XML_CONFIG_PERSISTENCE_H
