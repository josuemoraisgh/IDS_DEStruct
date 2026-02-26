#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include "imainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

static LONG WINAPI crashHandler(EXCEPTION_POINTERS *exInfo)
{
    // Gera um arquivo de crash dump para diagnostico
    QString dumpPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                       + QDir::separator() + QString("IDS_DEStruct_crash_%1.txt")
                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    FILE *f = fopen(dumpPath.toLocal8Bit().constData(), "w");
    if(f) {
        fprintf(f, "=== IDS_DEStruct Crash Report ===\n");
        fprintf(f, "Exception Code: 0x%08lX\n", exInfo->ExceptionRecord->ExceptionCode);
        fprintf(f, "Exception Address: 0x%p\n", exInfo->ExceptionRecord->ExceptionAddress);
        fprintf(f, "Time: %s\n", QDateTime::currentDateTime().toString(Qt::ISODate).toLocal8Bit().constData());
        if(exInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && exInfo->ExceptionRecord->NumberParameters >= 2) {
            fprintf(f, "Access Type: %s\n", exInfo->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ");
            fprintf(f, "Target Address: 0x%p\n", (void*)exInfo->ExceptionRecord->ExceptionInformation[1]);
        }
#ifdef _M_X64
        fprintf(f, "RIP: 0x%016llX\n", exInfo->ContextRecord->Rip);
        fprintf(f, "RSP: 0x%016llX\n", exInfo->ContextRecord->Rsp);
        fprintf(f, "RBP: 0x%016llX\n", exInfo->ContextRecord->Rbp);
#else
        fprintf(f, "EIP: 0x%08lX\n", exInfo->ContextRecord->Eip);
        fprintf(f, "ESP: 0x%08lX\n", exInfo->ContextRecord->Esp);
#endif
        fclose(f);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main( int argc, char** argv )
{
#ifdef Q_OS_WIN
        SetUnhandledExceptionFilter(crashHandler);
#endif
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
