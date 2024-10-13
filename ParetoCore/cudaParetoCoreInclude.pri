#----------------------------------------------------------------
#-------------------------Cuda setup-----------------------------
#----------------------------------------------------------------

OTHER_FILES +=\
AFElasticityGPU.cu \

CUDA_OBJECTS_DIR=.\tmp
#Enter your gencode here!
GENCODE = arch=compute_30,code=sm_30

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

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib/$$SYSTEM_NAME

# The following library conflicts with something in Cuda
#QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:msvcrt.lib
LIBS += -lcuda -lcudart -lcufft

# MSVCRT link option (static or dynamic, it must be the same with your Qt SDK link option)
MSVCRT_LINK_FLAG_DEBUG = "/MDd"
MSVCRT_LINK_FLAG_RELEASE = "/MD"
QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:libc.lib
CUDA_INC = $$join(INCLUDEPATH,'"-I"','-I"','"')


QMAKE_EXTRA_COMPILERS += cuda
# Configuration of the Cuda compiler
    # Release mode
    cuda.input = CUDA_SOURCES
    cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.obj
    cuda.commands = $$CUDA_DIR/bin/nvcc.exe $$NVCC_OPTIONS $$CUDA_INC $$LIBS \
                    --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH \
                    --compile -cudart static -DWIN64 -D_MBCS \
                    -Xcompiler $$MSVCRT_LINK_FLAG_RELEASE \
                    -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda.dependency_type = TYPE_C

INCLUDEPATH += ./inc ./inc/hexmesh ./ProjectFiles/ParetoResearch

