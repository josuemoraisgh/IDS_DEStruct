/********************************************************************************
** Form generated from reading UI file 'icarregar.ui'
**
** Created: Sun 30. Dec 14:09:11 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ICARREGAR_H
#define UI_ICARREGAR_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QSplitter>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogCar
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *LECaminho;
    QPushButton *BSeleciona;
    QCheckBox *CBSalvar;
    QHBoxLayout *horizontalLayout_5;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_4;
    QSplitter *splitter_3;
    QListView *LVEntradas;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_6;
    QSplitter *splitter;
    QListView *LVSaida;
    QHBoxLayout *horizontalLayout;
    QLabel *label_4;
    QLineEdit *LETIni;
    QComboBox *CBTIni;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_5;
    QLineEdit *LETFim;
    QComboBox *CBTFim;
    QSpacerItem *horizontalSpacer;
    QSplitter *splitter_5;
    QPushButton *BConcatenar;
    QPushButton *BCarregar;
    QPushButton *BCancel;
    QProgressBar *DIC_pb;
    QHBoxLayout *horizontalLayout_3;
    QLabel *LVStBar;
    QSpacerItem *horizontalSpacer_2;
    QLabel *LVStBar2;

    void setupUi(QDialog *DialogCar)
    {
        if (DialogCar->objectName().isEmpty())
            DialogCar->setObjectName(QString::fromUtf8("DialogCar"));
        DialogCar->resize(705, 399);
        verticalLayout = new QVBoxLayout(DialogCar);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(DialogCar);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        LECaminho = new QLineEdit(DialogCar);
        LECaminho->setObjectName(QString::fromUtf8("LECaminho"));

        horizontalLayout_2->addWidget(LECaminho);

        BSeleciona = new QPushButton(DialogCar);
        BSeleciona->setObjectName(QString::fromUtf8("BSeleciona"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(BSeleciona->sizePolicy().hasHeightForWidth());
        BSeleciona->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(BSeleciona);

        CBSalvar = new QCheckBox(DialogCar);
        CBSalvar->setObjectName(QString::fromUtf8("CBSalvar"));

        horizontalLayout_2->addWidget(CBSalvar);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        groupBox = new QGroupBox(DialogCar);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setAlignment(Qt::AlignCenter);
        horizontalLayout_4 = new QHBoxLayout(groupBox);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        splitter_3 = new QSplitter(groupBox);
        splitter_3->setObjectName(QString::fromUtf8("splitter_3"));
        splitter_3->setOrientation(Qt::Vertical);
        LVEntradas = new QListView(splitter_3);
        LVEntradas->setObjectName(QString::fromUtf8("LVEntradas"));
        splitter_3->addWidget(LVEntradas);

        horizontalLayout_4->addWidget(splitter_3);


        horizontalLayout_5->addWidget(groupBox);

        groupBox_2 = new QGroupBox(DialogCar);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setLayoutDirection(Qt::LeftToRight);
        groupBox_2->setAlignment(Qt::AlignCenter);
        horizontalLayout_6 = new QHBoxLayout(groupBox_2);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        splitter = new QSplitter(groupBox_2);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Vertical);
        LVSaida = new QListView(splitter);
        LVSaida->setObjectName(QString::fromUtf8("LVSaida"));
        splitter->addWidget(LVSaida);

        horizontalLayout_6->addWidget(splitter);


        horizontalLayout_5->addWidget(groupBox_2);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_4 = new QLabel(DialogCar);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout->addWidget(label_4);

        LETIni = new QLineEdit(DialogCar);
        LETIni->setObjectName(QString::fromUtf8("LETIni"));
        sizePolicy.setHeightForWidth(LETIni->sizePolicy().hasHeightForWidth());
        LETIni->setSizePolicy(sizePolicy);
        LETIni->setMaximumSize(QSize(50, 16777215));
        LETIni->setMaxLength(9999999);

        horizontalLayout->addWidget(LETIni);

        CBTIni = new QComboBox(DialogCar);
        CBTIni->setObjectName(QString::fromUtf8("CBTIni"));
        sizePolicy.setHeightForWidth(CBTIni->sizePolicy().hasHeightForWidth());
        CBTIni->setSizePolicy(sizePolicy);
        CBTIni->setMaximumSize(QSize(50, 16777215));

        horizontalLayout->addWidget(CBTIni);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        label_5 = new QLabel(DialogCar);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout->addWidget(label_5);

        LETFim = new QLineEdit(DialogCar);
        LETFim->setObjectName(QString::fromUtf8("LETFim"));
        sizePolicy.setHeightForWidth(LETFim->sizePolicy().hasHeightForWidth());
        LETFim->setSizePolicy(sizePolicy);
        LETFim->setMaximumSize(QSize(50, 16777215));
        LETFim->setMaxLength(9999999);

        horizontalLayout->addWidget(LETFim);

        CBTFim = new QComboBox(DialogCar);
        CBTFim->setObjectName(QString::fromUtf8("CBTFim"));
        sizePolicy.setHeightForWidth(CBTFim->sizePolicy().hasHeightForWidth());
        CBTFim->setSizePolicy(sizePolicy);
        CBTFim->setMaximumSize(QSize(50, 16777215));

        horizontalLayout->addWidget(CBTFim);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        splitter_5 = new QSplitter(DialogCar);
        splitter_5->setObjectName(QString::fromUtf8("splitter_5"));
        splitter_5->setOrientation(Qt::Horizontal);
        BConcatenar = new QPushButton(splitter_5);
        BConcatenar->setObjectName(QString::fromUtf8("BConcatenar"));
        BConcatenar->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(BConcatenar->sizePolicy().hasHeightForWidth());
        BConcatenar->setSizePolicy(sizePolicy1);
        splitter_5->addWidget(BConcatenar);

        horizontalLayout->addWidget(splitter_5);

        BCarregar = new QPushButton(DialogCar);
        BCarregar->setObjectName(QString::fromUtf8("BCarregar"));
        BCarregar->setEnabled(false);
        sizePolicy1.setHeightForWidth(BCarregar->sizePolicy().hasHeightForWidth());
        BCarregar->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(BCarregar);

        BCancel = new QPushButton(DialogCar);
        BCancel->setObjectName(QString::fromUtf8("BCancel"));
        sizePolicy1.setHeightForWidth(BCancel->sizePolicy().hasHeightForWidth());
        BCancel->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(BCancel);


        verticalLayout->addLayout(horizontalLayout);

        DIC_pb = new QProgressBar(DialogCar);
        DIC_pb->setObjectName(QString::fromUtf8("DIC_pb"));
        DIC_pb->setValue(0);

        verticalLayout->addWidget(DIC_pb);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        LVStBar = new QLabel(DialogCar);
        LVStBar->setObjectName(QString::fromUtf8("LVStBar"));

        horizontalLayout_3->addWidget(LVStBar);

        horizontalSpacer_2 = new QSpacerItem(37, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        LVStBar2 = new QLabel(DialogCar);
        LVStBar2->setObjectName(QString::fromUtf8("LVStBar2"));

        horizontalLayout_3->addWidget(LVStBar2);


        verticalLayout->addLayout(horizontalLayout_3);


        retranslateUi(DialogCar);
        QObject::connect(BCarregar, SIGNAL(clicked()), DialogCar, SLOT(slot_UL_Carregar()));
        QObject::connect(BSeleciona, SIGNAL(clicked()), DialogCar, SLOT(slot_UL_Caminho()));
        QObject::connect(LECaminho, SIGNAL(textChanged(QString)), DialogCar, SLOT(slot_UL_Indicar(QString)));
        QObject::connect(BConcatenar, SIGNAL(clicked()), DialogCar, SLOT(slot_UL_Concatenar()));
        QObject::connect(LVSaida, SIGNAL(clicked(QModelIndex)), DialogCar, SLOT(slot_UL_ModelClicked()));
        QObject::connect(BCancel, SIGNAL(clicked()), DialogCar, SLOT(accept()));
        QObject::connect(DialogCar, SIGNAL(finished(int)), DialogCar, SLOT(slot_UL_Finalizar()));
        QObject::connect(LVEntradas, SIGNAL(clicked(QModelIndex)), DialogCar, SLOT(slot_UL_ModelClicked()));

        CBTIni->setCurrentIndex(2);
        CBTFim->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(DialogCar);
    } // setupUi

    void retranslateUi(QDialog *DialogCar)
    {
        DialogCar->setWindowTitle(QApplication::translate("DialogCar", "Dialog", 0 ));
        label_3->setText(QApplication::translate("DialogCar", "Nome do Arquivo:", 0 ));
        BSeleciona->setText(QApplication::translate("DialogCar", "...", 0 ));
        CBSalvar->setText(QApplication::translate("DialogCar", "Salvar", 0 ));
        groupBox->setTitle(QApplication::translate("DialogCar", "Entrada(s)", 0 ));
        groupBox_2->setTitle(QApplication::translate("DialogCar", "Saida(s)", 0 ));
#ifndef QT_NO_TOOLTIP
        label_4->setToolTip(QApplication::translate("DialogCar", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Instante para iniciar a coleta dos dados</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">OBS.: &quot;0&quot; =  coleta do inicio do arquivo </span></p></body></html>", 0 ));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("DialogCar", "T. Inicial", 0 ));
#ifndef QT_NO_TOOLTIP
        LETIni->setToolTip(QApplication::translate("DialogCar", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Instante para iniciar a coleta dos dados</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">OBS.: &quot;0&quot; =  coleta do inicio do arquivo </span></p></body></html>", 0 ));
#endif // QT_NO_TOOLTIP
        LETIni->setText(QApplication::translate("DialogCar", "0", 0 ));
        CBTIni->clear();
        CBTIni->insertItems(0, QStringList()
         << QApplication::translate("DialogCar", "seg", 0 )
         << QApplication::translate("DialogCar", "ms", 0 )
         << QApplication::translate("DialogCar", "us", 0 )
        );
#ifndef QT_NO_TOOLTIP
        label_5->setToolTip(QApplication::translate("DialogCar", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Instante para finalizar a coleta dos dados</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">OBS.: &quot;0&quot; =  coleta at\303\251 o fim do arquivo </span></p></body></html>", 0 ));
#endif // QT_NO_TOOLTIP
        label_5->setText(QApplication::translate("DialogCar", "T.  Final", 0 ));
#ifndef QT_NO_TOOLTIP
        LETFim->setToolTip(QApplication::translate("DialogCar", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Instante para finalizar a coleta dos dados</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">OBS.: &quot;0&quot; =  coleta at\303\251 o fim do arquivo </span></p></body></html>", 0 ));
#endif // QT_NO_TOOLTIP
        LETFim->setText(QApplication::translate("DialogCar", "0", 0 ));
        CBTFim->clear();
        CBTFim->insertItems(0, QStringList()
         << QApplication::translate("DialogCar", "seg", 0 )
         << QApplication::translate("DialogCar", "ms", 0 )
         << QApplication::translate("DialogCar", "us", 0 )
        );
        BConcatenar->setText(QApplication::translate("DialogCar", "Concatenar", 0 ));
        BCarregar->setText(QApplication::translate("DialogCar", "Carregar", 0 ));
        BCancel->setText(QApplication::translate("DialogCar", "Cancel", 0 ));
        LVStBar->setText(QApplication::translate("DialogCar", "Aguardando Usu\303\241rio...", 0 ));
        LVStBar2->setText(QApplication::translate("DialogCar", "Linhas: 0, Colunas: 0", 0 ));
    } // retranslateUi

};

namespace Ui {
    class DialogCar: public Ui_DialogCar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ICARREGAR_H
