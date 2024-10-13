#-------------------------------------------------
# ----------------QTPareto------------------------
#-------------------------------------------------
# to use cuda code, uncomment below
#include(./cudaParetoWinInclude.pri)

QT += core gui
QT += opengl xml
QT += network
QT += charts



PROJECT = pareto

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pareto
TEMPLATE = app

RESOURCES = qtResources.qrc

unix:!macx: QMAKE_CXXFLAGS = -fopenmp -fPIC
macx: QMAKE_CXXFLAGS =  -fPIC
win32: QMAKE_CXXFLAGS = -MP

#macx: QMAKE_CXX = /usr/local/bin/g++-6
#macx: QMAKE_LINK = /usr/local/bin/g++-6

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_QT

unix:INCLUDEPATH += ./ ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc ../ExternalLibs/tetgen

macx:INCLUDEPATH += ./ ./Ui ./inc ../ParetoCore/inc/ ../ParetoCore/inc/hexmesh $(EIGEN_DIR)/include/eigen3 ../ExternalLibs \
                    ../ExternalLibs/QGLViewer/inc ./Ui/inc ../ExternalLibs/tetgen

win32:INCLUDEPATH += .\ ..\ParetoCore\inc\ ..\ParetoCore\inc\hexmesh  ..\ExternalLibs \
                        ..\ExternalLibs\QGLViewer\inc .\Ui_Files  ..\ExternalLibs\tetgen  \
                         .\inc\ .\Ui\inc\ ..\ExternalLibs\pmp\inc ..\ExternalLibs\qcustomplot

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
    src/Renderer.cpp \
    src/CoreFunctions.cpp \
    Ui\src\ui_mainWindow.cpp \
    Ui\src\ui_controlDialog.cpp \
    Ui\src\ui_unitsDialog.cpp \
    Ui\src\ui_geometryDialog.cpp \
    Ui\src\ui_loadPRJ.cpp \
    Ui\src\ui_materialDialog.cpp \
    Ui\src\ui_structuralAnalysisDialog.cpp \
    Ui\src\ui_accelerationWindow.cpp \
    Ui\src\ui_topologyOptimizationDialog.cpp \
    Ui\src\ui_structuralLoadsWindow.cpp \
    Ui/src/ui_thermalLoadsWindow.cpp \
    Ui/src/ui_constraintsDialog.cpp \
    Ui/src/ui_displayOptionsDialog.cpp \
    Ui/src/ui_optimizationResultDialog.cpp \
    Ui/src/ui_toolsDialog.cpp \
    Ui/src/ui_verboseDialog.cpp \
    Ui/src/ui_latticeStructureDialog.cpp \
    Ui/src/TopOptViewer.cpp \
    Ui/src/ui_materialOptimizationDialog.cpp \
    Ui/src/ui_AMBuild.cpp \
    src/MaterialHandler.cpp \
    src/text3d.cpp \
    Ui/src/ui_autoDesignDialog.cpp \
    src/main.cpp \
    Ui/src/ui_elastoPlasticDialog.cpp \
    ../ExternalLibs/tetgen/tetgen.cxx \
    ../ExternalLibs/tetgen/predicates.cxx \
    ../ExternalLibs/tetgen/DelaunaySurface.cpp \
    Ui/src/ui_meshMergingDialog.cpp \
    Ui/src/ui_meshProjectionDialog.cpp \
    Ui/src/ui_ZernikeDialog.cpp \
    Ui/src/ui_frequencyAnalysisDialog.cpp \
    Ui/src/ui_thermalOptimizationDialog.cpp


HEADERS += \
    inc/Renderer.hpp \
    inc/CoreFunctions.hpp \
    inc/LiveVariables.hpp \
    Ui/inc/ui_accelerationWindow.hpp\
    Ui/inc/ui_loadPRJ.hpp\
    Ui/inc/ui_mainWindow.hpp\
    Ui/inc/ui_materialDialog.hpp\
    Ui/inc/ui_topologyOptimizationDialog.hpp\
    Ui/inc/ui_unitsDialog.hpp\
    Ui/inc/ui_controlDialog.hpp \
    Ui/inc/ui_structuralLoadsWindow.hpp \
    Ui/inc/ui_thermalLoadsWindow.hpp \
    inc/text3D.hpp \
    inc/text3DCharSet.hpp \
    Ui/inc/ui_constraintsDialog.hpp \
    Ui/inc/ui_geomDialog.hpp \
    Ui/inc/ui_displayOptionsDialog.hpp \
    Ui/inc/ui_optimizationResultDialog.hpp \
    Ui\inc\ui_toolsDialog.hpp \
    Ui/inc/ui_verboseDialog.hpp \
    Ui/inc/ui_latticeStructureDialog.hpp \
    Ui/inc/ui_materialOptimizationDialog.hpp \
    Ui/inc/ui_AMBuild.hpp \
    Ui/inc/ui_structuralAnalysisDialog.hpp \
    Ui/inc/ui_autoDesignDialog.hpp \
    Ui/inc/TopOptViewer.hpp \
    inc/precompiledheader.h \
    Ui/inc/ui_elastoPlasticDialog.hpp \
    ../ExternalLibs/tetgen/tetgen.h \
    ../ExternalLibs/tetgen/trilib.hpp \
    ../ExternalLibs/tetgen/veclib.hpp \
    ../ExternalLibs/tetgen/DelaunaySurface.h \
    Ui/inc/ui_meshMergingDialog.hpp \
    Ui/inc/ui_meshProjectionDialog.hpp \
    Ui/inc/ui_ZernikeDialog.hpp \
    Ui/inc/ui_frequencyAnalysisDialog.hpp \
    Ui/inc/ui_thermalOptimizationDialog.hpp



