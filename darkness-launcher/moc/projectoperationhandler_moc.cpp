/****************************************************************************
** Meta object code from reading C++ file 'ProjectOperationHandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/ProjectOperationHandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ProjectOperationHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ProjectOperationHandler_t {
    QByteArrayData data[11];
    char stringdata0[135];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProjectOperationHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProjectOperationHandler_t qt_meta_stringdata_ProjectOperationHandler = {
    {
QT_MOC_LITERAL(0, 0, 23), // "ProjectOperationHandler"
QT_MOC_LITERAL(1, 24, 13), // "createProject"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 4), // "name"
QT_MOC_LITERAL(4, 44, 8), // "location"
QT_MOC_LITERAL(5, 53, 11), // "openProject"
QT_MOC_LITERAL(6, 65, 11), // "nameChanged"
QT_MOC_LITERAL(7, 77, 15), // "locationChanged"
QT_MOC_LITERAL(8, 93, 13), // "browseClicked"
QT_MOC_LITERAL(9, 107, 13), // "cancelClicked"
QT_MOC_LITERAL(10, 121, 13) // "createClicked"

    },
    "ProjectOperationHandler\0createProject\0"
    "\0name\0location\0openProject\0nameChanged\0"
    "locationChanged\0browseClicked\0"
    "cancelClicked\0createClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProjectOperationHandler[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x06 /* Public */,
       5,    1,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       1,    0,   72,    2, 0x0a /* Public */,
       5,    0,   73,    2, 0x0a /* Public */,
       6,    1,   74,    2, 0x0a /* Public */,
       7,    1,   77,    2, 0x0a /* Public */,
       8,    0,   80,    2, 0x0a /* Public */,
       9,    0,   81,    2, 0x0a /* Public */,
      10,    0,   82,    2, 0x0a /* Public */,
       5,    2,   83,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,

       0        // eod
};

void ProjectOperationHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ProjectOperationHandler *_t = static_cast<ProjectOperationHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->createProject((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->openProject((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->createProject(); break;
        case 3: _t->openProject(); break;
        case 4: _t->nameChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->locationChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->browseClicked(); break;
        case 7: _t->cancelClicked(); break;
        case 8: _t->createClicked(); break;
        case 9: _t->openProject((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ProjectOperationHandler::*_t)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ProjectOperationHandler::createProject)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ProjectOperationHandler::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ProjectOperationHandler::openProject)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject ProjectOperationHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ProjectOperationHandler.data,
      qt_meta_data_ProjectOperationHandler,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ProjectOperationHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProjectOperationHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ProjectOperationHandler.stringdata0))
        return static_cast<void*>(const_cast< ProjectOperationHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int ProjectOperationHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void ProjectOperationHandler::createProject(const QString & _t1, const QString & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ProjectOperationHandler::openProject(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
