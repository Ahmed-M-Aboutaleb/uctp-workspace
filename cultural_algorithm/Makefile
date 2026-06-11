# Makefile for the Cultural Algorithm UCTP solver
#
#   make            - build the binary `cultural_ca`
#   make run        - build and run on the bundled `medium3` dataset (360 s)
#   make quick      - build and run for 30 s for a quick smoke test
#   make clean      - remove objects and the binary
#

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic -MMD -MP
LDFLAGS  :=

SRC_DIR := src
OBJ_DIR := build
BIN     := cultural_ca

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS    := $(OBJECTS:.o=.d)

DATASET ?= ../attached_assets/medium3_1777391097129.txt
TIME    ?= 360

.PHONY: all run quick clean

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(BIN)
	./$(BIN) $(DATASET) -t $(TIME)

quick: $(BIN)
	./$(BIN) $(DATASET) -t 30

clean:
	rm -rf $(OBJ_DIR) $(BIN) solution.txt

-include $(DEPS)
