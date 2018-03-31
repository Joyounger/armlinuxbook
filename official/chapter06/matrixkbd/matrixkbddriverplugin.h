#ifndef MATRIXKBDDRIVERPLUGIN_H
#define MATRIXKBDDRIVERPLUGIN_H

#include <QtGui/QWSKeyboardHandlerFactoryInterface>

class MatrixKbdDriverPlugin : public QKbdDriverPlugin {
    Q_OBJECT
public:
    MatrixKbdDriverPlugin( QObject *parent  = 0 );
    ~MatrixKbdDriverPlugin();

    QWSKeyboardHandler* create(const QString& driver, const QString& device);
    QWSKeyboardHandler* create(const QString& driver);
    QStringList keys() const;
};

#endif // MATRIXKBDDRIVERPLUGIN_H
