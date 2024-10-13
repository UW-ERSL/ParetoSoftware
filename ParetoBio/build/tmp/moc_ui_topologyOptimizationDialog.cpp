/****************************************************************************
** Meta object code from reading C++ file 'ui_topologyOptimizationDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ui/inc/ui_topologyOptimizationDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_topologyOptimizationDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TopologyOptimizationDialog_t {
    QByteArrayData data[12];
    char stringdata0[160];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TopologyOptimizationDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TopologyOptimizationDialog_t qt_meta_stringdata_TopologyOptimizationDialog = {
    {
QT_MOC_LITERAL(0, 0, 26), // "TopologyOptimizationDialog"
QT_MOC_LITERAL(1, 27, 17), // "setTopOptComplete"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 18), // "setTopOptVariables"
QT_MOC_LITERAL(4, 65, 8), // "optimize"
QT_MOC_LITERAL(5, 74, 4), // "stop"
QT_MOC_LITERAL(6, 79, 11), // "closeDialog"
QT_MOC_LITERAL(7, 91, 21), // "updateLoadSetCallBack"
QT_MOC_LITERAL(8, 113, 22), // "onOptimizationFinished"
QT_MOC_LITERAL(9, 136, 9), // "showEvent"
QT_MOC_LITERAL(10, 146, 11), // "QShowEvent*"
QT_MOC_LITERAL(11, 158, 1) // "e"

    },
    "TopologyOptimizationDialog\0setTopOptComplete\0"
    "\0setTopOptVariables\0optimize\0stop\0"
    "closeDialog\0updateLoadSetCallBack\0"
    "onOptimizationFinished\0showEvent\0"
    "QShowEvent*\0e"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TopologyOptimizationDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   55,    2, 0x08 /* Private */,
       4,    0,   56,    2, 0x08 /* Private */,
       5,    0,   57,    2, 0x08 /* Private */,
       6,    0,   58,    2, 0x08 /* Private */,
       7,    0,   59,    2, 0x08 /* Private */,
       8,    0,   60,    2, 0x08 /* Private */,
       9,    1,   61,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void TopologyOptimizationDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TopologyOptimizationDialog *_t = static_cast<TopologyOptimizationDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setTopOptComplete(); break;
        case 1: _t->setTopOptVariables(); break;
        case 2: _t->optimize(); break;
        case 3: _t->stop(); break;
        case 4: _t->closeDialog(); break;
        case 5: _t->updateLoadSetCallBack(); break;
        case 6: _t->onOptimizationFinished(); break;
        case 7: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (TopologyOptimizationDialog::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&TopologyOptimizationDialog::setTopOptComplete)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject TopologyOptimizationDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_TopologyOptimizationDialog.data,
      qt_meta_data_TopologyOptimizationDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *TopologyOptimizationDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TopologyOptimizationDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_TopologyOptimizationDialog.stringdata0))
        return static_cast<void*>(const_cast< TopologyOptimizationDialog*>(this));
    if (!strcmp(_clname, "Ui_topologyOptimizationDialog"))
        return static_cast< Ui_topologyOptimizationDialog*>(const_cast< TopologyOptimizationDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int TopologyOptimizationDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void TopologyOptimizationDialog::setTopOptComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
