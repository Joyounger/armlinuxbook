#ifndef MATRIXKBDHANDLER_H
#define MATRIXKBDHANDLER_H

#include <QObject>
#include <QWSKeyboardHandler>

class QSocketNotifier;
class MatrixKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    MatrixKbdHandler(const QString &device = QString("/dev/mcu/kbd"));
    ~MatrixKbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFd;
    bool shift;

private Q_SLOTS:
    void readKbdData();
};


#endif // MATRIXKBDHANDLER_H
