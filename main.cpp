#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>

#include "threading/shared_state.h"
#include "ui/equation_formatter.h"
#include "io/xml_config_persistence.h"
#include "ui/main_window.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("LASEC - FEELT - UFU");
    app.setApplicationName("IDS_AGStruct");
    app.setStyle("Fusion");

    QSplashScreen splash;
    splash.setPixmap(QPixmap(":/images/lasec.png"));
    splash.maximumSize();
    splash.show();
    splash.showMessage(QObject::tr("Abrindo Janela Principal ..."),
                       Qt::AlignRight | Qt::AlignTop, Qt::black);

    SharedState state;
    EquationFormatter eqFormatter(&state);
    XmlConfigPersistence configPersist(&state);

    MainWindow mainwindow(&state);
    mainwindow.setEquationFormatter(&eqFormatter);
    mainwindow.setConfigPersistence(&configPersist);

    QTimer::singleShot(1000, &splash, SLOT(close()));
    QTimer::singleShot(1000, &mainwindow, SLOT(show()));

    return app.exec();
}
