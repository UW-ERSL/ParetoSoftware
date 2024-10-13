/****************************************************************************
** Meta object code from reading C++ file 'ui_constraintsDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Ui/inc/ui_constraintsDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_constraintsDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ConstraintsDialog_t {
    QByteArrayData data[8];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConstraintsDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConstraintsDialog_t qt_meta_stringdata_ConstraintsDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ConstraintsDialog"
QT_MOC_LITERAL(1, 18, 22), // "setConstraintsComplete"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 17), // "updateConstraints"
QT_MOC_LITERAL(4, 60, 11), // "closeDialog"
QT_MOC_LITERAL(5, 72, 9), // "showEvent"
QT_MOC_LITERAL(6, 82, 11), // "QShowEvent*"
QT_MOC_LITERAL(7, 94, 1) // "e"

    },
    "ConstraintsDialog\0setConstraintsComplete\0"
    "\0updateConstraints\0closeDialog\0showEvent\0"
    "QShowEvent*\0e"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConstraintsDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   35,    2, 0x08 /* Private */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    1,   37,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,

       0        // eod
};

void ConstraintsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ConstraintsDialog *_t = static_cast<ConstraintsDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setConstraintsComplete(); break;
        case 1: _t->updateConstraints(); break;
        case 2: _t->closeDialog(); break;
        case 3: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ConstraintsDialog::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ConstraintsDialog::setConstraintsComplete)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject ConstraintsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ConstraintsDialog.data,
      qt_meta_data_ConstraintsDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ConstraintsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConstraintsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ConstraintsDialog.stringdata0))
        return static_cast<void*>(const_cast< ConstraintsDialog*>(this));
    if (!strcmp(_clname, "Ui_constraintsDialog"))
        return static_cast< Ui_constraintsDialog*>(const_cast< ConstraintsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ConstraintsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ConstraintsDialog::setConstraintsComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
