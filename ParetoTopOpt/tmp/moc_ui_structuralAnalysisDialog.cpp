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
    QByteArrayData data[15];
    char stringdata0[251];
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
QT_MOC_LITERAL(4, 60, 21), // "solveStructuralStatic"
QT_MOC_LITERAL(5, 82, 20), // "solveStructuralModal"
QT_MOC_LITERAL(6, 103, 13), // "onFEAFinished"
QT_MOC_LITERAL(7, 117, 9), // "showEvent"
QT_MOC_LITERAL(8, 127, 11), // "QShowEvent*"
QT_MOC_LITERAL(9, 139, 1), // "e"
QT_MOC_LITERAL(10, 141, 29), // "closeStructuralAnalysisDialog"
QT_MOC_LITERAL(11, 171, 21), // "updateLoadSetCallBack"
QT_MOC_LITERAL(12, 193, 14), // "remeshCallBack"
QT_MOC_LITERAL(13, 208, 19), // "updateLiveVariables"
QT_MOC_LITERAL(14, 228, 22) // "updateNumberOfElements"

    },
    "StructuralAnalysisDialog\0setFeaComplete\0"
    "\0solveThermalStatic\0solveStructuralStatic\0"
    "solveStructuralModal\0onFEAFinished\0"
    "showEvent\0QShowEvent*\0e\0"
    "closeStructuralAnalysisDialog\0"
    "updateLoadSetCallBack\0remeshCallBack\0"
    "updateLiveVariables\0updateNumberOfElements"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StructuralAnalysisDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   70,    2, 0x08 /* Private */,
       4,    0,   71,    2, 0x08 /* Private */,
       5,    0,   72,    2, 0x08 /* Private */,
       6,    0,   73,    2, 0x08 /* Private */,
       7,    1,   74,    2, 0x08 /* Private */,
      10,    0,   77,    2, 0x08 /* Private */,
      11,    0,   78,    2, 0x08 /* Private */,
      12,    0,   79,    2, 0x08 /* Private */,
      13,    0,   80,    2, 0x08 /* Private */,
      14,    0,   81,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
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
        case 2: _t->solveStructuralStatic(); break;
        case 3: _t->solveStructuralModal(); break;
        case 4: _t->onFEAFinished(); break;
        case 5: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 6: _t->closeStructuralAnalysisDialog(); break;
        case 7: _t->updateLoadSetCallBack(); break;
        case 8: _t->remeshCallBack(); break;
        case 9: _t->updateLiveVariables(); break;
        case 10: _t->updateNumberOfElements(); break;
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

// SIGNAL 0
void StructuralAnalysisDialog::setFeaComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
