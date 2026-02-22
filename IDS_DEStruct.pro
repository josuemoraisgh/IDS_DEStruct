DIRETORIO = C:
DIRETORIO1 = C:

#################################################################
# QWT Location
QWT_LOCATION = ..\\qwt

# Installation paths (optional)
target.path    += $${QWT_LOCATION}\\lib
headers.path   += $${QWT_LOCATION}\\include
doc.path       += $${QWT_LOCATION}\\doc
#################################################################

# Debug/Release configuration
DEBUG_SUFFIX = d
RELEASE_SUFFIX =

CONFIG(debug, debug|release) {
    SUFFIX_STR = $${DEBUG_SUFFIX}
    DIR_STR = Debug
    DEFINES += _DEBUG
} else {
    SUFFIX_STR = $${RELEASE_SUFFIX}
    DIR_STR = Release
}

OBJECTS_DIR  = obj$${SUFFIX_STR}
MOC_DIR      = moc$${SUFFIX_STR}
RCC_DIR      = rcc$${SUFFIX_STR}
UI_DIR       = ui$${SUFFIX_STR}

#################################################################
# QWT Configuration
INCLUDEPATH += $${QWT_LOCATION}\\src
DEPENDPATH  += $${QWT_LOCATION}\\src
DEFINES     += QT_DLL QWT_DLL

# Link with QWT - Versão correta
win32:CONFIG(release, debug|release): LIBS += -L$${QWT_LOCATION}\\lib -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_LOCATION}\\lib -lqwtd

#################################################################
# Qt Modules
QT += opengl svg script
contains(QT_CONFIG, scripttools): QT += scripttools

CONFIG += qt warn_on thread
TEMPLATE = app
RESOURCES = qrc/images.qrc

# Target name and destination
TARGET = IDS_DEStruct$${SUFFIX_STR}
DESTDIR = $${DIRETORIO1}\\SourceCode\\IDS_DEStruct\\Exec\\$${DIR_STR}

SOURCES += \
    main.cpp \
    imainwindow.cpp \
    destruct.cpp \
    icarregar.cpp \
    xbelreader.cpp \
    xmlreaderwriter.cpp

HEADERS += \
    mtrand.h \
    imainwindow.h \
    destruct.h \
    icarregar.h \
    xvetor.h \
    xmatriz.h \
    xtipodados.h \
    xbelreader.h \
    xmlreaderwriter.h

FORMS += \
    imainwindow.ui \
    icarregar.ui \
    dialogMaxMin.ui \
    dialogConfig.ui \
    dialogDecimacao.ui

OTHER_FILES +=