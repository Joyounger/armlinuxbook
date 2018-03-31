/****************************************************************************
** KIO::SlaveConfig meta object code from reading C++ file 'slaveconfig.h'
**
** Created: Thu Nov 20 06:16:07 2008
**      by: The Qt MOC ($Id: qt/src/moc/moc.y   2.3.10   edited 2005-01-24 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 9
#elif Q_MOC_OUTPUT_REVISION != 9
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "slaveconfig.h"
#include <qmetaobject.h>
#include <qapplication.h>
#ifdef QWS
#include <qobjectdict.h>
#endif



const char *KIO::SlaveConfig::className() const
{
    return "KIO::SlaveConfig";
}

QMetaObject *KIO::SlaveConfig::metaObj = 0;

#ifdef QWS
static class KIO__SlaveConfig_metaObj_Unloader {
public:
    ~KIO__SlaveConfig_metaObj_Unloader()
    {
         if ( objectDict )
             objectDict->remove( "KIO::SlaveConfig" );
    }
} KIO__SlaveConfig_metaObj_unloader;
#endif

void KIO::SlaveConfig::initMetaObject()
{
    if ( metaObj )
	return;
    if ( qstrcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("KIO::SlaveConfig","QObject");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION

QString KIO::SlaveConfig::tr(const char* s)
{
    return qApp->translate( "KIO::SlaveConfig", s, 0 );
}

QString KIO::SlaveConfig::tr(const char* s, const char * c)
{
    return qApp->translate( "KIO::SlaveConfig", s, c );
}

#endif // QT_NO_TRANSLATION

QMetaObject* KIO::SlaveConfig::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QObject::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    typedef void (KIO::SlaveConfig::*m2_t0)(const QString&,const QString&);
    typedef void (QObject::*om2_t0)(const QString&,const QString&);
    m2_t0 v2_0 = &KIO::SlaveConfig::configNeeded;
    om2_t0 ov2_0 = (om2_t0)v2_0;
    QMetaData *signal_tbl = QMetaObject::new_metadata(1);
    signal_tbl[0].name = "configNeeded(const QString&,const QString&)";
    signal_tbl[0].ptr = (QMember)ov2_0;
    metaObj = QMetaObject::new_metaobject(
	"KIO::SlaveConfig", "QObject",
	0, 0,
	signal_tbl, 1,
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

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL configNeeded
void KIO::SlaveConfig::configNeeded( const QString& t0, const QString& t1 )
{
    // No builtin function for signal parameter type const QString&,const QString&
    QConnectionList *clist = receivers("configNeeded(const QString&,const QString&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef void (QObject::*RT1)(const QString&);
    typedef void (QObject::*RT2)(const QString&,const QString&);
    RT0 r0;
    RT1 r1;
    RT2 r2;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
#ifdef Q_FP_CCAST_BROKEN
		r0 = reinterpret_cast<RT0>(*(c->member()));
#else
		r0 = (RT0)*(c->member());
#endif
		(object->*r0)();
		break;
	    case 1:
#ifdef Q_FP_CCAST_BROKEN
		r1 = reinterpret_cast<RT1>(*(c->member()));
#else
		r1 = (RT1)*(c->member());
#endif
		(object->*r1)(t0);
		break;
	    case 2:
#ifdef Q_FP_CCAST_BROKEN
		r2 = reinterpret_cast<RT2>(*(c->member()));
#else
		r2 = (RT2)*(c->member());
#endif
		(object->*r2)(t0, t1);
		break;
	}
    }
}
