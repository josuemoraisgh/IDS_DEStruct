#################################################################
# IDS_DEStruct (Refactored) - Differential Evolution Optimization
# SOLID Architecture
#################################################################
QT += core gui widgets opengl svg printsupport

CONFIG += qt warn_on thread c++11
TEMPLATE = app

#################################################################
# Target Configuration
#################################################################
TARGET = IDS_DEStruct

CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
    DEFINES += _DEBUG
}

DESTDIR = $$OUT_PWD/release

#################################################################
# Qwt Library
#################################################################
PROJECT_ROOT = $$PWD
QWT_LOCATION = $$PROJECT_ROOT/qwt

INCLUDEPATH += $${QWT_LOCATION}/src
DEPENDPATH  += $${QWT_LOCATION}/src

win32:CONFIG(release, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwtd
else:unix: LIBS += -L$${QWT_LOCATION}/lib -lqwt

DEFINES += QT_DLL QWT_DLL

#################################################################
# Include paths for shared headers
#################################################################
isEmpty(LEGACY_ROOT) {
    LEGACY_ROOT = $$PROJECT_ROOT
}

!exists($$LEGACY_ROOT/xtipodados.h):exists($$PROJECT_ROOT/legacy/xtipodados.h) {
    LEGACY_ROOT = $$PROJECT_ROOT/legacy
}

!exists($$LEGACY_ROOT/xtipodados.h):exists($$PROJECT_ROOT/src/legacy/xtipodados.h) {
    LEGACY_ROOT = $$PROJECT_ROOT/src/legacy
}

!exists($$LEGACY_ROOT/xtipodados.h):exists($$PROJECT_ROOT/../xtipodados.h) {
    LEGACY_ROOT = $$PROJECT_ROOT/..
}

CARREGAR_ROOT = $$PROJECT_ROOT/qrc

INCLUDEPATH += $$LEGACY_ROOT
INCLUDEPATH += $$PROJECT_ROOT/interfaces
INCLUDEPATH += $$PROJECT_ROOT/core
INCLUDEPATH += $$PROJECT_ROOT/io
INCLUDEPATH += $$PROJECT_ROOT/threading
INCLUDEPATH += $$PROJECT_ROOT/ui
INCLUDEPATH += $$CARREGAR_ROOT

#################################################################
# Build Directories
#################################################################
CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$OUT_PWD/obj/debug
    MOC_DIR     = $$OUT_PWD/moc/debug
    RCC_DIR     = $$OUT_PWD/rcc/debug
    UI_DIR      = $$OUT_PWD/ui/debug
} else {
    OBJECTS_DIR = $$OUT_PWD/obj/release
    MOC_DIR     = $$OUT_PWD/moc/release
    RCC_DIR     = $$OUT_PWD/rcc/release
    UI_DIR      = $$OUT_PWD/ui/release
}

#################################################################
# Source Files
#################################################################
SOURCES += \
    main.cpp \
    threading/shared_state.cpp \
    threading/thread_worker.cpp \
    core/chromosome_service.cpp \
    core/evolution_engine.cpp \
    io/data_service.cpp \
    io/xml_config_persistence.cpp \
    ui/equation_formatter.cpp \
    ui/main_window.cpp \
    compat/destruct_statics_stub.cpp \
    $$LEGACY_ROOT/icarregar.cpp \
    $$LEGACY_ROOT/xbelreader.cpp \
    $$LEGACY_ROOT/xmlreaderwriter.cpp \
    $$LEGACY_ROOT/xvetor.cpp \
    $$LEGACY_ROOT/xmatriz.cpp \
    $$LEGACY_ROOT/norwegianwoodstyle.cpp \
    $$LEGACY_ROOT/designerworkaround.cpp

HEADERS += \
    interfaces/i_chromosome_service.h \
    interfaces/i_data_service.h \
    interfaces/i_evolution_engine.h \
    interfaces/i_config_persistence.h \
    interfaces/i_equation_formatter.h \
    threading/shared_state.h \
    threading/thread_worker.h \
    core/linear_algebra.h \
    core/chromosome_service.h \
    core/evolution_engine.h \
    io/data_service.h \
    io/xml_config_persistence.h \
    ui/equation_formatter.h \
    ui/main_window.h \
    $$LEGACY_ROOT/icarregar.h \
    $$LEGACY_ROOT/xbelreader.h \
    $$LEGACY_ROOT/xmlreaderwriter.h \
    $$LEGACY_ROOT/xvetor.h \
    $$LEGACY_ROOT/xmatriz.h \
    $$LEGACY_ROOT/xtipodados.h \
    $$LEGACY_ROOT/mtrand.h \
    $$LEGACY_ROOT/norwegianwoodstyle.h \
    $$LEGACY_ROOT/designerworkaround.h

FORMS += \
    ui/forms/imainwindow.ui \
    ui/forms/icarregar.ui \
    ui/forms/dialogMaxMin.ui \
    ui/forms/dialogConfig.ui \
    ui/forms/dialogDecimacao.ui

#################################################################
# Resources
#################################################################
RESOURCES += $$PROJECT_ROOT/qrc/images.qrc

#################################################################
# Platform-specific settings
#################################################################
win32 {
    RC_FILE = $$PROJECT_ROOT/qrc/icon.rc

    win32-msvc {
        QMAKE_LFLAGS_RELEASE += /INCREMENTAL:NO
        QMAKE_CXXFLAGS += /wd4996
    }
}

#################################################################
# Strict Build Profile (MSVC only)
#################################################################
win32-msvc:contains(CONFIG, strict_build) {
    message(Strict build profile enabled: /W4 /WX /permissive-)
    QMAKE_CFLAGS_WARN_ON = /W4
    QMAKE_CXXFLAGS_WARN_ON = /W4
    QMAKE_CFLAGS += /WX
    QMAKE_CXXFLAGS += /WX /permissive- /w14577 /w14456 /w14457 /w14458 /w14100 /w14189 /w14467 /wd4127
}
