#-------------------------------------------------
#
# Project - ParetoQT
#
#-------------------------------------------------

# to compile cuda code,
# First download and install CUDA 11.1 toolkit.
# Then place the files in C:\Users\ksure\Dropbox\Personal\suresh\Programs\CUDAPrograms\Nvcuda  in the folder
# C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\VCProjectDefaults

# 1. uncomment the line #define COMPILE_GPUCODE in CommonDefines.h
# 2. uncomment line below
# 3. uncomment the line include(./cudaParetoWinInclude.pri) in paretoTopOpt.pro
#include(./cudaParetoCoreInclude.pri)

TARGET = pareto
TEMPLATE = lib
CONFIG += staticlib
CONFIG += release warn_off c++11

unix:!macx: QMAKE_CXXFLAGS = -fopenmp -fPIC
macx: QMAKE_CXXFLAGS =  -fPIC
win32: QMAKE_CXXFLAGS = /openmp -MP


# for staticlib build, uncomment the following 3 lines
# then clean, qmake and build. After build, remember to comment the lines after build
#win32: QMAKE_CXXFLAGS_RELEASE +=/MT
#win32: QMAKE_CFLAGS_RELEASE +=/MT
#TARGET = paretostatic

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_QT

unix:OBJECTS_DIR = ./tmp
macx:OBJECTS_DIR = ./tmp
win32:OBJECTS_DIR = .\tmp

unix : MOC_DIR = ./tmp
macx : MOC_DIR = ./tmp
win32 : MOC_DIR = .\tmp

unix: DESTDIR = ./build
macx: DESTDIR = ./build
win32: DESTDIR = .\build

INCLUDEPATH += ./inc ./inc/hexmesh

unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(./SourceFiles/sources.pri)
include(./inc/hexmesh/includes.pri)

