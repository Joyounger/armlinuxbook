
#ifndef MIXER_H
#define MIXER_H
#include "ui_mixerbase.h"

class MixerBase : public QWidget, public Ui_MixerBase
{
public:
    MixerBase( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~MixerBase();

};

class Mixer : public MixerBase
{
    Q_OBJECT
public:
    Mixer( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~Mixer();
    void getVolume();
public slots:
   void on_QuitButton_clicked();
private slots:
    void setMasterVolume(int);private:
     void keyReleaseEvent ( QKeyEvent * );
protected:
    void keyPressEvent( QKeyEvent *) ;
};

#endif // MIXER_H
