/********************************************************************************
** Form generated from reading UI file 'imainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAINWINDOW_H
#define UI_IMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qwt_plot.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionIni;
    QAction *actionParar;
    QAction *actionExit;
    QAction *actionCarDados;
    QAction *actionConfig;
    QAction *actionZoom;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QwtPlot *qwtPlot1;
    QHBoxLayout *horizontalLayout_2;
    QTextEdit *textEdit;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1013, 600);
        actionIni = new QAction(MainWindow);
        actionIni->setObjectName(QString::fromUtf8("actionIni"));
        actionParar = new QAction(MainWindow);
        actionParar->setObjectName(QString::fromUtf8("actionParar"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionCarDados = new QAction(MainWindow);
        actionCarDados->setObjectName(QString::fromUtf8("actionCarDados"));
        actionConfig = new QAction(MainWindow);
        actionConfig->setObjectName(QString::fromUtf8("actionConfig"));
        actionZoom = new QAction(MainWindow);
        actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        qwtPlot1 = new QwtPlot(centralWidget);
        qwtPlot1->setObjectName(QString::fromUtf8("qwtPlot1"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qwtPlot1->sizePolicy().hasHeightForWidth());
        qwtPlot1->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(qwtPlot1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(textEdit->sizePolicy().hasHeightForWidth());
        textEdit->setSizePolicy(sizePolicy1);
        textEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(textEdit);


        verticalLayout->addLayout(horizontalLayout_2);

        MainWindow->setCentralWidget(centralWidget);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        toolBar->addAction(actionExit);
        toolBar->addSeparator();
        toolBar->addAction(actionIni);
        toolBar->addAction(actionParar);
        toolBar->addAction(actionCarDados);
        toolBar->addAction(actionZoom);
        toolBar->addAction(actionConfig);
        toolBar->addSeparator();

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LASEC - FEELT - UFU", nullptr));
        actionIni->setText(QCoreApplication::translate("MainWindow", "Iniciar", nullptr));
#if QT_CONFIG(tooltip)
        actionIni->setToolTip(QCoreApplication::translate("MainWindow", "Inicializa a identifica\303\247\303\243o do sistema", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionIni->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I", nullptr));
#endif // QT_CONFIG(shortcut)
        actionParar->setText(QCoreApplication::translate("MainWindow", "Parar", nullptr));
#if QT_CONFIG(tooltip)
        actionParar->setToolTip(QCoreApplication::translate("MainWindow", "P\303\241ra a identifica\303\247\303\243o do sistema", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionParar->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
#if QT_CONFIG(tooltip)
        actionExit->setToolTip(QCoreApplication::translate("MainWindow", "Sai da plataforma", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCarDados->setText(QCoreApplication::translate("MainWindow", "Carregar", nullptr));
#if QT_CONFIG(tooltip)
        actionCarDados->setToolTip(QCoreApplication::translate("MainWindow", "Carrega os dados ", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionCarDados->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionConfig->setText(QCoreApplication::translate("MainWindow", "Configura\303\247\303\265es", nullptr));
#if QT_CONFIG(tooltip)
        actionConfig->setToolTip(QCoreApplication::translate("MainWindow", "Abre tela para ajustes das configura\303\247\303\265es", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionConfig->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom->setText(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
#if QT_CONFIG(tooltip)
        actionZoom->setToolTip(QCoreApplication::translate("MainWindow", "habilita zoom na tela", nullptr));
#endif // QT_CONFIG(tooltip)
        textEdit->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">-&gt; Inicializando...</span></p></body></html>", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAINWINDOW_H
