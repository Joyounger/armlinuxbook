
#include "matrixkbdhandler.h"


#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>
#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

MatrixKbdHandler::MatrixKbdHandler(const QString &device)
{
    qLog(Input) << "Loaded Matrix keyboard plugin!";
    setObjectName("Matrix Keypad Handler");
    kbdFd = ::open(device.toLocal8Bit().constData(), O_RDONLY, 0);
    if (kbdFd >= 0) {
        qLog(Input) << "Opened" << device << "as keyboard input";
        m_notify = new QSocketNotifier(kbdFd, QSocketNotifier::Read, this);
        connect(m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    } else {
        qWarning("Cannot open %s for keyboard input (%s)",
                 device.toLocal8Bit().constData(), strerror(errno));
        return;
    }
    shift = false;
}

MatrixKbdHandler::~MatrixKbdHandler()
{
    if (kbdFd >= 0)
        ::close(kbdFd);
}

void MatrixKbdHandler::readKbdData()
{

    static bool numlock=false;
	 Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int unicode =0;
    int key_code =0;
 	 bool press=false;
	 unsigned char buf,pure;
    int n = read(kbdFd,&buf, 1);
	 if(n>0){
	 if(buf&0x80) press=false;
    else press=true;
	 pure=buf&0x7f;
	 switch(pure){
		case 0x02:unicode='1';key_code=Qt::Key_1;break;
		
		case 0x03:unicode='2';
		if(!numlock) key_code=Qt::Key_2;
		else { unicode=0xffff;key_code=Qt::Key_Down;}
		break;
	
		case 0x04:unicode='3';key_code=Qt::Key_3;break;
		
		case 0x05:unicode='4';
		if(!numlock) key_code=Qt::Key_4;
		else  {unicode=0xffff;key_code=Qt::Key_Left;}
		break;
		
		case 0x06:unicode='5';key_code=Qt::Key_5;break;
		
		case 0x07:unicode='6';
		if(!numlock) key_code=Qt::Key_6;
      else { unicode=0xffff;key_code=Qt::Key_Right;}
      break;

		case 0x08:unicode='7';key_code=Qt::Key_7;break;
		
		case 0x09:unicode='8';
		if(!numlock) key_code=Qt::Key_8;
		else { unicode=0xffff;key_code=Qt::Key_Up;}
      break;

		case 0x0a:unicode='9';key_code=Qt::Key_9;break;
		case 0x0b:unicode='0';key_code=Qt::Key_0;break;
		
		case 0x2a:unicode=0xffff;key_code=Qt::Key_Shift;
      if(!numlock&&press) numlock=true;else if(press) numlock=false;
      break;
		
      case 0x35:unicode='/';key_code=Qt::Key_Slash;break;
		case 0x37:unicode='*';key_code=Qt::Key_Asterisk;break;
		case 0x4a:unicode='-';key_code=Qt::Key_Minus;break;
		case 0x4e:unicode='+';key_code=Qt::Key_Plus;break;
		case 0x1c:unicode=13;key_code=Qt::Key_Enter;break;
		case 0x53:unicode='.';key_code=Qt::Key_Period;break;
		default:unicode='U',key_code=Qt::Key_U;break;
          }
 
    processKeyEvent(unicode, key_code, modifiers,press,false);}
}

