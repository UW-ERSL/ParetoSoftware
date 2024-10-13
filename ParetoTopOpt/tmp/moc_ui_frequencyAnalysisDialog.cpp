/****************************************************************************
** Meta object code from reading C++ file 'ui_frequencyAnalysisDialog.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ui/inc/ui_frequencyAnalysisDialog.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ui_frequencyAnalysisDialog.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FrequencyAnalysisDialog_t {
    QByteArrayData data[24];
    char stringdata0[318];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FrequencyAnalysisDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FrequencyAnalysisDialog_t qt_meta_stringdata_FrequencyAnalysisDialog = {
    {
QT_MOC_LITERAL(0, 0, 23), // "FrequencyAnalysisDialog"
QT_MOC_LITERAL(1, 24, 22), // "solveFrequencyResponse"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 21), // "plotFrequencyResponse"
QT_MOC_LITERAL(4, 70, 21), // "stopFrequencyResponse"
QT_MOC_LITERAL(5, 92, 8), // "plotData"
QT_MOC_LITERAL(6, 101, 8), // "nSamples"
QT_MOC_LITERAL(7, 110, 7), // "double*"
QT_MOC_LITERAL(8, 118, 5), // "xData"
QT_MOC_LITERAL(9, 124, 5), // "yData"
QT_MOC_LITERAL(10, 130, 11), // "const char*"
QT_MOC_LITERAL(11, 142, 5), // "Title"
QT_MOC_LITERAL(12, 148, 6), // "XLabel"
QT_MOC_LITERAL(13, 155, 6), // "YLabel"
QT_MOC_LITERAL(14, 162, 12), // "QMainWindow&"
QT_MOC_LITERAL(15, 175, 10), // "dataWindow"
QT_MOC_LITERAL(16, 186, 13), // "onFEAFinished"
QT_MOC_LITERAL(17, 200, 9), // "showEvent"
QT_MOC_LITERAL(18, 210, 11), // "QShowEvent*"
QT_MOC_LITERAL(19, 222, 1), // "e"
QT_MOC_LITERAL(20, 224, 28), // "closeFrequencyAnalysisDialog"
QT_MOC_LITERAL(21, 253, 21), // "updateLoadSetCallBack"
QT_MOC_LITERAL(22, 275, 19), // "updateLiveVariables"
QT_MOC_LITERAL(23, 295, 22) // "updateNumberOfElements"

    },
    "FrequencyAnalysisDialog\0solveFrequencyResponse\0"
    "\0plotFrequencyResponse\0stopFrequencyResponse\0"
    "plotData\0nSamples\0double*\0xData\0yData\0"
    "const char*\0Title\0XLabel\0YLabel\0"
    "QMainWindow&\0dataWindow\0onFEAFinished\0"
    "showEvent\0QShowEvent*\0e\0"
    "closeFrequencyAnalysisDialog\0"
    "updateLoadSetCallBack\0updateLiveVariables\0"
    "updateNumberOfElements"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FrequencyAnalysisDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x08 /* Private */,
       3,    0,   65,    2, 0x08 /* Private */,
       4,    0,   66,    2, 0x08 /* Private */,
       5,    7,   67,    2, 0x08 /* Private */,
      16,    0,   82,    2, 0x08 /* Private */,
      17,    1,   83,    2, 0x08 /* Private */,
      20,    0,   86,    2, 0x08 /* Private */,
      21,    0,   87,    2, 0x08 /* Private */,
      22,    0,   88,    2, 0x08 /* Private */,
      23,    0,   89,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 7, 0x80000000 | 7, 0x80000000 | 10, 0x80000000 | 10, 0x80000000 | 10, 0x80000000 | 14,    6,    8,    9,   11,   12,   13,   15,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FrequencyAnalysisDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FrequencyAnalysisDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->solveFrequencyResponse(); break;
        case 1: _t->plotFrequencyResponse(); break;
        case 2: _t->stopFrequencyResponse(); break;
        case 3: _t->plotData((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double*(*)>(_a[2])),(*reinterpret_cast< double*(*)>(_a[3])),(*reinterpret_cast< const char*(*)>(_a[4])),(*reinterpret_cast< const char*(*)>(_a[5])),(*reinterpret_cast< const char*(*)>(_a[6])),(*reinterpret_cast< QMainWindow(*)>(_a[7]))); break;
        case 4: _t->onFEAFinished(); break;
        case 5: _t->showEvent((*reinterpret_cast< QShowEvent*(*)>(_a[1]))); break;
        case 6: _t->closeFrequencyAnalysisDialog(); break;
        case 7: _t->updateLoadSetCallBack(); break;
        case 8: _t->updateLiveVariables(); break;
        case 9: _t->updateNumberOfElements(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FrequencyAnalysisDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_FrequencyAnalysisDialog.data,
    qt_meta_data_FrequencyAnalysisDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FrequencyAnalysisDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FrequencyAnalysisDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FrequencyAnalysisDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Ui_FrequencyAnalysisDialog"))
        return static_cast< Ui_FrequencyAnalysisDialog*>(this);
    return QDialog::qt_metacast(_clname);
}

int FrequencyAnalysisDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
