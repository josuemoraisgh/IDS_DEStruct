/********************************************************************************
** Form generated from reading UI file 'dialogMaxMin.ui'
**
** Created: Sun 30. Dec 14:09:11 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGMAXMIN_H
#define UI_DIALOGMAXMIN_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogMaxMin
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QComboBox *comboBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *lineEditMax;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *lineEditMin;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *labelTal20;
    QLineEdit *lineEditDelta;
    QLabel *labelTal10;
    QHBoxLayout *_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *DialogMaxMin)
    {
        if (DialogMaxMin->objectName().isEmpty())
            DialogMaxMin->setObjectName(QString::fromUtf8("DialogMaxMin"));
        DialogMaxMin->resize(309, 277);
        verticalLayout = new QVBoxLayout(DialogMaxMin);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(DialogMaxMin);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        comboBox = new QComboBox(DialogMaxMin);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(comboBox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(DialogMaxMin);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEditMax = new QLineEdit(DialogMaxMin);
        lineEditMax->setObjectName(QString::fromUtf8("lineEditMax"));

        horizontalLayout->addWidget(lineEditMax);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(DialogMaxMin);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(label_3);

        lineEditMin = new QLineEdit(DialogMaxMin);
        lineEditMin->setObjectName(QString::fromUtf8("lineEditMin"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lineEditMin->sizePolicy().hasHeightForWidth());
        lineEditMin->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(lineEditMin);


        verticalLayout->addLayout(horizontalLayout_2);

        groupBox = new QGroupBox(DialogMaxMin);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        sizePolicy2.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy2);
        horizontalLayout_4 = new QHBoxLayout(groupBox);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        labelTal20 = new QLabel(groupBox);
        labelTal20->setObjectName(QString::fromUtf8("labelTal20"));
        sizePolicy2.setHeightForWidth(labelTal20->sizePolicy().hasHeightForWidth());
        labelTal20->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(labelTal20);

        lineEditDelta = new QLineEdit(groupBox);
        lineEditDelta->setObjectName(QString::fromUtf8("lineEditDelta"));
        sizePolicy2.setHeightForWidth(lineEditDelta->sizePolicy().hasHeightForWidth());
        lineEditDelta->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(lineEditDelta);

        labelTal10 = new QLabel(groupBox);
        labelTal10->setObjectName(QString::fromUtf8("labelTal10"));
        sizePolicy2.setHeightForWidth(labelTal10->sizePolicy().hasHeightForWidth());
        labelTal10->setSizePolicy(sizePolicy2);

        horizontalLayout_4->addWidget(labelTal10);


        verticalLayout->addWidget(groupBox);

        _2 = new QHBoxLayout();
        _2->setSpacing(6);
        _2->setObjectName(QString::fromUtf8("_2"));
        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        _2->addItem(horizontalSpacer);

        pushButton = new QPushButton(DialogMaxMin);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy);

        _2->addWidget(pushButton);


        verticalLayout->addLayout(_2);


        retranslateUi(DialogMaxMin);
        QObject::connect(pushButton, SIGNAL(clicked()), DialogMaxMin, SLOT(accept()));

        QMetaObject::connectSlotsByName(DialogMaxMin);
    } // setupUi

    void retranslateUi(QDialog *DialogMaxMin)
    {
        DialogMaxMin->setWindowTitle(QApplication::translate("DialogMaxMin", "Dialog", 0 ));
        label->setText(QApplication::translate("DialogMaxMin", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">Variavel:</span></p></body></html>", 0 ));
        label_2->setText(QApplication::translate("DialogMaxMin", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">M\303\241ximo:</span></p></body></html>", 0 ));
        label_3->setText(QApplication::translate("DialogMaxMin", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">M\303\255nimo:</span></p></body></html>", 0 ));
        groupBox->setTitle(QApplication::translate("DialogMaxMin", "Decima\303\247\303\243o", 0 ));
        labelTal20->setText(QApplication::translate("DialogMaxMin", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">tal/20 &lt; </span></p></body></html>", 0 ));
#ifndef QT_NO_WHATSTHIS
        lineEditDelta->setWhatsThis(QApplication::translate("DialogMaxMin", "Valor de Decima\303\247\303\243o de cada Saida", 0 ));
#endif // QT_NO_WHATSTHIS
        labelTal10->setText(QApplication::translate("DialogMaxMin", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\"> &lt; tal/10</span></p></body></html>", 0 ));
        pushButton->setText(QApplication::translate("DialogMaxMin", "Fechar", 0 ));
    } // retranslateUi

};

namespace Ui {
    class DialogMaxMin: public Ui_DialogMaxMin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGMAXMIN_H
