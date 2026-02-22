/********************************************************************************
** Form generated from reading UI file 'dialogConfig.ui'
**
** Created: Sun 30. Dec 14:09:11 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGCONFIG_H
#define UI_DIALOGCONFIG_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogConfig
{
public:
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout_6;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QSpinBox *sb_DC_Cromo;
    QLabel *label;
    QCheckBox *cb_DC_TComo;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QCheckBox *cb_DC_config;
    QPushButton *pb_SArquivo;
    QVBoxLayout *verticalLayout_6;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QSpinBox *sb_DC_Cromo_2;
    QLabel *label_2;
    QCheckBox *cb_DC_TComo_2;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QSpinBox *sb_DC_Cromo_3;
    QLabel *label_3;
    QCheckBox *cb_DC_TComo_3;
    QCheckBox *cb_DC_Auto;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pb_DC_Concatenar;
    QPushButton *pb_DC_Carregar;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pb_DC_Salvar;
    QPushButton *pb_Fechar;

    void setupUi(QDialog *DialogConfig)
    {
        if (DialogConfig->objectName().isEmpty())
            DialogConfig->setObjectName(QString::fromUtf8("DialogConfig"));
        DialogConfig->resize(409, 283);
        verticalLayout_7 = new QVBoxLayout(DialogConfig);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        groupBox = new QGroupBox(DialogConfig);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        sb_DC_Cromo = new QSpinBox(groupBox);
        sb_DC_Cromo->setObjectName(QString::fromUtf8("sb_DC_Cromo"));
        sb_DC_Cromo->setEnabled(false);
        sb_DC_Cromo->setMaximum(9999);

        horizontalLayout_3->addWidget(sb_DC_Cromo);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_3->addWidget(label);


        verticalLayout_2->addLayout(horizontalLayout_3);

        cb_DC_TComo = new QCheckBox(groupBox);
        cb_DC_TComo->setObjectName(QString::fromUtf8("cb_DC_TComo"));
        cb_DC_TComo->setChecked(true);

        verticalLayout_2->addWidget(cb_DC_TComo);


        verticalLayout_5->addWidget(groupBox);

        groupBox_2 = new QGroupBox(DialogConfig);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        cb_DC_config = new QCheckBox(groupBox_2);
        cb_DC_config->setObjectName(QString::fromUtf8("cb_DC_config"));
        cb_DC_config->setChecked(true);

        verticalLayout->addWidget(cb_DC_config);

        pb_SArquivo = new QPushButton(groupBox_2);
        pb_SArquivo->setObjectName(QString::fromUtf8("pb_SArquivo"));

        verticalLayout->addWidget(pb_SArquivo);


        verticalLayout_5->addWidget(groupBox_2);


        horizontalLayout_6->addLayout(verticalLayout_5);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        groupBox_3 = new QGroupBox(DialogConfig);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        sb_DC_Cromo_2 = new QSpinBox(groupBox_3);
        sb_DC_Cromo_2->setObjectName(QString::fromUtf8("sb_DC_Cromo_2"));
        sb_DC_Cromo_2->setEnabled(false);
        sb_DC_Cromo_2->setMaximum(9999);

        horizontalLayout_4->addWidget(sb_DC_Cromo_2);

        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);


        verticalLayout_3->addLayout(horizontalLayout_4);

        cb_DC_TComo_2 = new QCheckBox(groupBox_3);
        cb_DC_TComo_2->setObjectName(QString::fromUtf8("cb_DC_TComo_2"));
        cb_DC_TComo_2->setChecked(true);

        verticalLayout_3->addWidget(cb_DC_TComo_2);


        verticalLayout_6->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(DialogConfig);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_4 = new QVBoxLayout(groupBox_4);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        sb_DC_Cromo_3 = new QSpinBox(groupBox_4);
        sb_DC_Cromo_3->setObjectName(QString::fromUtf8("sb_DC_Cromo_3"));
        sb_DC_Cromo_3->setEnabled(false);
        sb_DC_Cromo_3->setMaximum(9999);

        horizontalLayout_5->addWidget(sb_DC_Cromo_3);

        label_3 = new QLabel(groupBox_4);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_5->addWidget(label_3);


        verticalLayout_4->addLayout(horizontalLayout_5);

        cb_DC_TComo_3 = new QCheckBox(groupBox_4);
        cb_DC_TComo_3->setObjectName(QString::fromUtf8("cb_DC_TComo_3"));
        cb_DC_TComo_3->setChecked(true);

        verticalLayout_4->addWidget(cb_DC_TComo_3);


        verticalLayout_6->addWidget(groupBox_4);


        horizontalLayout_6->addLayout(verticalLayout_6);


        verticalLayout_7->addLayout(horizontalLayout_6);

        cb_DC_Auto = new QCheckBox(DialogConfig);
        cb_DC_Auto->setObjectName(QString::fromUtf8("cb_DC_Auto"));
        cb_DC_Auto->setChecked(true);

        verticalLayout_7->addWidget(cb_DC_Auto);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        pb_DC_Concatenar = new QPushButton(DialogConfig);
        pb_DC_Concatenar->setObjectName(QString::fromUtf8("pb_DC_Concatenar"));

        horizontalLayout_2->addWidget(pb_DC_Concatenar);

        pb_DC_Carregar = new QPushButton(DialogConfig);
        pb_DC_Carregar->setObjectName(QString::fromUtf8("pb_DC_Carregar"));

        horizontalLayout_2->addWidget(pb_DC_Carregar);


        verticalLayout_7->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pb_DC_Salvar = new QPushButton(DialogConfig);
        pb_DC_Salvar->setObjectName(QString::fromUtf8("pb_DC_Salvar"));

        horizontalLayout->addWidget(pb_DC_Salvar);

        pb_Fechar = new QPushButton(DialogConfig);
        pb_Fechar->setObjectName(QString::fromUtf8("pb_Fechar"));

        horizontalLayout->addWidget(pb_Fechar);


        verticalLayout_7->addLayout(horizontalLayout);


        retranslateUi(DialogConfig);
        QObject::connect(cb_DC_TComo, SIGNAL(clicked(bool)), sb_DC_Cromo, SLOT(setDisabled(bool)));
        QObject::connect(pb_Fechar, SIGNAL(clicked()), DialogConfig, SLOT(accept()));

        QMetaObject::connectSlotsByName(DialogConfig);
    } // setupUi

    void retranslateUi(QDialog *DialogConfig)
    {
        DialogConfig->setWindowTitle(QApplication::translate("DialogConfig", "Dialog", 0 ));
        groupBox->setTitle(QApplication::translate("DialogConfig", "Quatidade de Cromossomos Salvos", 0 ));
#ifndef QT_NO_WHATSTHIS
        sb_DC_Cromo->setWhatsThis(QApplication::translate("DialogConfig", "Informe a ", 0 ));
#endif // QT_NO_WHATSTHIS
        label->setText(QApplication::translate("DialogConfig", "Cromossomos", 0 ));
        cb_DC_TComo->setText(QApplication::translate("DialogConfig", "Todos os cromossomos", 0 ));
        groupBox_2->setTitle(QApplication::translate("DialogConfig", "Salvar os dados carregados", 0 ));
        cb_DC_config->setText(QApplication::translate("DialogConfig", "No arquivo de configura\303\247\303\243o", 0 ));
        pb_SArquivo->setText(QApplication::translate("DialogConfig", "Salvar em arquivo *.txt", 0 ));
        groupBox_3->setTitle(QApplication::translate("DialogConfig", "Quatidade de Cromossomos Salvos", 0 ));
#ifndef QT_NO_WHATSTHIS
        sb_DC_Cromo_2->setWhatsThis(QApplication::translate("DialogConfig", "Informe a ", 0 ));
#endif // QT_NO_WHATSTHIS
        label_2->setText(QApplication::translate("DialogConfig", "Cromossomos", 0 ));
        cb_DC_TComo_2->setText(QApplication::translate("DialogConfig", "Todos os cromossomos", 0 ));
        groupBox_4->setTitle(QApplication::translate("DialogConfig", "Quatidade de Cromossomos Salvos", 0 ));
#ifndef QT_NO_WHATSTHIS
        sb_DC_Cromo_3->setWhatsThis(QApplication::translate("DialogConfig", "Informe a ", 0 ));
#endif // QT_NO_WHATSTHIS
        label_3->setText(QApplication::translate("DialogConfig", "Cromossomos", 0 ));
        cb_DC_TComo_3->setText(QApplication::translate("DialogConfig", "Todos os cromossomos", 0 ));
        cb_DC_Auto->setText(QApplication::translate("DialogConfig", "Salvar automaticamente.", 0 ));
        pb_DC_Concatenar->setText(QApplication::translate("DialogConfig", "Concatenar", 0 ));
        pb_DC_Carregar->setText(QApplication::translate("DialogConfig", "Carregar ", 0 ));
        pb_DC_Salvar->setText(QApplication::translate("DialogConfig", "Salvar Agora", 0 ));
        pb_Fechar->setText(QApplication::translate("DialogConfig", "Fechar", 0 ));
    } // retranslateUi

};

namespace Ui {
    class DialogConfig: public Ui_DialogConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGCONFIG_H
