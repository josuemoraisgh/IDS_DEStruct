/********************************************************************************
** Form generated from reading UI file 'dialogDecimacao.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGDECIMACAO_H
#define UI_DIALOGDECIMACAO_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogDecimacao
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QComboBox *comboBox;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *labelTal20;
    QLineEdit *lineEditDelta;
    QLabel *labelTal10;
    QHBoxLayout *_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *DialogDecimacao)
    {
        if (DialogDecimacao->objectName().isEmpty())
            DialogDecimacao->setObjectName(QString::fromUtf8("DialogDecimacao"));
        DialogDecimacao->resize(309, 188);
        verticalLayout = new QVBoxLayout(DialogDecimacao);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(DialogDecimacao);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);

        comboBox = new QComboBox(DialogDecimacao);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(comboBox);


        verticalLayout->addLayout(horizontalLayout_3);

        groupBox = new QGroupBox(DialogDecimacao);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        horizontalLayout_4 = new QHBoxLayout(groupBox);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        labelTal20 = new QLabel(groupBox);
        labelTal20->setObjectName(QString::fromUtf8("labelTal20"));
        sizePolicy1.setHeightForWidth(labelTal20->sizePolicy().hasHeightForWidth());
        labelTal20->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(labelTal20);

        lineEditDelta = new QLineEdit(groupBox);
        lineEditDelta->setObjectName(QString::fromUtf8("lineEditDelta"));
        sizePolicy1.setHeightForWidth(lineEditDelta->sizePolicy().hasHeightForWidth());
        lineEditDelta->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(lineEditDelta);

        labelTal10 = new QLabel(groupBox);
        labelTal10->setObjectName(QString::fromUtf8("labelTal10"));
        sizePolicy1.setHeightForWidth(labelTal10->sizePolicy().hasHeightForWidth());
        labelTal10->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(labelTal10);


        verticalLayout->addWidget(groupBox);

        _2 = new QHBoxLayout();
        _2->setSpacing(6);
        _2->setObjectName(QString::fromUtf8("_2"));
        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        _2->addItem(horizontalSpacer);

        pushButton = new QPushButton(DialogDecimacao);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy);

        _2->addWidget(pushButton);


        verticalLayout->addLayout(_2);


        retranslateUi(DialogDecimacao);
        QObject::connect(pushButton, SIGNAL(clicked()), DialogDecimacao, SLOT(accept()));

        QMetaObject::connectSlotsByName(DialogDecimacao);
    } // setupUi

    void retranslateUi(QDialog *DialogDecimacao)
    {
        DialogDecimacao->setWindowTitle(QCoreApplication::translate("DialogDecimacao", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("DialogDecimacao", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">Variavel:</span></p></body></html>", nullptr));
        groupBox->setTitle(QCoreApplication::translate("DialogDecimacao", "Decima\303\247\303\243o", nullptr));
        labelTal20->setText(QCoreApplication::translate("DialogDecimacao", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\">tal/20 &lt; </span></p></body></html>", nullptr));
#if QT_CONFIG(whatsthis)
        lineEditDelta->setWhatsThis(QCoreApplication::translate("DialogDecimacao", "Valor de Decima\303\247\303\243o de cada Saida", nullptr));
#endif // QT_CONFIG(whatsthis)
        labelTal10->setText(QCoreApplication::translate("DialogDecimacao", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt;\"> &lt; tal/10</span></p></body></html>", nullptr));
        pushButton->setText(QCoreApplication::translate("DialogDecimacao", "Fechar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogDecimacao: public Ui_DialogDecimacao {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGDECIMACAO_H
