#################################################################
# IDS_DEStruct Refatorado - Differential Evolution Optimization
# Arquitetura: Camadas separadas com SOLID principles
#################################################################

QT += core gui widgets opengl svg printsupport xml

CONFIG += qt warn_on thread c++11
TEMPLATE = app

#################################################################
# Target Configuration
#################################################################
TARGET = IDS_DEStruct_Refactored

CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
    DEFINES += _DEBUG
}

DESTDIR = $$PWD/build/bin

#################################################################
# Qwt Library Location
#################################################################
QWT_LOCATION = $$PWD/../qwt

INCLUDEPATH += $${QWT_LOCATION}/src
DEPENDPATH  += $${QWT_LOCATION}/src

win32:CONFIG(release, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwtd
else:unix: LIBS += -L$${QWT_LOCATION}/lib -lqwt

DEFINES += QT_DLL QWT_DLL

#################################################################
# Build Directories
#################################################################
CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/build/obj/debug
    MOC_DIR     = $$PWD/build/moc/debug
    RCC_DIR     = $$PWD/build/rcc/debug
    UI_DIR      = $$PWD/build/ui/debug
} else {
    OBJECTS_DIR = $$PWD/build/obj/release
    MOC_DIR     = $$PWD/build/moc/release
    RCC_DIR     = $$PWD/build/rcc/release
    UI_DIR      = $$PWD/build/ui/release
}

#################################################################
# Include Paths - Organizado por camadas
#################################################################
INCLUDEPATH += \
    $$PWD/domain \
    $$PWD/services \
    $$PWD/infrastructure \
    $$PWD/presentation \
    $$PWD/utils \
    $$PWD/interfaces \
    $$PWD/ui

#################################################################
# Source Files - Domain Layer (Entidades e Value Objects)
#################################################################
HEADERS += \
    domain/Term.h \
    domain/Chromosome.h \
    domain/SimulationData.h \
    domain/Configuration.h

SOURCES += \
    domain/Term.cpp \
    domain/Chromosome.cpp \
    domain/SimulationData.cpp \
    domain/Configuration.cpp

#################################################################
# Source Files - Utils Layer (Utilitários matemáticos)
#################################################################
HEADERS += \
    utils/MathVector.h \
    utils/MathMatrix.h \
    utils/RandomGenerator.h

SOURCES += \
    utils/RandomGenerator.cpp

#################################################################
# Source Files - Interfaces (Contratos - DIP)
#################################################################
HEADERS += \
    interfaces/IDataRepository.h \
    interfaces/IConfigRepository.h \
    interfaces/IAlgorithmService.h

#################################################################
# Source Files - Services Layer (Lógica de negócio)
#################################################################
HEADERS += \
    services/MathematicalService.h \
    services/DifferentialEvolutionService.h

SOURCES += \
    services/MathematicalService.cpp \
    services/DifferentialEvolutionService.cpp

#################################################################
# Source Files - Infrastructure Layer (Persistência)
#################################################################
HEADERS += \
    infrastructure/FileDataRepository.h \
    infrastructure/XMLConfigRepository.h

SOURCES += \
    infrastructure/FileDataRepository.cpp \
    infrastructure/XMLConfigRepository.cpp

#################################################################
# Source Files - Presentation Layer (MVP)
#################################################################
HEADERS += \
    presentation/MainWindow.h \
    presentation/MainWindowPresenter.h \
    presentation/PlotManager.h \
    presentation/DataLoaderDialog.h

SOURCES += \
    presentation/MainWindow.cpp \
    presentation/MainWindowPresenter.cpp \
    presentation/PlotManager.cpp \
    presentation/DataLoaderDialog.cpp

#################################################################
# Main Application
#################################################################
SOURCES += $$PWD/main.cpp

#################################################################
# UI Files (reutilizados da versão original se necessário)
#################################################################
FORMS += \
    ui/imainwindow.ui \
    ui/icarregar.ui \
    ui/dialogConfig.ui \
    ui/dialogMaxMin.ui \
    ui/dialogDecimacao.ui

#################################################################
# Resources
#################################################################
RESOURCES += ../qrc/images.qrc

#################################################################
# Platform-specific settings
#################################################################
win32 {
    RC_FILE = ../qrc/icon.rc
    
    # MSVC-specific
    *-msvc* {
        QMAKE_CXXFLAGS += /MP /W3
        DEFINES += _CRT_SECURE_NO_WARNINGS
    }
    
    # MinGW-specific
    *-g++* {
        QMAKE_CXXFLAGS += -Wno-unused-parameter
    }
}

unix {
    QMAKE_CXXFLAGS += -Wno-unused-parameter
}

#################################################################
# Informações sobre a arquitetura
#################################################################
message("=======================================================")
message("IDS_DEStruct - Arquitetura Refatorada")
message("=======================================================")
message("Estrutura de Camadas:")
message("  - Domain: Entidades e Value Objects puros")
message("  - Services: Lógica de negócio e algoritmos")
message("  - Infrastructure: Persistência e recursos externos")
message("  - Presentation: MVP para interface gráfica")
message("  - Utils: Utilitários matemáticos e auxiliares")
message("  - Interfaces: Contratos para inversão de dependências")
message("=======================================================")
message("Princípios SOLID aplicados:")
message("  - SRP: Cada classe tem uma responsabilidade única")
message("  - OCP: Aberto para extensão, fechado para modificação")
message("  - LSP: Substituição de Liskov através de interfaces")
message("  - ISP: Interfaces segregadas por funcionalidade")
message("  - DIP: Dependências apontam para abstrações")
message("=======================================================")
