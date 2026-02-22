DIRETORIO = C:
DIRETORIO1 = D:
#################################################################
#QWT3D_LOCATION = $${DIRETORIO}\\Qt\\qwtplot3d
QWT_LOCATION = $${DIRETORIO}\\qwt
#BOOST_LOCATION = $${DIRETORIO}\\QtSDK\\Boost_1_46_1\\

#target.path    += $${QWT3D_LOCATION}\\lib
#headers.path   += $${QWT3D_LOCATION}\\include
#doc.path       += $${QWT3D_LOCATION}\\doc

target.path    += $${QWT_LOCATION}\\lib
headers.path   += $${QWT_LOCATION}\\include
doc.path       += $${QWT_LOCATION}\\doc

#target.path    += $${BOOST_LOCATION}\\libs
#headers.path   += $${BOOST_LOCATION}
#doc.path       += $${BOOST_LOCATION}\\doc
#################################################################
DEBUG_SUFFIX = d
RELEASE_SUFFIX =

    CONFIG(debug, debug|release) {
        SUFFIX_STR = $${DEBUG_SUFFIX}
        DIR_STR = Debug
        DEFINES += _DEBUG
    }
    else {
        DIR_STR = Release
        SUFFIX_STR = $${RELEASE_SUFFIX}
    }
OBJECTS_DIR  = obj$${SUFFIX_STR}
MOC_DIR      = moc$${SUFFIX_STR}
#################################################################
#QWT3DLIB     = qwtplot3d$${SUFFIX_STR}
#INCLUDEPATH += $${QWT3D_LOCATION}/include
#DEPENDPATH   = $$INCLUDEPATH
#DEFINES     += QT_DLL QWT3D_DLL
##LIBS       += -L$${QWT3D_LOCATION}\lib\ -l$${QWT3DLIB}
#LIBS        += $${QWT3D_LOCATION}\lib\lib$${QWT3DLIB}.a
#################################################################
#INCLUDEPATH += $${BOOST_LOCATION}
#################################################################
QWTLIB       = qwt$${SUFFIX_STR}
VER_MAJ      =
INCLUDEPATH += $${QWT_LOCATION}\\src
DEPENDPATH  += $${QWT_LOCATION}\\src
DEFINES     += QT_DLL QWT_DLL
QWTLIB       = $${QWTLIB}$${VER_MAJ}
LIBS        += $${QWT_LOCATION}\\lib\\lib$${QWTLIB}.a
#LIBS      += -L$${QWT_LOCATION}\\lib
#################################################################
QT += opengl svg script
contains(QT_CONFIG, scripttools): QT += scripttools
CONFIG   += qt warn_on thread QwtDll QwtPlot QwtWidgets QwtDesigner
TEMPLATE  = app
RESOURCES = qrc/images.qrc
TARGET    = IDS_DEStruct$${SUFFIX_STR}
DESTDIR   = $${DIRETORIO1}\\Exec\\$${DIR_STR}
SOURCES += \
    main.cpp \
    imainwindow.cpp \
    destruct.cpp \
    icarregar.cpp \
    xbelreader.cpp \
    xmlreaderwriter.cpp
HEADERS  += \
    mtrand.h \
    imainwindow.h \
    destruct.h \
    icarregar.h \
    xvetor.h \
    xmatriz.h \
    xmatriz.h \
    xtipodados.h \
    xbelreader.h \
    xmlreaderwriter.h
FORMS   += imainwindow.ui \
        icarregar.ui \
        dialogMaxMin.ui \
        dialogConfig.ui \
    dialogDecimacao.ui

OTHER_FILES +=
