/****************************************************************************
** Meta object code from reading C++ file 'ui_latticeStructureDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ui/inc/ui_latticeStructureDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_latticeStructureDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LatticeStructureDialog_t {
    QByteArrayData data[9];
    char stringdata0[143];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LatticeStructureDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LatticeStructureDialog_t qt_meta_stringdata_LatticeStructureDialog = {
    {
QT_MOC_LITERAL(0, 0, 22), // "LatticeStructureDialog"
QT_MOC_LITERAL(1, 23, 19), // "createLatticeDesign"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 27), // "closeLatticeStructureDialog"
QT_MOC_LITERAL(4, 72, 9), // "showEvent"
QT_MOC_LITERAL(5, 82, 11), // "QShowEvent*"
QT_MOC_LITERAL(6, 94, 1), // "e"
QT_MOC_LITERAL(7, 96, 19), // "updateLiveVariables"
QT_MOC_LITERAL(8, 116, 26) // "stopLatticeStructureDesign"

    },
    "LatticeStructureDialog\0createLatticeDesign\0"
    "\0closeLatticeStructureDialog\0showEvent\0"
    "QShowEvent*\0e\0updateLiveVariables\0"
    "stopLatticeStructureDesign"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LatticeStructureDialog[] = {

 // content:
       7,       // revision
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
       4,    1,   41,    2, 0x08 /* Private */,
       7,    0,   44,    2, 0x08 /* Private */,
       8,    0,   45,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void LatticeStructureDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LatticeStructureDialog *_t = static_cast<LatticeStructureDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->createLatticeDesign(); break;
        case 1: _t->closeLatticeStructureDialog(); break;
        case 2: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 3: _t->updateLiveVariables(); break;
        case 4: _t->stopLatticeStructureDesign(); break;
        default: ;
        }
    }
}

const QMetaObject LatticeStructureDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_LatticeStructureDialog.data,
      qt_meta_data_LatticeStructureDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *LatticeStructureDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LatticeStructureDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_LatticeStructureDialog.stringdata0))
        return static_cast<void*>(const_cast< LatticeStructureDialog*>(this));
    if (!strcmp(_clname, "Ui_LatticeStructureDialog"))
        return static_cast< Ui_LatticeStructureDialog*>(const_cast< LatticeStructureDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int LatticeStructureDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
