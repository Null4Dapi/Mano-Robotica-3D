CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -O2 -Isrc -Ibuild/grammar
FLEX := flex
BISON := bison

BIN_DIR := bin
BUILD_DIR := build
SRC_DIR := src

TARGET := $(BIN_DIR)/cyberhand_compiler

# Grammar generated files
PARSER_TAB_CPP := $(BUILD_DIR)/grammar/parser.tab.cpp
PARSER_TAB_HPP := $(BUILD_DIR)/grammar/parser.tab.hpp
LEX_YY_CPP     := $(BUILD_DIR)/grammar/lex.yy.cpp

GRAMMAR_GEN_OBJS := $(BUILD_DIR)/grammar/parser.tab.o $(BUILD_DIR)/grammar/lex.yy.o

# C++ Source files
MODULE_SRCS := $(wildcard $(SRC_DIR)/modules/*/*.cpp)
DRIVER_SRC  := $(SRC_DIR)/grammar/ParserDriver.cpp
MAIN_SRC    := $(SRC_DIR)/main.cpp

ALL_SRCS := $(MODULE_SRCS) $(DRIVER_SRC) $(MAIN_SRC)
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(ALL_SRCS))

ALL_OBJS := $(CPP_OBJS) $(GRAMMAR_GEN_OBJS)

.PHONY: all clean dirs grammar test

all: dirs grammar $(TARGET)

dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)/core $(BUILD_DIR)/grammar
	@mkdir -p $(BUILD_DIR)/modules/ll1_analyzer
	@mkdir -p $(BUILD_DIR)/modules/lr0_analyzer
	@mkdir -p $(BUILD_DIR)/modules/semantics
	@mkdir -p $(BUILD_DIR)/modules/intermediate
	@mkdir -p $(BUILD_DIR)/modules/optimizer

grammar: $(PARSER_TAB_HPP) $(PARSER_TAB_CPP) $(LEX_YY_CPP)

$(PARSER_TAB_CPP) $(PARSER_TAB_HPP): $(SRC_DIR)/grammar/parser.y | dirs
	$(BISON) -d -o $(PARSER_TAB_CPP) $<

$(LEX_YY_CPP): $(SRC_DIR)/grammar/scanner.l $(PARSER_TAB_HPP) | dirs
	$(FLEX) -o $@ $<

$(BUILD_DIR)/grammar/parser.tab.o: $(PARSER_TAB_CPP)
	$(CXX) -std=c++20 -O2 -Isrc -Ibuild/grammar -c $< -o $@

$(BUILD_DIR)/grammar/lex.yy.o: $(LEX_YY_CPP)
	$(CXX) -std=c++20 -O2 -Isrc -Ibuild/grammar -c $< -o $@

$(BUILD_DIR)/grammar/ParserDriver.o: $(SRC_DIR)/grammar/ParserDriver.cpp $(PARSER_TAB_HPP) | dirs
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | dirs $(PARSER_TAB_HPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(ALL_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $@
	@echo "Compilado exitosamente: $@"

test: all
	@echo "Ejecutando suite de pruebas unitarias y E2E..."
	@if [ -f tests/test_cli_e2e.sh ]; then bash tests/test_cli_e2e.sh; fi

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
