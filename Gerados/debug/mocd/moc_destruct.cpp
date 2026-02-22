/****************************************************************************
** Meta object code from reading C++ file 'destruct.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../destruct.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'destruct.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DEStruct_t {
    QByteArrayData data[26];
    char stringdata[417];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DEStruct_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DEStruct_t qt_meta_stringdata_DEStruct = {
    {
QT_MOC_LITERAL(0, 0, 8), // "DEStruct"
QT_MOC_LITERAL(1, 9, 25), // "signal_DES_EscreveEquacao"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 17), // "signal_DES_closed"
QT_MOC_LITERAL(4, 54, 21), // "signal_DES_Finalizado"
QT_MOC_LITERAL(5, 76, 17), // "signal_DES_Parado"
QT_MOC_LITERAL(6, 94, 20), // "signal_DES_SetStatus"
QT_MOC_LITERAL(7, 115, 15), // "volatile qint64"
QT_MOC_LITERAL(8, 131, 9), // "iteracoes"
QT_MOC_LITERAL(9, 141, 21), // "const QVector<qreal>*"
QT_MOC_LITERAL(10, 163, 6), // "somaEr"
QT_MOC_LITERAL(11, 170, 29), // "const QList<QVector<qreal> >*"
QT_MOC_LITERAL(12, 200, 9), // "resObtido"
QT_MOC_LITERAL(13, 210, 7), // "residuo"
QT_MOC_LITERAL(14, 218, 26), // "const QVector<Cromossomo>*"
QT_MOC_LITERAL(15, 245, 6), // "crBest"
QT_MOC_LITERAL(16, 252, 18), // "signal_DES_Desenha"
QT_MOC_LITERAL(17, 271, 20), // "signal_DES_Finalizar"
QT_MOC_LITERAL(18, 292, 17), // "signal_DES_Status"
QT_MOC_LITERAL(19, 310, 5), // "index"
QT_MOC_LITERAL(20, 316, 14), // "signal_DES_Tam"
QT_MOC_LITERAL(21, 331, 19), // "slot_DES_Normalizar"
QT_MOC_LITERAL(22, 351, 23), // "slot_DES_EquacaoEscrita"
QT_MOC_LITERAL(23, 375, 21), // "slot_DES_StatusSetado"
QT_MOC_LITERAL(24, 397, 15), // "slot_DES_Estado"
QT_MOC_LITERAL(25, 413, 3) // "std"

    },
    "DEStruct\0signal_DES_EscreveEquacao\0\0"
    "signal_DES_closed\0signal_DES_Finalizado\0"
    "signal_DES_Parado\0signal_DES_SetStatus\0"
    "volatile qint64\0iteracoes\0"
    "const QVector<qreal>*\0somaEr\0"
    "const QList<QVector<qreal> >*\0resObtido\0"
    "residuo\0const QVector<Cromossomo>*\0"
    "crBest\0signal_DES_Desenha\0"
    "signal_DES_Finalizar\0signal_DES_Status\0"
    "index\0signal_DES_Tam\0slot_DES_Normalizar\0"
    "slot_DES_EquacaoEscrita\0slot_DES_StatusSetado\0"
    "slot_DES_Estado\0std"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DEStruct[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    0,   80,    2, 0x06 /* Public */,
       4,    0,   81,    2, 0x06 /* Public */,
       5,    0,   82,    2, 0x06 /* Public */,
       6,    5,   83,    2, 0x06 /* Public */,
      16,    0,   94,    2, 0x06 /* Public */,
      17,    0,   95,    2, 0x06 /* Public */,
      18,    1,   96,    2, 0x06 /* Public */,
      20,    0,   99,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      21,    0,  100,    2, 0x08 /* Private */,
      22,    0,  101,    2, 0x08 /* Private */,
      23,    0,  102,    2, 0x08 /* Private */,
      24,    1,  103,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 9, 0x80000000 | 11, 0x80000000 | 11, 0x80000000 | 14,    8,   10,   12,   13,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UShort,   19,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UShort,   25,

       0        // eod
};

