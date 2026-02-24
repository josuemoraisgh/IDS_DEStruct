#include <QApplication>
#include <QSplashScreen>
#include "imainwindow.h"

int main( int argc, char** argv )
{
        QApplication app(argc, argv);
        app.setOrganizationName("LASEC - FEELT - UFU");
        app.setApplicationName("IDS_AGStruct");
        app.setStyle("Fusion");
        QSplashScreen splash;
        splash.setPixmap(QPixmap(":/images/lasec.png"));
        splash.maximumSize();
        splash.show();
        splash.showMessage(QObject::tr("Abrindo Janela Principal ..."), Qt::AlignRight|Qt::AlignTop, Qt::black);
        ICalc mainwindow;
        QTimer::singleShot(1000, &splash, SLOT(close()));
        QTimer::singleShot(1000, &mainwindow, SLOT(show()));
        return app.exec();
}
