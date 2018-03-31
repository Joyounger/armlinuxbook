/****************************************************************************
** Meta object code from reading C++ file 'matrixkbdhandler.h'
**
** Created: Wed May 27 10:40:35 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../matrixkbdhandler.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'matrixkbdhandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_MatrixKbdHandler[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MatrixKbdHandler[] = {
    "MatrixKbdHandler\0\0readKbdData()\0"
};

const QMetaObject MatrixKbdHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MatrixKbdHandler,
      qt_meta_data_MatrixKbdHandler, 0 }
};

const QMetaObject *MatrixKbdHandler::metaObject() const
{
    return &staticMetaObject;
}

void *MatrixKbdHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MatrixKbdHandler))
	return static_cast<void*>(const_cast< MatrixKbdHandler*>(this));
    if (!strcmp(_clname, "QWSKeyboardHandler"))
	return static_cast< QWSKeyboardHandler*>(const_cast< MatrixKbdHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int MatrixKbdHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readKbdData(); break;
        }
        _id -= 1;
    }
    return _id;
}
