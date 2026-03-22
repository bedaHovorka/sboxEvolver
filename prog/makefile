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
FLAGS= -W -Wall -std=gnu++11 $(CURRENT_FLAGS) -fopenmp -march=k8-sse3 -O3 -I../galib247/ -L../galib247/ga -I/usr/local/include/python3.14
CXX_LIBS=-lga -lm

.SUFFIXES: .cpp
.cpp.o: ${HEADS}
	$(CXX) -fPIC $(FLAGS) $(INC_DIRS) -c $<

all: $(EXS)

# Use this for gnu make
$(EXS): $(OBJS)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $(LIB_DIRS) $(OBJS) $(FLAGS) $(CXX_LIBS)

VALGRIND_TEST=valgrind_test
VALGRIND_TEST_SRC=src/test/cpp/valgrind_test.cpp

$(VALGRIND_TEST): $(EXS) $(VALGRIND_TEST_SRC)
	$(CXX) $(FLAGS) $(INC_DIRS) -o $@ $(VALGRIND_TEST_SRC) -L. -lsboxevolution $(CXX_LIBS)

valgrind: $(VALGRIND_TEST)
	LD_LIBRARY_PATH=. valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 ./$(VALGRIND_TEST)

# Catch2 unit tests
TEST_DIR=src/test/cpp
TEST_MAIN_OBJ=$(TEST_DIR)/test_main.o
TEST_SRCS=$(filter-out $(TEST_DIR)/test_main.cpp $(VALGRIND_TEST_SRC) $(wildcard $(TEST_DIR)/bench_*.cpp), $(wildcard $(TEST_DIR)/test_*.cpp))
TEST_OBJS=$(TEST_SRCS:.cpp=.o)
TEST_BIN=run_tests

# Catch2 benchmarks
BENCH_MAIN_OBJ=$(TEST_DIR)/bench_main.o
BENCH_SRCS=$(filter-out $(TEST_DIR)/bench_main.cpp, $(wildcard $(TEST_DIR)/bench_*.cpp))
BENCH_OBJS=$(BENCH_SRCS:.cpp=.o)
BENCH_BIN=run_benchmarks

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) -fPIC $(FLAGS) -I$(TEST_DIR) -Isrc/main/cpp $(INC_DIRS) -c $< -o $@

$(TEST_BIN): $(EXS) $(TEST_MAIN_OBJ) $(TEST_OBJS)
	$(CXX) $(FLAGS) -I$(TEST_DIR) -Isrc/main/cpp $(INC_DIRS) -o $@ $(TEST_MAIN_OBJ) $(TEST_OBJS) -L. -lsboxevolution $(CXX_LIBS)

test: $(TEST_BIN)
	LD_LIBRARY_PATH=. ./$(TEST_BIN)

$(BENCH_BIN): $(EXS) $(BENCH_MAIN_OBJ) $(BENCH_OBJS)
	$(CXX) $(FLAGS) -I$(TEST_DIR) -Isrc/main/cpp $(INC_DIRS) -o $@ $(BENCH_MAIN_OBJ) $(BENCH_OBJS) -L. -lsboxevolution $(CXX_LIBS)

bench: $(BENCH_BIN)
	LD_LIBRARY_PATH=. ./$(BENCH_BIN)

clean:
	$(RM) $(OBJS) $(EXS) $(VALGRIND_TEST) $(TEST_BIN) $(BENCH_BIN) $(TEST_DIR)/*.o *.o *~ *.pyc core
