CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
INCLUDES = -I include/core -I include/algorithm

TARGET = gsnsga2
SRC    = main.cpp

HEADERS = include/core/problem.h include/core/matching.h include/core/individual.h \
          include/core/neighborhood.h include/core/local_search.h include/core/gs_strategy.h \
          include/core/nsga2.h include/core/crossover.h include/core/mutation.h \
          include/algorithm/gsnsga2.h

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) data/small1.txt 60 100

run20: $(TARGET)
	./$(TARGET) data/small1.txt 60 100 --runs 20

.PHONY: all clean run run20
