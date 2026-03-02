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
TARGET = IDS_DEStruct_Refactored

CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
    DEFINES += _DEBUG
}

DESTDIR = $$OUT_PWD/release

#################################################################
# Qwt Library
#################################################################
QWT_LOCATION = $$PWD/qwt

INCLUDEPATH += $${QWT_LOCATION}/src
DEPENDPATH  += $${QWT_LOCATION}/src

win32:CONFIG(release, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwtd
else:unix: LIBS += -L$${QWT_LOCATION}/lib -lqwt

DEFINES += QT_DLL QWT_DLL

#################################################################
# Include paths for original shared headers
#################################################################
ORIGINAL_ROOT = $$PWD/..

INCLUDEPATH += $$ORIGINAL_ROOT
INCLUDEPATH += $$PWD/interfaces
INCLUDEPATH += $$PWD/services
INCLUDEPATH += $$PWD/threading
INCLUDEPATH += $$PWD/ui

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
    services/chromosome_service.cpp \
    services/data_service.cpp \
    services/evolution_engine.cpp \
    services/equation_formatter.cpp \
    services/xml_config_persistence.cpp \
    services/destruct_statics_stub.cpp \
    ui/main_window.cpp \
    $$ORIGINAL_ROOT/icarregar.cpp \
    $$ORIGINAL_ROOT/xbelreader.cpp \
    $$ORIGINAL_ROOT/xmlreaderwriter.cpp \
    $$ORIGINAL_ROOT/xvetor.cpp \
    $$ORIGINAL_ROOT/xmatriz.cpp \
    $$ORIGINAL_ROOT/norwegianwoodstyle.cpp \
    $$ORIGINAL_ROOT/designerworkaround.cpp

HEADERS += \
    interfaces/i_chromosome_service.h \
    interfaces/i_data_service.h \
    interfaces/i_evolution_engine.h \
    interfaces/i_config_persistence.h \
    interfaces/i_equation_formatter.h \
    threading/shared_state.h \
    threading/thread_worker.h \
    services/linear_algebra.h \
    services/chromosome_service.h \
    services/data_service.h \
    services/evolution_engine.h \
    services/equation_formatter.h \
    services/xml_config_persistence.h \
    ui/main_window.h \
    $$ORIGINAL_ROOT/icarregar.h \
    $$ORIGINAL_ROOT/xbelreader.h \
    $$ORIGINAL_ROOT/xmlreaderwriter.h \
    $$ORIGINAL_ROOT/xvetor.h \
    $$ORIGINAL_ROOT/xmatriz.h \
    $$ORIGINAL_ROOT/xtipodados.h \
    $$ORIGINAL_ROOT/mtrand.h \
    $$ORIGINAL_ROOT/norwegianwoodstyle.h \
    $$ORIGINAL_ROOT/designerworkaround.h

FORMS += \
    $$ORIGINAL_ROOT/imainwindow.ui \
    $$ORIGINAL_ROOT/icarregar.ui \
    $$ORIGINAL_ROOT/dialogMaxMin.ui \
    $$ORIGINAL_ROOT/dialogConfig.ui \
    $$ORIGINAL_ROOT/dialogDecimacao.ui

#################################################################
# Resources
#################################################################
RESOURCES += $$ORIGINAL_ROOT/qrc/images.qrc

#################################################################
# Platform-specific settings
#################################################################
win32 {
    RC_FILE = $$ORIGINAL_ROOT/qrc/icon.rc

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
