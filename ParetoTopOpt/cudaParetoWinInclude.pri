#----------------------------------------------------------------
#-------------------------Cuda setup-----------------------------
#----------------------------------------------------------------
CUDA_OBJECTS_DIR=.\tmp

#We must define this as we get some confilcs in minwindef.h and helper_math.h
DEFINES += NOMINMAX

#set out cuda sources
CUDA_SOURCES = SourceFiles/AFElasticityGPU.cu

# Path to cuda toolkit install
CUDA_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.1"


SYSTEM_NAME=x64
SYSTEM_TYPE = 64
CUDA_ARCH = sm_35
NVCC_OPTIONS = --use_fast_math --allow-unsupported-compiler

# include paths
INCLUDEPATH += $$CUDA_DIR/include
               #$$CUDA_SDK/common/inc/ \
               #$$CUDA_SDK/../shared/inc/

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib/$$SYSTEM_NAME
                #$$CUDA_SDK/common/lib/$$SYSTEM_NAME \
                #$$CUDA_SDK/../shared/lib/$$SYSTEM_NAME


# The following library conflicts with something in Cuda
#QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:msvcrt.lib
# Add the necessary libraries
LIBS += -lcuda -lcudart -lcusparse -lcublas
