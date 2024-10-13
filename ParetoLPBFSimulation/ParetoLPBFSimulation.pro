#-------------------------------------------------
# ----------------QTPareto------------------------
#-------------------------------------------------

QT += core gui
QT += opengl xml
QT += network
QT += charts

PROJECT = paretoAMBuild

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ParetoLPBF
TEMPLATE = app

RESOURCES = qtResources.qrc

unix:!macx: QMAKE_CXXFLAGS = -fopenmp -fPIC

macx: QMAKE_CXXFLAGS =  -fPIC
#macx: QMAKE_CXX = /usr/local/bin/g++-6
#macx: QMAKE_LINK = /usr/local/bin/g++-6

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_QT

unix:INCLUDEPATH += ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc  ../ParetoTopOpt/inc/ \

macx:INCLUDEPATH += ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc ../ParetoTopOpt/inc/ \

win32:INCLUDEPATH += .\ ..\ParetoCore\inc\ ..\ParetoCore\inc\hexmesh  ..\ExternalLibs \
                        ..\ExternalLibs\QGLViewer\inc .\Ui_Files \
                         .\inc\ .\Ui\inc\ ../ParetoTopOpt/inc/

unix:UI_DIR = ./Ui/ui
macx:UI_DIR = ./Ui/ui
win32:UI_DIR = .\Ui\ui

CONFIG += release warn_off c++11


win32:LIBS +=-L"$$PWD/../ExternalLibs/QGLViewer/lib/windows" -lQGLViewer2 -lGLU32
unix:!macx: LIBS += -L "$$PWD/../ExternalLibs/QGLViewer/lib/unix" -lQGLViewer
unix:!macx: LIBS += -lGLU -lm
macx:LIBS += -L"$$PWD/../ExternalLibs/QGLViewer/lib/macos" -lQGLViewer -framework OpenGL

win32:LIBS +=-L"$$PWD/../ParetoCore/build" -lpareto -lopengl32
unix:!macx: LIBS += -L../ParetoCore/build  -lpareto -lgomp
macx: LIBS += -L"$$PWD/../ParetoCore/build" -lpareto

## (un)comment the following line to (include) exclude CUDA code
#if CUDA code needs to be compiled, the headers with COMPILE_GPUCODE needs to be used
#win32:LIBS += -L"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v11.1\\lib\\x64" -lcuda -lcudart_static -lcublas

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
    Ui/src/ui_structuralAnalysisDialog.cpp \
    Ui/src/ui_thermalLoadsWindow.cpp \
    Ui\src\ui_mainWindow.cpp \
    Ui\src\ui_controlDialog.cpp \
    Ui\src\ui_unitsDialog.cpp \
    Ui\src\ui_geometryDialog.cpp \
    Ui\src\ui_loadPRJ.cpp \
    Ui\src\ui_materialDialog.cpp \
    Ui/src/ui_displayOptionsDialog.cpp \
    Ui/src/ui_verboseDialog.cpp \
    Ui/src/ui_structuralLoadsWindow.cpp \
    src/ParetoLPBFRenderer.cpp \
    src/ParetoLPBFViewer.cpp \
    src/Template.cpp \
    src/TemplateMain.cpp

HEADERS += \
    Ui/inc/ui_loadPRJ.hpp\
    Ui/inc/ui_mainWindow.hpp\
    Ui/inc/ui_materialDialog.hpp\
    Ui/inc/ui_structuralAnalysisDialog.hpp \
    Ui/inc/ui_thermalLoadsWindow.hpp \
    Ui/inc/ui_unitsDialog.hpp\
    Ui/inc/ui_controlDialog.hpp \
    Ui/inc/ui_geomDialog.hpp \
    Ui/inc/ui_displayOptionsDialog.hpp \
    Ui/inc/ui_verboseDialog.hpp \
    inc/ParetoApplication.h \
    Ui/inc/ui_structuralLoadsWindow.hpp \
    inc/ParetoLPBFRenderer.hpp \
    inc/ParetoLPBFViewer.hpp \
    inc/Template.hpp





