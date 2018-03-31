/****************************************************************************
** Meta object code from reading C++ file 'mynewapp.h'
**
** Created: Tue May 12 13:12:52 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mynewapp.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mynewapp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_MyNewApp[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MyNewApp[] = {
    "MyNewApp\0\0on_QuitButton_clicked()\0"
};

const QMetaObject MyNewApp::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MyNewApp,
      qt_meta_data_MyNewApp, 0 }
};

const QMetaObject *MyNewApp::metaObject() const
{
    return &staticMetaObject;
}

void *MyNewApp::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyNewApp))
	return static_cast<void*>(const_cast< MyNewApp*>(this));
    if (!strcmp(_clname, "Ui_MyNewAppBase"))
	return static_cast< Ui_MyNewAppBase*>(const_cast< MyNewApp*>(this));
    return QWidget::qt_metacast(_clname);
}

int MyNewApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_QuitButton_clicked(); break;
        }
        _id -= 1;
    }
    return _id;
}
