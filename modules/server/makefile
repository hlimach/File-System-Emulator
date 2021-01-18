CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Ofast -pthread
OBJ_DIR  := ./obj
SRC_DIR  := ./src
APP_DIR  := .
TARGET   := server
INC      := -Iinclude/
SRC      := $(wildcard src/*.cpp)
OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
OBJS     := $(addprefix $(OBJ_DIR)/,$(notdir $(OBJECTS)))

# Makes object files for corresponding .cpp file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# Makes target using object files
$(APP_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "\nSuccessful! To run server, type ./server"

# Makes object directory if it doesn't exist
$(OBJ_DIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -f $(APP_DIR)/$(TARGET)