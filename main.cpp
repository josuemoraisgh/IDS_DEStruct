#include <QApplication>
#include <QSplashScreen>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "imainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

static LONG WINAPI DES_UnhandledExceptionFilter(EXCEPTION_POINTERS* ep)
{
    const QString logPath = QDir::currentPath() + "/crash_debug.log";
    QFile f(logPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&f);
        out << "=== CRASH ===\n";
        out << "Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
        if (ep && ep->ExceptionRecord)
        {
            out << "ExceptionCode: 0x" << QString::number(ep->ExceptionRecord->ExceptionCode, 16) << "\n";
            out << "ExceptionAddress: 0x" << QString::number(reinterpret_cast<quintptr>(ep->ExceptionRecord->ExceptionAddress), 16) << "\n";
        }

        HANDLE proc = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        if (SymInitialize(proc, NULL, TRUE))
        {
            DWORD64 displacement = 0;
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
            ZeroMemory(buffer, sizeof(buffer));
            PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD lineDisplacement = 0;
            IMAGEHLP_LINE64 lineInfo;
            ZeroMemory(&lineInfo, sizeof(lineInfo));
            lineInfo.SizeOfStruct = sizeof(lineInfo);

            if (ep && ep->ExceptionRecord)
            {
                const DWORD64 addr = reinterpret_cast<DWORD64>(ep->ExceptionRecord->ExceptionAddress);
                if (SymFromAddr(proc, addr, &displacement, symbol))
                {
                    out << "Symbol: " << symbol->Name << " +0x" << QString::number(displacement, 16) << "\n";
                }
                if (SymGetLineFromAddr64(proc, addr, &lineDisplacement, &lineInfo))
                {
                    out << "Source: " << lineInfo.FileName << ":" << lineInfo.LineNumber << "\n";
                }
            }
            SymCleanup(proc);
        }
        out << "\n";
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main( int argc, char** argv )
{
#ifdef Q_OS_WIN
        SetUnhandledExceptionFilter(DES_UnhandledExceptionFilter);
#endif
        QApplication app(argc, argv);
        app.setOrganizationName("LASEC - FEELT - UFU");
        app.setApplicationName("IDS_AGStruct");
        // Removed obsolete setStyle("plastique") - not available in Qt 5+
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
