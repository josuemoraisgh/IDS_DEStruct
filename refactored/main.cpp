#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QPixmap>
#include <QMainWindow>
#include <QDebug>

// Serviços
#include "services/DifferentialEvolutionService.h"
#include "services/MathematicalService.h"

// Infraestrutura
#include "infrastructure/FileDataRepository.h"
#include "infrastructure/XMLConfigRepository.h"

// Apresentação
#include "presentation/MainWindow.h"
#include "presentation/MainWindowPresenter.h"
#include "presentation/PlotManager.h"

/**
 * @file main.cpp
 * @brief Ponto de entrada da aplicação refatorada
 * 
 * Este arquivo demonstra a inversão de dependências:
 * - Cria as implementações concretas
 * - Injeta dependências via construtor
 * - Conecta apresentadores e serviços
 */

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    
    // Configurações da aplicação
    app.setOrganizationName("LASEC - FEELT - UFU");
    app.setApplicationName("IDS_DEStruct_Refactored");
    app.setStyle("Fusion");
    
    // Splash screen
    QSplashScreen splash;
    splash.setPixmap(QPixmap(":/images/lasec.png"));
    splash.show();
    splash.showMessage(QObject::tr("Inicializando aplicação refatorada..."), 
                      Qt::AlignRight | Qt::AlignTop, Qt::black);
    
    app.processEvents();
    
    // ========================================================================
    // INJEÇÃO DE DEPENDÊNCIAS - Criação das implementações concretas
    // ========================================================================
    
    splash.showMessage(QObject::tr("Criando serviços..."), 
                      Qt::AlignRight | Qt::AlignTop, Qt::black);
    
    // Serviços (camada de domínio/aplicação)
    auto* mathService = new Services::MathematicalService();
    auto* algorithmService = new Services::DifferentialEvolutionService(mathService);
    
    // Infraestrutura (persistência)
    auto* dataRepository = new Infrastructure::FileDataRepository();
    auto* configRepository = new Infrastructure::XMLConfigRepository();
    
    splash.showMessage(QObject::tr("Criando apresentador..."), 
                      Qt::AlignRight | Qt::AlignTop, Qt::black);
    
    // Apresentação (MVP pattern)
    auto* presenter = new Presentation::MainWindowPresenter(
        algorithmService,
        dataRepository,
        configRepository
    );
    
    auto* plotManager = new Presentation::PlotManager();
    
    // ========================================================================
    // CRIAÇÃO DA VIEW (MainWindow) - Padrão MVP
    // ========================================================================
    
    splash.showMessage(QObject::tr("Criando interface gráfica..."), 
                      Qt::AlignRight | Qt::AlignTop, Qt::black);
    
    // Cria a MainWindow (View) e setPresenter com o presentador
    auto mainWindow = std::make_unique<Presentation::MainWindow>();
    mainWindow->setPresenter(std::shared_ptr<Presentation::MainWindowPresenter>(presenter));
    
    // Conecta sinais do presenter também para log de demonstração
    QObject::connect(presenter, &Presentation::MainWindowPresenter::statusMessageChanged,
                    [](const QString& msg) {
                        qDebug() << "Status:" << msg;
                    });
    
    QObject::connect(presenter, &Presentation::MainWindowPresenter::errorOccurred,
                    [](const QString& error) {
                        qWarning() << "Erro:" << error;
                    });
    
    // Fecha splash e mostra janela principal
    QTimer::singleShot(500, [&]() { 
        splash.close();
        mainWindow->show();
    });
    
    qDebug() << "=======================================================";
    qDebug() << "Aplicação iniciada com arquitetura refatorada";
    qDebug() << "=======================================================";
    qDebug() << "Princ\u00edpios aplicados:";
    qDebug() << "- Responsabilidade \u00danica (SRP)";
    qDebug() << "- Invers\u00e3o de Depend\u00eancias (DIP)";
    qDebug() << "- Separa\u00e7\u00e3o em Camadas";
    qDebug() << "- Padr\u00e3o MVP na Apresenta\u00e7\u00e3o";
    qDebug() << "- Interfaces para desacoplamento";
    qDebug() << "=======================================================";
    
    int result = app.exec();
    
    // Cleanup - mainWindow \u00e9 deletado automaticamente via unique_ptr
    delete plotManager;
    // delete presenter;  // deletado via shared_ptr
    delete configRepository;
    delete dataRepository;
    delete algorithmService;
    delete mathService;
    
    return result;
}
