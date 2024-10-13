/****************************************************************************
** Meta object code from reading C++ file 'ui_thermalLoadsWindow.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Ui/inc/ui_thermalLoadsWindow.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_thermalLoadsWindow.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ThermalLoadsWindow_t {
    QByteArrayData data[9];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ThermalLoadsWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ThermalLoadsWindow_t qt_meta_stringdata_ThermalLoadsWindow = {
    {
QT_MOC_LITERAL(0, 0, 18), // "ThermalLoadsWindow"
QT_MOC_LITERAL(1, 19, 23), // "setThermalLoadsComplete"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 18), // "assignThermalLoads"
QT_MOC_LITERAL(4, 63, 16), // "applyThermalLoad"
QT_MOC_LITERAL(5, 80, 9), // "showEvent"
QT_MOC_LITERAL(6, 90, 11), // "QShowEvent*"
QT_MOC_LITERAL(7, 102, 1), // "e"
QT_MOC_LITERAL(8, 104, 11) // "closeDialog"

    },
    "ThermalLoadsWindow\0setThermalLoadsComplete\0"
    "\0assignThermalLoads\0applyThermalLoad\0"
    "showEvent\0QShowEvent*\0e\0closeDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ThermalLoadsWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    1,   42,    2, 0x08 /* Private */,
       8,    0,   45,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

       0        // eod
};

void ThermalLoadsWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ThermalLoadsWindow *_t = static_cast<ThermalLoadsWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setThermalLoadsComplete(); break;
        case 1: _t->assignThermalLoads(); break;
        case 2: _t->applyThermalLoad(); break;
        case 3: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 4: _t->closeDialog(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (ThermalLoadsWindow::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ThermalLoadsWindow::setThermalLoadsComplete)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject ThermalLoadsWindow::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ThermalLoadsWindow.data,
      qt_meta_data_ThermalLoadsWindow,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ThermalLoadsWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThermalLoadsWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ThermalLoadsWindow.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_thermalLoadWindow"))
        return static_cast< Ui_thermalLoadWindow*>(this);
    return QDialog::qt_metacast(_clname);
}

int ThermalLoadsWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void ThermalLoadsWindow::setThermalLoadsComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