void DEStruct::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DEStruct *_t = static_cast<DEStruct *>(_o);
        switch (_id) {
        case 0: _t->signal_DES_EscreveEquacao(); break;
        case 1: _t->signal_DES_closed(); break;
        case 2: _t->signal_DES_Finalizado(); break;
        case 3: _t->signal_DES_Parado(); break;
        case 4: _t->signal_DES_SetStatus((*reinterpret_cast< const volatile qint64(*)>(_a[1])),(*reinterpret_cast< const QVector<qreal>*(*)>(_a[2])),(*reinterpret_cast< const QList<QVector<qreal> >*(*)>(_a[3])),(*reinterpret_cast< const QList<QVector<qreal> >*(*)>(_a[4])),(*reinterpret_cast< const QVector<Cromossomo>*(*)>(_a[5]))); break;
        case 5: _t->signal_DES_Desenha(); break;
        case 6: _t->signal_DES_Finalizar(); break;
        case 7: _t->signal_DES_Status((*reinterpret_cast< const quint16(*)>(_a[1]))); break;
        case 8: _t->signal_DES_Tam(); break;
        case 9: _t->slot_DES_Normalizar(); break;
        case 10: _t->slot_DES_EquacaoEscrita(); break;
        case 11: _t->slot_DES_StatusSetado(); break;
        case 12: _t->slot_DES_Estado((*reinterpret_cast< const quint16(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DEStruct::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_EscreveEquacao)) {
                *result = 0;
            }
        }
        {
            typedef void (DEStruct::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_closed)) {
                *result = 1;
            }
        }
        {
            typedef void (DEStruct::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Finalizado)) {
                *result = 2;
            }
        }
        {
            typedef void (DEStruct::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Parado)) {
                *result = 3;
            }
        }
        {
            typedef void (DEStruct::*_t)(const volatile qint64 & , const QVector<qreal> * , const QList<QVector<qreal> > * , const QList<QVector<qreal> > * , const QVector<Cromossomo> * ) const;
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_SetStatus)) {
                *result = 4;
            }
        }
        {
            typedef void (DEStruct::*_t)() const;
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Desenha)) {
                *result = 5;
            }
        }
        {
            typedef void (DEStruct::*_t)() const;
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Finalizar)) {
                *result = 6;
            }
        }
        {
            typedef void (DEStruct::*_t)(const quint16 ) const;
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Status)) {
                *result = 7;
            }
        }
        {
            typedef void (DEStruct::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DEStruct::signal_DES_Tam)) {
                *result = 8;
            }
        }
    }
}

const QMetaObject DEStruct::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_DEStruct.data,
      qt_meta_data_DEStruct,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DEStruct::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DEStruct::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DEStruct.stringdata))
        return static_cast<void*>(const_cast< DEStruct*>(this));
    return QThread::qt_metacast(_clname);
}

int DEStruct::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void DEStruct::signal_DES_EscreveEquacao()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void DEStruct::signal_DES_closed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void DEStruct::signal_DES_Finalizado()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void DEStruct::signal_DES_Parado()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void DEStruct::signal_DES_SetStatus(const volatile qint64 & _t1, const QVector<qreal> * _t2, const QList<QVector<qreal> > * _t3, const QList<QVector<qreal> > * _t4, const QVector<Cromossomo> * _t5)const
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const volatile void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(const_cast< DEStruct *>(this), &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DEStruct::signal_DES_Desenha()const
{
    QMetaObject::activate(const_cast< DEStruct *>(this), &staticMetaObject, 5, Q_NULLPTR);
}

// SIGNAL 6
void DEStruct::signal_DES_Finalizar()const
{
    QMetaObject::activate(const_cast< DEStruct *>(this), &staticMetaObject, 6, Q_NULLPTR);
}

// SIGNAL 7
void DEStruct::signal_DES_Status(const quint16 _t1)const
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(const_cast< DEStruct *>(this), &staticMetaObject, 7, _a);
}

// SIGNAL 8
void DEStruct::signal_DES_Tam()
{
    QMetaObject::activate(this, &staticMetaObject, 8, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
