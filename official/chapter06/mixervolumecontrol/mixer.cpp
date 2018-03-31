#include "mixer.h"


#include <qpushbutton.h>
#include <QWSServer>
#include <QKeyEvent>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <errno.h>

MixerBase::MixerBase( QWidget *parent, Qt::WFlags f )
        : QWidget( parent, f )
{
    setupUi( this );
}

MixerBase::~MixerBase()
{
}

Mixer::Mixer( QWidget *parent, Qt::WFlags f )
        : MixerBase( parent, f )
{

    connect(verticalSlider_master,SIGNAL(valueChanged(int)), 
            this, SLOT(setMasterVolume(int)));
	 connect(QuitButton,SIGNAL(clicked()), 
            this, SLOT(on_QuitButton_clicked()));
   
    getVolume();
	 QPalette* pPal=new QPalette(palette());
	// QPixmap* pPic=new QPixmap("./flag.png");
    pPal->setBrush(QPalette::Base,QBrush(QPixmap("./flag.png")));
	// pPal->setBrush(QPalette::Window,QBrush(*pPic));
	 setAutoFillBackground(true);
	 this->setPalette(*pPal);

}
void Mixer::on_QuitButton_clicked()
{
	close();
}

Mixer::~Mixer()
{

}

void Mixer::getVolume()
{
    unsigned int master_volume;
   // unsigned int mic_volume;

    unsigned int stereodevs;

    int mixerHandle = open( "/dev/sound/dsp", O_RDWR );
    if ( mixerHandle <= 0 ) {
        perror("open(\"/dev/sound/mixer\")");
        return;
    }

    if(ioctl( mixerHandle, SOUND_MIXER_READ_STEREODEVS, &stereodevs )==-1)
        perror("ioctl(\" SOUND_MIXER_READ_STEREODEVS\")");

    if(ioctl( mixerHandle, MIXER_READ(SOUND_MIXER_VOLUME), 
              & master_volume )==-1)
        perror("ioctl(\"MIXER_READ\")");
    ::close( mixerHandle );

          int masterlevel=0;
          if(stereodevs & SOUND_MIXER_VOLUME)
              masterlevel = (master_volume & 0xff00) >> 8;
          else
              masterlevel = master_volume;
         verticalSlider_master->setSliderPosition(masterlevel / 10);
  
     
          
}

void Mixer::setMasterVolume(int vol)
{
    unsigned int stereodevs;
    int level = ((vol << 8) + vol)*10;
    int fd = 0;
    if ((fd = open("/dev/sound/mixer", O_RDWR))>=0) {
        if(ioctl(fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &level) == -1)
            perror("ioctl(\"MIXER_VOLUME_WRITE\")");

    }
    ::close(fd);
}


void Mixer::keyReleaseEvent( QKeyEvent * e)
{

    
}

void Mixer::keyPressEvent( QKeyEvent *e) 
{
     QWidget *wd = QApplication::focusWidget();
    QSlider *currentSlider;
    if(wd->inherits("QSlider")) {
        currentSlider = qobject_cast<QSlider *>(wd);
    }

    switch(e->key()) {
    case Qt::Key_Plus:
        currentSlider->setSliderPosition(currentSlider->value()+1);
		  break;
    case Qt::Key_Minus:
        currentSlider->setSliderPosition(currentSlider->value()-1);
	     break;
 case Qt::Key_0:
	     currentSlider->setSliderPosition(0);
        break;
	 case Qt::Key_1:
	     currentSlider->setSliderPosition(1);
        break;
 case Qt::Key_2:
	     currentSlider->setSliderPosition(2);
        break;
 case Qt::Key_3:
	     currentSlider->setSliderPosition(3);
        break;
 case Qt::Key_4:
	     currentSlider->setSliderPosition(4);
        break;
 case Qt::Key_5:
	     currentSlider->setSliderPosition(5);
        break;
 case Qt::Key_6:
	     currentSlider->setSliderPosition(6);
        break;
 case Qt::Key_7:
	     currentSlider->setSliderPosition(7);
        break;
 case Qt::Key_8:
	     currentSlider->setSliderPosition(8);
        break;
 case Qt::Key_9:
	     currentSlider->setSliderPosition(9);
        break;
  };
}
#include"mixer.moc"
