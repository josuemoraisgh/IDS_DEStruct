/********************************************************************************
** Form generated from reading UI file 'imainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAINWINDOW_H
#define UI_IMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
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
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1013, 600);
        actionIni = new QAction(MainWindow);
        actionIni->setObjectName(QStringLiteral("actionIni"));
        actionParar = new QAction(MainWindow);
        actionParar->setObjectName(QStringLiteral("actionParar"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionCarDados = new QAction(MainWindow);
        actionCarDados->setObjectName(QStringLiteral("actionCarDados"));
        actionConfig = new QAction(MainWindow);
        actionConfig->setObjectName(QStringLiteral("actionConfig"));
        actionZoom = new QAction(MainWindow);
        actionZoom->setObjectName(QStringLiteral("actionZoom"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        qwtPlot1 = new QwtPlot(centralWidget);
        qwtPlot1->setObjectName(QStringLiteral("qwtPlot1"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qwtPlot1->sizePolicy().hasHeightForWidth());
        qwtPlot1->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(qwtPlot1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));
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
        toolBar->setObjectName(QStringLiteral("toolBar"));
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
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "LASEC - FEELT - UFU", 0));
        actionIni->setText(QApplication::translate("MainWindow", "Iniciar", 0));
#ifndef QT_NO_TOOLTIP
        actionIni->setToolTip(QApplication::translate("MainWindow", "Inicializa a identifica\303\247\303\243o do sistema", 0));
#endif // QT_NO_TOOLTIP
        actionIni->setShortcut(QApplication::translate("MainWindow", "Ctrl+I", 0));
        actionParar->setText(QApplication::translate("MainWindow", "Parar", 0));
#ifndef QT_NO_TOOLTIP
        actionParar->setToolTip(QApplication::translate("MainWindow", "P\303\241ra a identifica\303\247\303\243o do sistema", 0));
#endif // QT_NO_TOOLTIP
        actionParar->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", 0));
#ifndef QT_NO_TOOLTIP
        actionExit->setToolTip(QApplication::translate("MainWindow", "Sai da plataforma", 0));
#endif // QT_NO_TOOLTIP
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+E", 0));
        actionCarDados->setText(QApplication::translate("MainWindow", "Carregar", 0));
#ifndef QT_NO_TOOLTIP
        actionCarDados->setToolTip(QApplication::translate("MainWindow", "Carrega os dados ", 0));
#endif // QT_NO_TOOLTIP
        actionCarDados->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0));
        actionConfig->setText(QApplication::translate("MainWindow", "Configura\303\247\303\265es", 0));
#ifndef QT_NO_TOOLTIP
        actionConfig->setToolTip(QApplication::translate("MainWindow", "Abre tela para ajustes das configura\303\247\303\265es", 0));
#endif // QT_NO_TOOLTIP
        actionConfig->setShortcut(QApplication::translate("MainWindow", "Ctrl+G", 0));
        actionZoom->setText(QApplication::translate("MainWindow", "Zoom", 0));
#ifndef QT_NO_TOOLTIP
        actionZoom->setToolTip(QApplication::translate("MainWindow", "habilita zoom na tela", 0));
#endif // QT_NO_TOOLTIP
        textEdit->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">-&gt; Inicializando...</span></p></body></html>", 0));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAINWINDOW_H
