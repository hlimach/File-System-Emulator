CXX      := g++
CXXFLAGS := -std=c++17 -Wall -lpthread
OBJ_DIR  := ./obj
SRC_DIR  := ./src
APP_DIR  := .
TARGET   := server
INCLUDE  := -Iinclude/
SRC      := $(wildcard src/*.cpp)
OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)
#OBJECTS  := $(notdir $(SRC:/%.cpp=$(OBJ_DIR)/%.o))

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^

-include $(DEPENDENCIES)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -f $(APP_DIR)/$(TARGET)