/****************************************************************************
** Meta object code from reading C++ file 'ui_solidLatticeDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_solidLatticeDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_solidLatticeDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LatticeRadiiDialog_t {
    QByteArrayData data[9];
    char stringdata0[135];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LatticeRadiiDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LatticeRadiiDialog_t qt_meta_stringdata_LatticeRadiiDialog = {
    {
QT_MOC_LITERAL(0, 0, 18), // "LatticeRadiiDialog"
QT_MOC_LITERAL(1, 19, 19), // "createSolidLattice1"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 19), // "createSolidLattice2"
QT_MOC_LITERAL(4, 60, 23), // "closeLatticeRadiiDialog"
QT_MOC_LITERAL(5, 84, 26), // "loadLatticeRadiiFileDialog"
QT_MOC_LITERAL(6, 111, 9), // "showEvent"
QT_MOC_LITERAL(7, 121, 11), // "QShowEvent*"
QT_MOC_LITERAL(8, 133, 1) // "e"

    },
    "LatticeRadiiDialog\0createSolidLattice1\0"
    "\0createSolidLattice2\0closeLatticeRadiiDialog\0"
    "loadLatticeRadiiFileDialog\0showEvent\0"
    "QShowEvent*\0e"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LatticeRadiiDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    1,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void LatticeRadiiDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LatticeRadiiDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->createSolidLattice1(); break;
        case 1: _t->createSolidLattice2(); break;
        case 2: _t->closeLatticeRadiiDialog(); break;
        case 3: _t->loadLatticeRadiiFileDialog(); break;
        case 4: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LatticeRadiiDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_LatticeRadiiDialog.data,
    qt_meta_data_LatticeRadiiDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LatticeRadiiDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LatticeRadiiDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LatticeRadiiDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_LatticeRadiiDialog"))
        return static_cast< Ui_LatticeRadiiDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int LatticeRadiiDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
