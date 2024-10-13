/****************************************************************************
** Meta object code from reading C++ file 'ui_toolsDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_toolsDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_toolsDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ToolsDialog_t {
    QByteArrayData data[6];
    char stringdata0[79];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ToolsDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ToolsDialog_t qt_meta_stringdata_ToolsDialog = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ToolsDialog"
QT_MOC_LITERAL(1, 12, 17), // "captureScreenShot"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 18), // "captureScreenVideo"
QT_MOC_LITERAL(4, 50, 16), // "captureFrequency"
QT_MOC_LITERAL(5, 67, 11) // "closeDialog"

    },
    "ToolsDialog\0captureScreenShot\0\0"
    "captureScreenVideo\0captureFrequency\0"
    "closeDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ToolsDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    1,   35,    2, 0x08 /* Private */,
       3,    0,   38,    2, 0x28 /* Private | MethodCloned */,
       5,    0,   39,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ToolsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ToolsDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->captureScreenShot(); break;
        case 1: _t->captureScreenVideo((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->captureScreenVideo(); break;
        case 3: _t->closeDialog(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ToolsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ToolsDialog.data,
    qt_meta_data_ToolsDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ToolsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ToolsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ToolsDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_ToolsDialog"))
        return static_cast< Ui_ToolsDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int ToolsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
