/****************************************************************************
** Meta object code from reading C++ file 'ProjectListModel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/ProjectListModel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ProjectListModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ProjectListModel_t {
    QByteArrayData data[16];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProjectListModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProjectListModel_t qt_meta_stringdata_ProjectListModel = {
    {
QT_MOC_LITERAL(0, 0, 16), // "ProjectListModel"
QT_MOC_LITERAL(1, 17, 7), // "setData"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 4), // "rowP"
QT_MOC_LITERAL(4, 31, 6), // "valueP"
QT_MOC_LITERAL(5, 38, 5), // "roleP"
QT_MOC_LITERAL(6, 44, 6), // "remove"
QT_MOC_LITERAL(7, 51, 13), // "createProject"
QT_MOC_LITERAL(8, 65, 4), // "name"
QT_MOC_LITERAL(9, 70, 8), // "location"
QT_MOC_LITERAL(10, 79, 11), // "openProject"
QT_MOC_LITERAL(11, 91, 16), // "ProjectListRoles"
QT_MOC_LITERAL(12, 108, 8), // "NameRole"
QT_MOC_LITERAL(13, 117, 8), // "PathRole"
QT_MOC_LITERAL(14, 126, 8), // "LineRole"
QT_MOC_LITERAL(15, 135, 8) // "LoadRole"

    },
    "ProjectListModel\0setData\0\0rowP\0valueP\0"
    "roleP\0remove\0createProject\0name\0"
    "location\0openProject\0ProjectListRoles\0"
    "NameRole\0PathRole\0LineRole\0LoadRole"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProjectListModel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       1,   52, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    3,   34,    2, 0x0a /* Public */,
       6,    1,   41,    2, 0x0a /* Public */,
       7,    2,   44,    2, 0x0a /* Public */,
      10,    1,   49,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QVariant, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    8,    9,
    QMetaType::Void, QMetaType::QString,    9,

 // enums: name, flags, count, data
      11, 0x0,    4,   56,

 // enum data: key, value
      12, uint(ProjectListModel::NameRole),
      13, uint(ProjectListModel::PathRole),
      14, uint(ProjectListModel::LineRole),
      15, uint(ProjectListModel::LoadRole),

       0        // eod
};

void ProjectListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ProjectListModel *_t = static_cast<ProjectListModel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setData((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->remove((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 2: _t->createProject((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->openProject((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ProjectListModel::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_ProjectListModel.data,
      qt_meta_data_ProjectListModel,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ProjectListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProjectListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ProjectListModel.stringdata0))
        return static_cast<void*>(const_cast< ProjectListModel*>(this));
    return QAbstractListModel::qt_metacast(_clname);
}

int ProjectListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
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
QT_END_MOC_NAMESPACE
