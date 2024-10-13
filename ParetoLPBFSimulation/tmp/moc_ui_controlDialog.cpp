/****************************************************************************
** Meta object code from reading C++ file 'ui_controlDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_controlDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_controlDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ControlDialog_t {
    QByteArrayData data[13];
    char stringdata0[235];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ControlDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ControlDialog_t qt_meta_stringdata_ControlDialog = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ControlDialog"
QT_MOC_LITERAL(1, 14, 17), // "openVerboseDialog"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 14), // "openUnitDialog"
QT_MOC_LITERAL(4, 48, 18), // "openGeometryDialog"
QT_MOC_LITERAL(5, 67, 18), // "openMaterialDialog"
QT_MOC_LITERAL(6, 86, 22), // "openThermalLoadsDialog"
QT_MOC_LITERAL(7, 109, 18), // "openAnalysisDialog"
QT_MOC_LITERAL(8, 128, 25), // "openStructuralLoadsDialog"
QT_MOC_LITERAL(9, 154, 24), // "openDisplayOptionsDialog"
QT_MOC_LITERAL(10, 179, 18), // "openProjectsDialog"
QT_MOC_LITERAL(11, 198, 18), // "closeControlDialog"
QT_MOC_LITERAL(12, 217, 17) // "updatePushButtons"

    },
    "ControlDialog\0openVerboseDialog\0\0"
    "openUnitDialog\0openGeometryDialog\0"
    "openMaterialDialog\0openThermalLoadsDialog\0"
    "openAnalysisDialog\0openStructuralLoadsDialog\0"
    "openDisplayOptionsDialog\0openProjectsDialog\0"
    "closeControlDialog\0updatePushButtons"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ControlDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x08 /* Private */,
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    0,   74,    2, 0x08 /* Private */,
       8,    0,   75,    2, 0x08 /* Private */,
       9,    0,   76,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    0,   79,    2, 0x0a /* Public */,

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

       0        // eod
};

void ControlDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ControlDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->openVerboseDialog(); break;
        case 1: _t->openUnitDialog(); break;
        case 2: _t->openGeometryDialog(); break;
        case 3: _t->openMaterialDialog(); break;
        case 4: _t->openThermalLoadsDialog(); break;
        case 5: _t->openAnalysisDialog(); break;
        case 6: _t->openStructuralLoadsDialog(); break;
        case 7: _t->openDisplayOptionsDialog(); break;
        case 8: _t->openProjectsDialog(); break;
        case 9: _t->closeControlDialog(); break;
        case 10: _t->updatePushButtons(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject ControlDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ControlDialog.data,
    qt_meta_data_ControlDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ControlDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ControlDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ControlDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_ControlDialog"))
        return static_cast< Ui_ControlDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int ControlDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
