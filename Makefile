CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -O2 -Isrc
FLEX := flex
BISON := bison

BIN_DIR := bin
BUILD_DIR := build
SRC_DIR := src

TARGET := $(BIN_DIR)/cyberhand_compiler

# Lista de fuentes (se irá poblando conforme agreguemos módulos)
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/modules/*/*.cpp)

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean test dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BIN_DIR) $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/core $(BUILD_DIR)/grammar
	@mkdir -p $(BUILD_DIR)/modules/ll1_analyzer
	@mkdir -p $(BUILD_DIR)/modules/lr0_analyzer
	@mkdir -p $(BUILD_DIR)/modules/semantics
	@mkdir -p $(BUILD_DIR)/modules/intermediate
	@mkdir -p $(BUILD_DIR)/modules/optimizer

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	@if [ -n "$(OBJS)" ]; then \
		$(CXX) $(CXXFLAGS) $(OBJS) -o $@; \
		echo "Compilado exitosamente: $@"; \
	else \
		echo "Estructura lista. Añada archivos .cpp para enlazar $(TARGET)"; \
	fi

test:
	@echo "Ejecutando pruebas de módulos..."
	@if [ -f $(BIN_DIR)/test_runner ]; then $(BIN_DIR)/test_runner; fi

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
