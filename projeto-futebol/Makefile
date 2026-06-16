CXX      = g++
CXXFLAGS = -Wall -O2 -std=c++17 -I./include -MMD -MP
LDFLAGS  = -lGL -lGLU -lglut -lm -lpthread -ldl

SRC_DIR  = src
OBJ_DIR  = obj
TARGET   = futebol_cg

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: all
	./$(TARGET)

# Recompila automaticamente quando um header (.h) incluído muda
-include $(DEPS)

.PHONY: all clean run
