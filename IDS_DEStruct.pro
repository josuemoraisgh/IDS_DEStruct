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

CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/debug
} else:CONFIG(release, debug|release) {
    DESTDIR = $$OUT_PWD/release
}

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
INCLUDEPATH += $$PROJECT_ROOT/interfaces
INCLUDEPATH += $$PROJECT_ROOT/core
INCLUDEPATH += $$PROJECT_ROOT/io
INCLUDEPATH += $$PROJECT_ROOT/threading
INCLUDEPATH += $$PROJECT_ROOT/ui
INCLUDEPATH += $$PROJECT_ROOT/qrc

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
    core/adaptive_tuning_engine.cpp \
    core/xvetor.cpp \
    core/xmatriz.cpp \
    io/data_service.cpp \
    io/xml_config_persistence.cpp \
    io/xbelreader.cpp \
    io/xmlreaderwriter.cpp \
    ui/equation_formatter.cpp \
    ui/main_window.cpp \
    ui/icarregar.cpp \
    ui/norwegianwoodstyle.cpp \
    ui/designerworkaround.cpp

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
    core/adaptive_tuning_engine.h \
    core/adaptive_state.h \
    core/model_pruning_config.h \
    core/xtipodados.h \
    core/mtrand.h \
    core/xvetor.h \
    core/xmatriz.h \
    io/data_service.h \
    io/xml_config_persistence.h \
    io/xbelreader.h \
    io/xmlreaderwriter.h \
    ui/equation_formatter.h \
    ui/main_window.h \
    ui/icarregar.h \
    ui/norwegianwoodstyle.h \
    ui/designerworkaround.h

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

    # Keep qwt*.dll next to the executable after linking.
    CONFIG(debug, debug|release) {
        QWT_DLL_TO_COPY = $$QWT_LOCATION/lib/qwtd.dll
        !exists($$QWT_DLL_TO_COPY): QWT_DLL_TO_COPY = $$QWT_LOCATION/lib/qwt.dll
    } else {
        QWT_DLL_TO_COPY = $$QWT_LOCATION/lib/qwt.dll
        !exists($$QWT_DLL_TO_COPY): QWT_DLL_TO_COPY = $$QWT_LOCATION/lib/qwtd.dll
    }
    QMAKE_POST_LINK += $$quote(if exist \"$$shell_path($$QWT_DLL_TO_COPY)\" copy /Y \"$$shell_path($$QWT_DLL_TO_COPY)\" \"$$shell_path($$DESTDIR)\" > nul 2>&1)

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
