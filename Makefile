
## You may need to edit this path to gecode
GECODE_HOME=${HOME}/opt/gecode/release-4.3.1
#UNAME_S := $(shell uname -s)
#ifeq ($(UNAME_S),Linux)
#	GECODE_HOME=${HOME}/opt/gecode/release
#else
#	ifeq ($(UNAME_S),Darwin)
#		GECODE_HOME=/opt/gecode/release
#  	endif
#endif

PROG=xyz

GECODE_LINKS=-lgecodegist -lgecodeint -lgecodesearch -lgecodekernel -lgecodesupport -lgecodedriver -lgecodeminimodel -lgecodeset 

LINKER=g++
COMPILER=g++

COMPOPTS += -std=c++11 -g
LINKOPTS+=
#LINKOPTS+= 

SOURCE_FILES = tinyxml2.cpp Data.cpp main.cpp
OBJECT_FILES = tinyxml2.o Data.o main.o
HEADER_FILES = tinyxml2.h Data.h


xyz: $(OBJECT_FILES)
	$(LINKER) $(COMPOPTS) $(LINKOPTS) -o ${PROG} -L${GECODE_HOME} $(OBJECT_FILES) ${GECODE_LINKS}

%.o: %.cpp 
	$(COMPILER) $(COMPOPTS) -I${GECODE_HOME} -Wfatal-errors -c $*.cpp


clean:
	rm -fr ${PROG} *.o

#g++ -g -O0 -fno-inline program.cpp