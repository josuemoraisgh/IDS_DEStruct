#################################################################
# IDS_DEStruct - Differential Evolution Optimization
#################################################################
QT += core gui widgets opengl svg script

# ScriptTools module removed - not available in Qt 5.15.2

CONFIG += qt warn_on thread c++11
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#################################################################
# Target Configuration
#################################################################
TARGET = IDS_DEStruct

# Debug/Release suffixes
CONFIG(debug, debug|release) {
    TARGET = $${TARGET}d
    DEFINES += _DEBUG
}

# Output directory (relative to build directory)
DESTDIR = $$OUT_PWD/release

#################################################################
# Qwt Library Location (relative to project root)
#################################################################
QWT_LOCATION = $$PWD/qwt

INCLUDEPATH += $${QWT_LOCATION}/src
DEPENDPATH  += $${QWT_LOCATION}/src

# Qwt library linking
win32:CONFIG(release, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_LOCATION}/lib -lqwtd
else:unix: LIBS += -L$${QWT_LOCATION}/lib -lqwt

# Define DLL usage
DEFINES += QT_DLL QWT_DLL

#################################################################
# Build Directories (keeps project clean)
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
    imainwindow.cpp \
    destruct.cpp \
    icarregar.cpp \
    xbelreader.cpp \
    xmlreaderwriter.cpp \
    xvetor.cpp \
    xmatriz.cpp \
    norwegianwoodstyle.cpp \
    srlevmarq.cpp \
    designerworkaround.cpp

HEADERS += \
    imainwindow.h \
    destruct.h \
    icarregar.h \
    xbelreader.h \
    xmlreaderwriter.h \
    xvetor.h \
    xmatriz.h \
    xtipodados.h \
    mtrand.h \
    norwegianwoodstyle.h \
    srlevmarq.h \
    designerworkaround.h

FORMS += \
    imainwindow.ui \
    icarregar.ui \
    dialogMaxMin.ui \
    dialogConfig.ui \
    dialogDecimacao.ui

#################################################################
# Resources
#################################################################
RESOURCES += qrc/images.qrc

#################################################################
# Platform-specific settings
#################################################################
win32 {
    RC_FILE = qrc/icon.rc
    QMAKE_LFLAGS_RELEASE += /INCREMENTAL:NO
    # Suppress deprecated warnings on MSVC
    QMAKE_CXXFLAGS += /wd4996
}