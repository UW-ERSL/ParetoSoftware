/****************************************************************************
** Meta object code from reading C++ file 'ui_structuralAnalysisDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_structuralAnalysisDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_structuralAnalysisDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StructuralAnalysisDialog_t {
    QByteArrayData data[20];
    char stringdata0[385];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StructuralAnalysisDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StructuralAnalysisDialog_t qt_meta_stringdata_StructuralAnalysisDialog = {
    {
QT_MOC_LITERAL(0, 0, 24), // "StructuralAnalysisDialog"
QT_MOC_LITERAL(1, 25, 14), // "setFeaComplete"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 18), // "solveThermalStatic"
QT_MOC_LITERAL(4, 60, 24), // "solveLinearThermalStatic"
QT_MOC_LITERAL(5, 85, 27), // "solveNonLinearThermalStatic"
QT_MOC_LITERAL(6, 113, 21), // "solveThermalTransient"
QT_MOC_LITERAL(7, 135, 27), // "solveLinearThermalTransient"
QT_MOC_LITERAL(8, 163, 30), // "solveNonLinearThermalTransient"
QT_MOC_LITERAL(9, 194, 21), // "solveStructuralStatic"
QT_MOC_LITERAL(10, 216, 20), // "solveStructuralModal"
QT_MOC_LITERAL(11, 237, 13), // "onFEAFinished"
QT_MOC_LITERAL(12, 251, 9), // "showEvent"
QT_MOC_LITERAL(13, 261, 11), // "QShowEvent*"
QT_MOC_LITERAL(14, 273, 1), // "e"
QT_MOC_LITERAL(15, 275, 29), // "closeStructuralAnalysisDialog"
QT_MOC_LITERAL(16, 305, 21), // "updateLoadSetCallBack"
QT_MOC_LITERAL(17, 327, 14), // "remeshCallBack"
QT_MOC_LITERAL(18, 342, 19), // "updateLiveVariables"
QT_MOC_LITERAL(19, 362, 22) // "updateNumberOfElements"

    },
    "StructuralAnalysisDialog\0setFeaComplete\0"
    "\0solveThermalStatic\0solveLinearThermalStatic\0"
    "solveNonLinearThermalStatic\0"
    "solveThermalTransient\0solveLinearThermalTransient\0"
    "solveNonLinearThermalTransient\0"
    "solveStructuralStatic\0solveStructuralModal\0"
    "onFEAFinished\0showEvent\0QShowEvent*\0"
    "e\0closeStructuralAnalysisDialog\0"
    "updateLoadSetCallBack\0remeshCallBack\0"
    "updateLiveVariables\0updateNumberOfElements"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StructuralAnalysisDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   95,    2, 0x08 /* Private */,
       4,    0,   96,    2, 0x08 /* Private */,
       5,    0,   97,    2, 0x08 /* Private */,
       6,    0,   98,    2, 0x08 /* Private */,
       7,    0,   99,    2, 0x08 /* Private */,
       8,    0,  100,    2, 0x08 /* Private */,
       9,    0,  101,    2, 0x08 /* Private */,
      10,    0,  102,    2, 0x08 /* Private */,
      11,    0,  103,    2, 0x08 /* Private */,
      12,    1,  104,    2, 0x08 /* Private */,
      15,    0,  107,    2, 0x08 /* Private */,
      16,    0,  108,    2, 0x08 /* Private */,
      17,    0,  109,    2, 0x08 /* Private */,
      18,    0,  110,    2, 0x08 /* Private */,
      19,    0,  111,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

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
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void StructuralAnalysisDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StructuralAnalysisDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setFeaComplete(); break;
        case 1: _t->solveThermalStatic(); break;
        case 2: _t->solveLinearThermalStatic(); break;
        case 3: _t->solveNonLinearThermalStatic(); break;
        case 4: _t->solveThermalTransient(); break;
        case 5: _t->solveLinearThermalTransient(); break;
        case 6: _t->solveNonLinearThermalTransient(); break;
        case 7: _t->solveStructuralStatic(); break;
        case 8: _t->solveStructuralModal(); break;
        case 9: _t->onFEAFinished(); break;
        case 10: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 11: _t->closeStructuralAnalysisDialog(); break;
        case 12: _t->updateLoadSetCallBack(); break;
        case 13: _t->remeshCallBack(); break;
        case 14: _t->updateLiveVariables(); break;
        case 15: _t->updateNumberOfElements(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StructuralAnalysisDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StructuralAnalysisDialog::setFeaComplete)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StructuralAnalysisDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_StructuralAnalysisDialog.data,
    qt_meta_data_StructuralAnalysisDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StructuralAnalysisDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StructuralAnalysisDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StructuralAnalysisDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_StructuralAnalysisDialog"))
        return static_cast< Ui_StructuralAnalysisDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int StructuralAnalysisDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void StructuralAnalysisDialog::setFeaComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
