/****************************************************************************
** KAsyncIO meta object code from reading C++ file 'kasyncio.h'
**
** Created: Thu Nov 20 06:14:59 2008
**      by: The Qt MOC ($Id: qt/src/moc/moc.y   2.3.10   edited 2005-01-24 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "kasyncio.h"
#include <qmetaobject.h>
#include <qapplication.h>
#ifdef QWS
#include <qobjectdict.h>
#endif



const char *KAsyncIO::className() const
{
    return "KAsyncIO";
}

QMetaObject *KAsyncIO::metaObj = 0;

#ifdef QWS
static class KAsyncIO_metaObj_Unloader {
public:
    ~KAsyncIO_metaObj_Unloader()
    {
         if ( objectDict )
             objectDict->remove( "KAsyncIO" );
    }
} KAsyncIO_metaObj_unloader;
#endif

void KAsyncIO::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("KAsyncIO","QObject");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString KAsyncIO::tr(const char* s)
{
    return qApp->translate( "KAsyncIO", s, 0 );
}

QString KAsyncIO::tr(const char* s, const char * c)
{
    return qApp->translate( "KAsyncIO", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* KAsyncIO::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QObject::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    typedef void (KAsyncIO::*m2_t0)();
    typedef void (QObject::*om2_t0)();
    typedef void (KAsyncIO::*m2_t1)();
    typedef void (QObject::*om2_t1)();
    m2_t0 v2_0 = &KAsyncIO::readyRead;
    om2_t0 ov2_0 = (om2_t0)v2_0;
    m2_t1 v2_1 = &KAsyncIO::readyWrite;
    om2_t1 ov2_1 = (om2_t1)v2_1;
    QMetaData *signal_tbl = QMetaObject::new_metadata(2);
    signal_tbl[0].name = "readyRead()";
    signal_tbl[0].ptr = (QMember)ov2_0;
    signal_tbl[1].name = "readyWrite()";
    signal_tbl[1].ptr = (QMember)ov2_1;
    metaObj = QMetaObject::new_metaobject(
	"KAsyncIO", "QObject",
	0, 0,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}

// SIGNAL readyRead
void KAsyncIO::readyRead()
{
    activate_signal( "readyRead()" );
}

// SIGNAL readyWrite
void KAsyncIO::readyWrite()
{
    activate_signal( "readyWrite()" );
}
