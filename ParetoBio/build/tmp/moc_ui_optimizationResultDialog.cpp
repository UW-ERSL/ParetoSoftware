/****************************************************************************
** Meta object code from reading C++ file 'ui_optimizationResultDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ui/inc/ui_optimizationResultDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_optimizationResultDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OptimizationResultDialog_t {
    QByteArrayData data[16];
    char stringdata0[205];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OptimizationResultDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OptimizationResultDialog_t qt_meta_stringdata_OptimizationResultDialog = {
    {
QT_MOC_LITERAL(0, 0, 24), // "OptimizationResultDialog"
QT_MOC_LITERAL(1, 25, 16), // "createHTMLReport"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 10), // "plotCGIter"
QT_MOC_LITERAL(4, 54, 19), // "plotComplianceCurve"
QT_MOC_LITERAL(5, 74, 15), // "plotStressCurve"
QT_MOC_LITERAL(6, 90, 14), // "plotEigenCurve"
QT_MOC_LITERAL(7, 105, 15), // "saveMeshSurface"
QT_MOC_LITERAL(8, 121, 8), // "saveMesh"
QT_MOC_LITERAL(9, 130, 8), // "loadMesh"
QT_MOC_LITERAL(10, 139, 16), // "setTopologyAtVol"
QT_MOC_LITERAL(11, 156, 12), // "saveTopology"
QT_MOC_LITERAL(12, 169, 11), // "closeDialog"
QT_MOC_LITERAL(13, 181, 9), // "showEvent"
QT_MOC_LITERAL(14, 191, 11), // "QShowEvent*"
QT_MOC_LITERAL(15, 203, 1) // "e"

    },
    "OptimizationResultDialog\0createHTMLReport\0"
    "\0plotCGIter\0plotComplianceCurve\0"
    "plotStressCurve\0plotEigenCurve\0"
    "saveMeshSurface\0saveMesh\0loadMesh\0"
    "setTopologyAtVol\0saveTopology\0closeDialog\0"
    "showEvent\0QShowEvent*\0e"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OptimizationResultDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x08 /* Private */,
       3,    0,   75,    2, 0x08 /* Private */,
       4,    0,   76,    2, 0x08 /* Private */,
       5,    0,   77,    2, 0x08 /* Private */,
       6,    0,   78,    2, 0x08 /* Private */,
       7,    0,   79,    2, 0x08 /* Private */,
       8,    0,   80,    2, 0x08 /* Private */,
       9,    0,   81,    2, 0x08 /* Private */,
      10,    0,   82,    2, 0x08 /* Private */,
      11,    0,   83,    2, 0x08 /* Private */,
      12,    0,   84,    2, 0x08 /* Private */,
      13,    1,   85,    2, 0x08 /* Private */,

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
    QMetaType::Void, 0x80000000 | 14,   15,

       0        // eod
};

void OptimizationResultDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        OptimizationResultDialog *_t = static_cast<OptimizationResultDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->createHTMLReport(); break;
        case 1: _t->plotCGIter(); break;
        case 2: _t->plotComplianceCurve(); break;
        case 3: _t->plotStressCurve(); break;
        case 4: _t->plotEigenCurve(); break;
        case 5: _t->saveMeshSurface(); break;
        case 6: _t->saveMesh(); break;
        case 7: _t->loadMesh(); break;
        case 8: _t->setTopologyAtVol(); break;
        case 9: _t->saveTopology(); break;
        case 10: _t->closeDialog(); break;
        case 11: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject OptimizationResultDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_OptimizationResultDialog.data,
      qt_meta_data_OptimizationResultDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *OptimizationResultDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OptimizationResultDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_OptimizationResultDialog.stringdata0))
        return static_cast<void*>(const_cast< OptimizationResultDialog*>(this));
    if (!strcmp(_clname, "Ui_OptimizationResultDialog"))
        return static_cast< Ui_OptimizationResultDialog*>(const_cast< OptimizationResultDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int OptimizationResultDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
