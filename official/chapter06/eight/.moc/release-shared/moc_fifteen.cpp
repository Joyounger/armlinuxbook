/****************************************************************************
** Meta object code from reading C++ file 'fifteen.h'
**
** Created: Tue May 26 20:35:52 2009
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../fifteen.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fifteen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_PiecesTable[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      23,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      40,   12,   12,   12, 0x0a,
      52,   12,   12,   12, 0x0a,
      60,   12,   12,   12, 0x0a,
      73,   12,   12,   12, 0x0a,
      85,   12,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PiecesTable[] = {
    "PiecesTable\0\0gameWon()\0updateMenu(bool)\0"
    "randomize()\0reset()\0showNumber()\0"
    "loadImage()\0deleteImage()\0"
};

const QMetaObject PiecesTable::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_PiecesTable,
      qt_meta_data_PiecesTable, 0 }
};

const QMetaObject *PiecesTable::metaObject() const
{
    return &staticMetaObject;
}

void *PiecesTable::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PiecesTable))
	return static_cast<void*>(const_cast< PiecesTable*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int PiecesTable::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: gameWon(); break;
        case 1: updateMenu((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: randomize(); break;
        case 3: reset(); break;
        case 4: showNumber(); break;
        case 5: loadImage(); break;
        case 6: deleteImage(); break;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void PiecesTable::gameWon()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void PiecesTable::updateMenu(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_PiecesView[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PiecesView[] = {
    "PiecesView\0\0announceWin()\0"
};

const QMetaObject PiecesView::staticMetaObject = {
    { &QTableView::staticMetaObject, qt_meta_stringdata_PiecesView,
      qt_meta_data_PiecesView, 0 }
};

const QMetaObject *PiecesView::metaObject() const
{
    return &staticMetaObject;
}

void *PiecesView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PiecesView))
	return static_cast<void*>(const_cast< PiecesView*>(this));
    return QTableView::qt_metacast(_clname);
}

int PiecesView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: announceWin(); break;
        }
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_FifteenMainWindow[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x0a,
      32,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FifteenMainWindow[] = {
    "FifteenMainWindow\0\0showNumber()\0"
    "updateMenu(bool)\0"
};

const QMetaObject FifteenMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_FifteenMainWindow,
      qt_meta_data_FifteenMainWindow, 0 }
};

const QMetaObject *FifteenMainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *FifteenMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FifteenMainWindow))
	return static_cast<void*>(const_cast< FifteenMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int FifteenMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: showNumber(); break;
        case 1: updateMenu((*reinterpret_cast< bool(*)>(_a[1]))); break;
        }
        _id -= 2;
    }
    return _id;
}
