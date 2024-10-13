/****************************************************************************
** Meta object code from reading C++ file 'ui_controlDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Ui/inc/ui_controlDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_controlDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ControlDialog_t {
    QByteArrayData data[15];
    char stringdata0[284];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ControlDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ControlDialog_t qt_meta_stringdata_ControlDialog = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ControlDialog"
QT_MOC_LITERAL(1, 14, 14), // "openUnitDialog"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 18), // "openGeometryDialog"
QT_MOC_LITERAL(4, 49, 18), // "openMaterialDialog"
QT_MOC_LITERAL(5, 68, 29), // "openStructuralBoundCondDialog"
QT_MOC_LITERAL(6, 98, 26), // "openThermalBoundCondDialog"
QT_MOC_LITERAL(7, 125, 22), // "openAccelerationDialog"
QT_MOC_LITERAL(8, 148, 24), // "openDisplayOptionsDialog"
QT_MOC_LITERAL(9, 173, 13), // "openFEADialog"
QT_MOC_LITERAL(10, 187, 21), // "openConstraintsDialog"
QT_MOC_LITERAL(11, 209, 18), // "openOptimizeDialog"
QT_MOC_LITERAL(12, 228, 17), // "openResultsDialog"
QT_MOC_LITERAL(13, 246, 18), // "openProjectsDialog"
QT_MOC_LITERAL(14, 265, 18) // "closeControlDialog"

    },
    "ControlDialog\0openUnitDialog\0\0"
    "openGeometryDialog\0openMaterialDialog\0"
    "openStructuralBoundCondDialog\0"
    "openThermalBoundCondDialog\0"
    "openAccelerationDialog\0openDisplayOptionsDialog\0"
    "openFEADialog\0openConstraintsDialog\0"
    "openOptimizeDialog\0openResultsDialog\0"
    "openProjectsDialog\0closeControlDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ControlDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    0,   90,    2, 0x08 /* Private */,
      14,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ControlDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ControlDialog *_t = static_cast<ControlDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->openUnitDialog(); break;
        case 1: _t->openGeometryDialog(); break;
        case 2: _t->openMaterialDialog(); break;
        case 3: _t->openStructuralBoundCondDialog(); break;
        case 4: _t->openThermalBoundCondDialog(); break;
        case 5: _t->openAccelerationDialog(); break;
        case 6: _t->openDisplayOptionsDialog(); break;
        case 7: _t->openFEADialog(); break;
        case 8: _t->openConstraintsDialog(); break;
        case 9: _t->openOptimizeDialog(); break;
        case 10: _t->openResultsDialog(); break;
        case 11: _t->openProjectsDialog(); break;
        case 12: _t->closeControlDialog(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ControlDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ControlDialog.data,
      qt_meta_data_ControlDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ControlDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ControlDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ControlDialog.stringdata0))
        return static_cast<void*>(const_cast< ControlDialog*>(this));
    if (!strcmp(_clname, "Ui_ControlDialog"))
        return static_cast< Ui_ControlDialog*>(const_cast< ControlDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ControlDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
