#
#      Brno University of Technology
#      Faculty of Information Technology
#
#      MSc Thesis       2009/2010
#
#      Design of S-Boxes Using Genetic Algorithms
#
#      Created on Feb 18, 2010
#
#      @author: Bedrich Hovorka
#

# na merlinovi je treba CXX nastavit na g++-4.4 a (march nechat...) -I../galib247/ -L../galib247/ga
#INC_DIRS= -I$(GA_INC_DIR)
#LIB_DIRS= -L$(GA_LIB_DIR)

CXX = g++-4.4
VPATH = src/main/cpp

EXS=libsboxevolution.so
OBJS= eda.o cgp.o softwareSbox.o criterions.o multicriterial.o main.o
HEADS=common.h eda.h cgp.h criterions.h main.h softwareSbox.h multicriterial.h
DEBUG_FLAGS = -ggdb3
PROD_FLAGS = -DNDEBUG
CURRENT_FLAGS = $(DEBUG_FLAGS)
FLAGS= -W -Wall -std=gnu++11 $(CURRENT_FLAGS) -fopenmp -march=k8-sse3 -O3 -I../galib247/ -L../galib247/ga -I/usr/local/include/python2.5
CXX_LIBS=-lga -lm

.SUFFIXES: .cpp
.cpp.o: ${HEADS}
	$(CXX) -fPIC $(FLAGS) $(INC_DIRS) -c $<

all: $(EXS)

# Use this for gnu make
$(EXS): $(OBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $(LIB_DIRS) $(OBJS) $(FLAGS) $(CXX_LIBS)

clean:
	$(RM) $(OBJS) $(EXS) *.o *~ *.pyc core
