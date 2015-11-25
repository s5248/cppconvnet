GXX := /usr/bin/g++
GCC := /usr/bin/gcc
CXXFLAG := -march=core2 -O2 -fomit-frame-pointer -pipe
CFLAG := $(CXXFLAG)
DIR_INC := ./include
DIR_SRC := ./src
DIR_OBJ := ./obj
DIR_BIN := ./bin
DIR_LIB := ./lib
INCLUDE := -I$(DIR_INC)
LIBS := -lopencv_highgui -lopencv_core -lopencv_imgproc
BLAS_LIB := /usr/lib64/atlas/libatlas.a /usr/lib64/atlas/libcblas.a 

SOCROP := lib/libcactus_intf.so # DO NOT CHANGE THR LIB NAME
COBJS := $(DIR_OBJ)/cactus_intf.o $(DIR_OBJ)/ConvLayer.o $(DIR_OBJ)/CppConvNet.o $(DIR_OBJ)/Layers.o $(DIR_OBJ)/NormalizationLayer.o $(DIR_OBJ)/PoolLayer.o $(DIR_OBJ)/ReluLayer.o
DEF := -DLINUX -DCPU_ONLY #-DR_DEBUG
SRC := $(wildcard ${DIR_SRC}/*.cpp)

all: $(SOCROP)

# $(BLAS_LIB)
#-static -lcblas -Wl,-Bdynamic

$(SOCROP):$(COBJS)
	$(GXX) -shared -o $@ -fPIC $(CXXFLAG) $(LDFLAG) $(COBJS) $(LIBS) -Wl,--whole-archive $(BLAS_LIB) -Wl,--no-whole-archive

$(DIR_OBJ)/%.o:$(DIR_SRC)/%.cpp
	$(GXX) -fPIC -o $@ -c $< $(CXXFLAG) $(INCLUDE) $(DEF)

clean:
	rm -fr $(COBJS) $(SOCROP);

