/********************************************************************************
** Form generated from reading ui file 'mixerbase.ui'
**
** Created: Tue Jun 2 10:47:46 2009
**      by: Qt User Interface Compiler version 4.3.5
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MIXERBASE_H
#define UI_MIXERBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MixerBase
{
public:
    QWidget *widget;
    QVBoxLayout *vboxLayout;
    QLabel *label_author;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout1;
    QLCDNumber *lcdNumber_master;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem;
    QSlider *verticalSlider_master;
    QSpacerItem *spacerItem1;
    QLabel *label_master;
    QVBoxLayout *vboxLayout2;
    QSpacerItem *spacerItem2;
    QLabel *label_promt;
    QSpacerItem *spacerItem3;
    QPushButton *QuitButton;

    void setupUi(QWidget *MixerBase)
    {
    if (MixerBase->objectName().isEmpty())
        MixerBase->setObjectName(QString::fromUtf8("MixerBase"));
    MixerBase->resize(517, 565);
    widget = new QWidget(MixerBase);
    widget->setObjectName(QString::fromUtf8("widget"));
    widget->setGeometry(QRect(130, 30, 271, 451));
    vboxLayout = new QVBoxLayout(widget);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    label_author = new QLabel(widget);
    label_author->setObjectName(QString::fromUtf8("label_author"));

    vboxLayout->addWidget(label_author);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    lcdNumber_master = new QLCDNumber(widget);
    lcdNumber_master->setObjectName(QString::fromUtf8("lcdNumber_master"));
    lcdNumber_master->setMinimumSize(QSize(80, 0));
    lcdNumber_master->setNumDigits(2);
    lcdNumber_master->setSegmentStyle(QLCDNumber::Outline);

    vboxLayout1->addWidget(lcdNumber_master);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    spacerItem = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem);

    verticalSlider_master = new QSlider(widget);
    verticalSlider_master->setObjectName(QString::fromUtf8("verticalSlider_master"));
    verticalSlider_master->setMinimumSize(QSize(80, 0));
    verticalSlider_master->setLayoutDirection(Qt::LeftToRight);
    verticalSlider_master->setAutoFillBackground(true);
    verticalSlider_master->setMaximum(10);
    verticalSlider_master->setOrientation(Qt::Vertical);
    verticalSlider_master->setTickPosition(QSlider::TicksBothSides);

    hboxLayout1->addWidget(verticalSlider_master);

    spacerItem1 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem1);


    vboxLayout1->addLayout(hboxLayout1);

    label_master = new QLabel(widget);
    label_master->setObjectName(QString::fromUtf8("label_master"));
    label_master->setMinimumSize(QSize(80, 0));
    label_master->setAlignment(Qt::AlignCenter);

    vboxLayout1->addWidget(label_master);


    hboxLayout->addLayout(vboxLayout1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    spacerItem2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout2->addItem(spacerItem2);

    label_promt = new QLabel(widget);
    label_promt->setObjectName(QString::fromUtf8("label_promt"));
    label_promt->setAlignment(Qt::AlignCenter);

    vboxLayout2->addWidget(label_promt);

    spacerItem3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout2->addItem(spacerItem3);


    hboxLayout->addLayout(vboxLayout2);


    vboxLayout->addLayout(hboxLayout);

    QuitButton = new QPushButton(widget);
    QuitButton->setObjectName(QString::fromUtf8("QuitButton"));

    vboxLayout->addWidget(QuitButton);


    retranslateUi(MixerBase);
    QObject::connect(verticalSlider_master, SIGNAL(valueChanged(int)), lcdNumber_master, SLOT(display(int)));

    QMetaObject::connectSlotsByName(MixerBase);
    } // setupUi

    void retranslateUi(QWidget *MixerBase)
    {
    MixerBase->setWindowTitle(QApplication::translate("MixerBase", "Set Volume", 0, QApplication::UnicodeUTF8));
    label_author->setText(QApplication::translate("MixerBase", " China University of Petroleum\n"
" Zheng Pengfei", 0, QApplication::UnicodeUTF8));
    label_master->setText(QApplication::translate("MixerBase", "Master\n"
"Volume", 0, QApplication::UnicodeUTF8));
    label_promt->setText(QApplication::translate("MixerBase", "press \n"
"\"+\"\n"
"or \n"
"\"-\"\n"
"on\n"
"kbd", 0, QApplication::UnicodeUTF8));
    QuitButton->setText(QApplication::translate("MixerBase", "Quit", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MixerBase);
    } // retranslateUi

};

namespace Ui {
    class MixerBase: public Ui_MixerBase {};
} // namespace Ui

#endif // UI_MIXERBASE_H
