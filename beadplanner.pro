QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += xml

requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backgrounddialog.cpp \
    beadcoloritem.cpp \
    beadcolortable.cpp \
    beadlistdialog.cpp \
    beadmodel.cpp \
    beadpatternfileio.cpp \
    beadsizedialog.cpp \
    colorchange.cpp \
    customcolordialog.cpp \
    griddialog.cpp \
    imagecolorsdialog.cpp \
    imagescene.cpp \
    imageviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    palettedialog.cpp \
    settingsdialog.cpp \
    sidebar.cpp \
    tableitemdelegate.cpp \
    tablemodel.cpp \
    treemodel.cpp \
    undomanager.cpp \
    worker.cpp

HEADERS += \
    backgrounddialog.h \
    beadcolor.h \
    beadcoloritem.h \
    beadcolortable.h \
    beadlistdialog.h \
    beadmodel.h \
    beadpatternfileio.h \
    beadpixelcoordtransform.h \
    beadsizedialog.h \
    colorchange.h \
    customcolordialog.h \
    grid.h \
    griddialog.h \
    imagecolorsdialog.h \
    imagescene.h \
    imageviewer.h \
    mainwindow.h \
    palettedialog.h \
    settingsdialog.h \
    sidebar.h \
    tableitemdelegate.h \
    tablemodel.h \
    treemodel.h \
    undomanager.h \
    worker.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Colours (Hama).xml


QMAKE_CXXFLAGS += -O3 -fopenmp -ffast-math
QMAKE_LFLAGS += -fopenmp
QMAKE_CXXFLAGS_WARN_ON += -Wno-padded

RESOURCES += \
    resources.qrc \
    tango.qrc
