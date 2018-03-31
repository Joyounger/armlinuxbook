/********************************************************************************
** Form generated from reading ui file 'mynewappbase.ui'
**
** Created: Tue May 12 13:12:50 2009
**      by: Qt User Interface Compiler version 4.3.5
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MYNEWAPPBASE_H
#define UI_MYNEWAPPBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MyNewAppBase
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *TextLabel1;
    QSpacerItem *spacerItem;
    QLabel *label;
    QTextEdit *textEdit;
    QSpacerItem *spacerItem1;
    QPushButton *QuitButton;

    void setupUi(QWidget *MyNewAppBase)
    {
    if (MyNewAppBase->objectName().isEmpty())
        MyNewAppBase->setObjectName(QString::fromUtf8("MyNewAppBase"));
    MyNewAppBase->resize(196, 245);
    vboxLayout = new QVBoxLayout(MyNewAppBase);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    TextLabel1 = new QLabel(MyNewAppBase);
    TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));
    TextLabel1->setWordWrap(true);

    vboxLayout->addWidget(TextLabel1);

    spacerItem = new QSpacerItem(20, 58, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem);

    label = new QLabel(MyNewAppBase);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout->addWidget(label);

    textEdit = new QTextEdit(MyNewAppBase);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));

    vboxLayout->addWidget(textEdit);

    spacerItem1 = new QSpacerItem(20, 28, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem1);

    QuitButton = new QPushButton(MyNewAppBase);
    QuitButton->setObjectName(QString::fromUtf8("QuitButton"));

    vboxLayout->addWidget(QuitButton);


    retranslateUi(MyNewAppBase);

    QMetaObject::connectSlotsByName(MyNewAppBase);
    } // setupUi

    void retranslateUi(QWidget *MyNewAppBase)
    {
    MyNewAppBase->setWindowTitle(QApplication::translate("MyNewAppBase", "Example", 0, QApplication::UnicodeUTF8));
    TextLabel1->setText(QApplication::translate("MyNewAppBase", "Zheng Pengfei UPC Software Engineering", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MyNewAppBase", "Contact to him", 0, QApplication::UnicodeUTF8));
    QuitButton->setText(QApplication::translate("MyNewAppBase", "Quit", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MyNewAppBase);
    } // retranslateUi

};

namespace Ui {
    class MyNewAppBase: public Ui_MyNewAppBase {};
} // namespace Ui

#endif // UI_MYNEWAPPBASE_H
