///////////////////////////////////////////////////////////////////////////////
/// destruct_statics_stub.cpp
///
/// Fornece definições dos membros estáticos de DEStruct que são referenciados
/// por icarregar.cpp (código original não modificado).
/// No build refatorado, DES_Adj aponta para m_state->Adj em SharedState.
/// Este stub mantém compatibilidade de link sem modificar icarregar.cpp.
///////////////////////////////////////////////////////////////////////////////

#include "destruct.h"

Config     DEStruct::DES_Adj;
QList<qint32> DEStruct::DES_cVariaveis;
bool       DEStruct::DES_isCarregar = false;
bool       DEStruct::DES_idParadaJust[TAMPIPELINE] = {};
