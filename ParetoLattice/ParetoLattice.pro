#-------------------------------------------------
# ----------------QTPareto------------------------
#-------------------------------------------------

QT += core gui
QT += opengl xml
QT += network
#QT += charts

PROJECT = paretoLattice

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ParetoLattice
TEMPLATE = app

RESOURCES = qtResources.qrc

unix:!macx: QMAKE_CXXFLAGS = -fopenmp -fPIC
macx: QMAKE_CXXFLAGS =  -fPIC
win32: QMAKE_CXXFLAGS = /openmp -MP
#macx: QMAKE_CXX = /usr/local/bin/g++-6
#macx: QMAKE_LINK = /usr/local/bin/g++-6

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_QT


unix:INCLUDEPATH += ./ ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc ../ExternalLibs/tetgen  ../ParetoTopOpt/inc/ \

macx:INCLUDEPATH += ./ ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc ../ExternalLibs/tetgen ../ParetoTopOpt/inc/  \

win32:INCLUDEPATH += .\ ..\ParetoCore\inc\ ..\ParetoCore\inc\hexmesh  ..\ExternalLibs \
                        ..\ExternalLibs\QGLViewer\inc .\Ui_Files  ..\ExternalLibs\tetgen  \
                         .\inc\ .\Ui\inc\ ..\ExternalLibs\pmp\inc ..\ParetoTopOpt\inc\


unix:UI_DIR = ./Ui/ui
macx:UI_DIR = ./Ui/ui
win32:UI_DIR = .\Ui\ui

CONFIG += release warn_off c++11

win32:LIBS +=-L"$$PWD/../ExternalLibs/QGLViewer/lib/windows" -L"$$PWD/VHACD/lib"  -lQGLViewer2 -lGLU32
unix:!macx: LIBS += -L "$$PWD/../ExternalLibs/QGLViewer/lib/unix" -lQGLViewer
unix:!macx: LIBS += -lGLU -lm
macx:LIBS += -L"$$PWD/../ExternalLibs/QGLViewer/lib/macos" -lQGLViewer -framework OpenGL

win32:LIBS +=-L"$$PWD/../ParetoCore/build" -lpareto -lopengl32
unix:!macx: LIBS += -L../ParetoCore/build  -lpareto -lgomp
macx: LIBS += -L"$$PWD/../ParetoCore/build" -lpareto

win32:LIBS +=-L"$$PWD/../ExternalLibs/pmp/lib" -lpmp
unix:OBJECTS_DIR = ./tmp
macx:OBJECTS_DIR = ./tmp
win32:OBJECTS_DIR = .\tmp

unix : MOC_DIR = ./tmp
macx : MOC_DIR = ./tmp
win32 : MOC_DIR = .\tmp

unix: DESTDIR = ./build
macx: DESTDIR = ./build
win32: DESTDIR = .\build

SOURCES += \
    Ui/src/ui_latticeGraphDialog.cpp \
    Ui/src/ui_saveFiles.cpp \
    Ui/src/ui_solidLatticeDialog.cpp \
    Ui\src\ui_mainWindow.cpp \
    Ui\src\ui_controlDialog.cpp \
    Ui\src\ui_unitsDialog.cpp \
    Ui/src/ui_displayOptionsDialog.cpp \
    Ui/src/ui_verboseDialog.cpp \
    src/LatticeGenerator.cpp \
    src/TriangulateWires.cpp \
    inc/ConvexHull/QuickHull.cpp \
    src/LatticeRenderer.cpp \
    src/LatticeViewer.cpp \
    src/LatticeMain.cpp

HEADERS += \
    Ui/inc/ui_latticeGraphDialog.hpp \
    Ui/inc/ui_mainWindow.hpp\
    Ui/inc/ui_saveFiles.hpp \
    Ui/inc/ui_solidLatticeDialog.hpp \
    Ui/inc/ui_unitsDialog.hpp\
    Ui/inc/ui_controlDialog.hpp \
    Ui/inc/ui_displayOptionsDialog.hpp \
    Ui/inc/ui_verboseDialog.hpp \
    inc/ParetoApplication.h \
    inc/LatticeGenerator.hpp \
    inc/Quaternion.hpp \
    inc/TriangulateWires.hpp \
    inc/LatticeViewer.hpp





