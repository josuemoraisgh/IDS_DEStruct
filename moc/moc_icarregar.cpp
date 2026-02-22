/****************************************************************************
** Meta object code from reading C++ file 'icarregar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../icarregar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'icarregar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ICarregar_t {
    QByteArrayData data[24];
    char stringdata0[374];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ICarregar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ICarregar_t qt_meta_stringdata_ICarregar = {
    {
QT_MOC_LITERAL(0, 0, 9), // "ICarregar"
QT_MOC_LITERAL(1, 10, 20), // "signal_UL_Normalizar"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 18), // "signal_UL_Carregar"
QT_MOC_LITERAL(4, 51, 21), // "const QList<quint32>*"
QT_MOC_LITERAL(5, 73, 13), // "UL_cVariaveis"
QT_MOC_LITERAL(6, 87, 10), // "isCarregar"
QT_MOC_LITERAL(7, 98, 16), // "signal_UL_Estado"
QT_MOC_LITERAL(8, 115, 3), // "std"
QT_MOC_LITERAL(9, 119, 23), // "signal_UL_SalvarArquivo"
QT_MOC_LITERAL(10, 143, 15), // "signal_UL_FName"
QT_MOC_LITERAL(11, 159, 8), // "fileName"
QT_MOC_LITERAL(12, 168, 18), // "slot_UL_Finalizado"
QT_MOC_LITERAL(13, 187, 14), // "slot_UL_Parado"
QT_MOC_LITERAL(14, 202, 14), // "slot_UL_Status"
QT_MOC_LITERAL(15, 217, 16), // "slot_UL_Carregar"
QT_MOC_LITERAL(16, 234, 15), // "slot_UL_Indicar"
QT_MOC_LITERAL(17, 250, 18), // "slot_UL_Concatenar"
QT_MOC_LITERAL(18, 269, 17), // "slot_UL_Finalizar"
QT_MOC_LITERAL(19, 287, 15), // "slot_UL_Caminho"
QT_MOC_LITERAL(20, 303, 20), // "slot_UL_ModelClicked"
QT_MOC_LITERAL(21, 324, 11), // "slot_UL_Tam"
QT_MOC_LITERAL(22, 336, 19), // "slot_UL_ChangeCombo"
QT_MOC_LITERAL(23, 356, 17) // "slot_UL_ChangeFim"

    },
    "ICarregar\0signal_UL_Normalizar\0\0"
    "signal_UL_Carregar\0const QList<quint32>*\0"
    "UL_cVariaveis\0isCarregar\0signal_UL_Estado\0"
    "std\0signal_UL_SalvarArquivo\0signal_UL_FName\0"
    "fileName\0slot_UL_Finalizado\0slot_UL_Parado\0"
    "slot_UL_Status\0slot_UL_Carregar\0"
    "slot_UL_Indicar\0slot_UL_Concatenar\0"
    "slot_UL_Finalizar\0slot_UL_Caminho\0"
    "slot_UL_ModelClicked\0slot_UL_Tam\0"
    "slot_UL_ChangeCombo\0slot_UL_ChangeFim"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ICarregar[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   99,    2, 0x06 /* Public */,
       3,    2,  100,    2, 0x06 /* Public */,
       7,    1,  105,    2, 0x06 /* Public */,
       9,    0,  108,    2, 0x06 /* Public */,
      10,    1,  109,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,  112,    2, 0x0a /* Public */,
      13,    0,  113,    2, 0x0a /* Public */,
      14,    1,  114,    2, 0x0a /* Public */,
      15,    0,  117,    2, 0x08 /* Private */,
      16,    1,  118,    2, 0x08 /* Private */,
      17,    0,  121,    2, 0x08 /* Private */,
      18,    0,  122,    2, 0x08 /* Private */,
      19,    0,  123,    2, 0x08 /* Private */,
      20,    0,  124,    2, 0x08 /* Private */,
      21,    0,  125,    2, 0x08 /* Private */,
      22,    1,  126,    2, 0x08 /* Private */,
      23,    1,  129,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Bool,    5,    6,
    QMetaType::Void, QMetaType::UShort,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UShort,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void ICarregar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ICarregar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signal_UL_Normalizar(); break;
        case 1: _t->signal_UL_Carregar((*reinterpret_cast< const QList<quint32>*(*)>(_a[1])),(*reinterpret_cast< const bool(*)>(_a[2]))); break;
        case 2: _t->signal_UL_Estado((*reinterpret_cast< const quint16(*)>(_a[1]))); break;
        case 3: _t->signal_UL_SalvarArquivo(); break;
        case 4: _t->signal_UL_FName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->slot_UL_Finalizado(); break;
        case 6: _t->slot_UL_Parado(); break;
        case 7: _t->slot_UL_Status((*reinterpret_cast< const quint16(*)>(_a[1]))); break;
        case 8: _t->slot_UL_Carregar(); break;
        case 9: _t->slot_UL_Indicar((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->slot_UL_Concatenar(); break;
        case 11: _t->slot_UL_Finalizar(); break;
        case 12: _t->slot_UL_Caminho(); break;
        case 13: _t->slot_UL_ModelClicked(); break;
        case 14: _t->slot_UL_Tam(); break;
        case 15: _t->slot_UL_ChangeCombo((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->slot_UL_ChangeFim((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ICarregar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ICarregar::signal_UL_Normalizar)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ICarregar::*)(const QList<quint32> * , const bool & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ICarregar::signal_UL_Carregar)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ICarregar::*)(const quint16 & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ICarregar::signal_UL_Estado)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ICarregar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ICarregar::signal_UL_SalvarArquivo)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ICarregar::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ICarregar::signal_UL_FName)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ICarregar::staticMetaObject = { {
    QMetaObject::SuperData::link<DICarregar::staticMetaObject>(),
    qt_meta_stringdata_ICarregar.data,
    qt_meta_data_ICarregar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ICarregar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ICarregar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ICarregar.stringdata0))
        return static_cast<void*>(this);
    return DICarregar::qt_metacast(_clname);
}

int ICarregar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DICarregar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void ICarregar::signal_UL_Normalizar()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ICarregar::signal_UL_Carregar(const QList<quint32> * _t1, const bool & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ICarregar::signal_UL_Estado(const quint16 & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ICarregar::signal_UL_SalvarArquivo()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ICarregar::signal_UL_FName(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
