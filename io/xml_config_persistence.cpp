#include "xml_config_persistence.h"
#include "../threading/shared_state.h"
#include "xmlreaderwriter.h"

XmlConfigPersistence::XmlConfigPersistence(SharedState *state)
    : m_state(state)
{
}

bool XmlConfigPersistence::save(QIODevice *device)
{
    XmlReaderWriter writer(&m_state->Adj);
    bool ok = writer.writeFile(device);
    if (!ok) m_errorString = writer.errorString();
    return ok;
}

bool XmlConfigPersistence::load(QIODevice *device)
{
    XmlReaderWriter reader(&m_state->Adj);
    bool ok = reader.readFile(device);
    if (!ok) m_errorString = reader.errorString();
    return ok;
}

QString XmlConfigPersistence::errorString() const
{
    return m_errorString;
}
