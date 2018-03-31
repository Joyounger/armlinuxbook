#include "matrixkbddriverplugin.h"
#include "matrixkbdhandler.h"

#include <qtopiaglobal.h>

#include <qtopialog.h>

MatrixKbdDriverPlugin::MatrixKbdDriverPlugin( QObject *parent )
    : QKbdDriverPlugin( parent )
{
}

MatrixKbdDriverPlugin::~MatrixKbdDriverPlugin()
{
}

QWSKeyboardHandler* MatrixKbdDriverPlugin::create(const QString& driver,
                                                   const QString& device)
{
    if (driver.toLower() == "matrixkbd") {
        qLog(Input) << "Before call MatrixKbdHandler()";
        return new MatrixKbdHandler(device);
    }
    return 0;
}

QWSKeyboardHandler* MatrixKbdDriverPlugin::create(const QString& driver)
{
    if (driver.toLower() == "matrixkbd") {
        qLog(Input) << "Before call MatrixKbdHandler()";
        return new MatrixKbdHandler();
    }
    return 0;
}

QStringList MatrixKbdDriverPlugin::keys() const
{
    return QStringList() << "matrixkbd";
}

QTOPIA_EXPORT_QT_PLUGIN(MatrixKbdDriverPlugin)
