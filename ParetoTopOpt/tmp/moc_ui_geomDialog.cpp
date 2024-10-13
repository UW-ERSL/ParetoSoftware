/****************************************************************************
** Meta object code from reading C++ file 'ui_geomDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_geomDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_geomDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GeometryDialog_t {
    QByteArrayData data[11];
    char stringdata0[127];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GeometryDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GeometryDialog_t qt_meta_stringdata_GeometryDialog = {
    {
QT_MOC_LITERAL(0, 0, 14), // "GeometryDialog"
QT_MOC_LITERAL(1, 15, 19), // "setGeometryComplete"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 7), // "loadSTL"
QT_MOC_LITERAL(4, 44, 12), // "loadGeometry"
QT_MOC_LITERAL(5, 57, 9), // "saveAsSTL"
QT_MOC_LITERAL(6, 67, 9), // "updateSTL"
QT_MOC_LITERAL(7, 77, 13), // "createTetmesh"
QT_MOC_LITERAL(8, 91, 13), // "scaleGeometry"
QT_MOC_LITERAL(9, 105, 9), // "saveAsOff"
QT_MOC_LITERAL(10, 115, 11) // "closeDialog"

    },
    "GeometryDialog\0setGeometryComplete\0\0"
    "loadSTL\0loadGeometry\0saveAsSTL\0updateSTL\0"
    "createTetmesh\0scaleGeometry\0saveAsOff\0"
    "closeDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GeometryDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   60,    2, 0x08 /* Private */,
       4,    0,   61,    2, 0x08 /* Private */,
       5,    0,   62,    2, 0x08 /* Private */,
       6,    0,   63,    2, 0x08 /* Private */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    0,   67,    2, 0x08 /* Private */,

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

       0        // eod
};

void GeometryDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GeometryDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setGeometryComplete(); break;
        case 1: _t->loadSTL(); break;
        case 2: _t->loadGeometry(); break;
        case 3: _t->saveAsSTL(); break;
        case 4: _t->updateSTL(); break;
        case 5: _t->createTetmesh(); break;
        case 6: _t->scaleGeometry(); break;
        case 7: _t->saveAsOff(); break;
        case 8: _t->closeDialog(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GeometryDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeometryDialog::setGeometryComplete)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject GeometryDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_GeometryDialog.data,
    qt_meta_data_GeometryDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GeometryDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GeometryDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GeometryDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_GeometryDialog"))
        return static_cast< Ui_GeometryDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int GeometryDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void GeometryDialog::setGeometryComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
