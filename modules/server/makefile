CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Ofast -pthread
SRC_DIR  := ./src
APP_DIR  := .
TARGET   := server
INC      := -Iinclude/
SRC      := $(wildcard src/*.cpp)
# OBJ_DIR  := ./obj
# OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
# OBJS     := $(addprefix $(OBJ_DIR)/,$(notdir $(OBJECTS)))

# Makes object files for corresponding .cpp file
# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
# 	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# # Makes target using object files
# $(APP_DIR)/$(TARGET): $(OBJS)
# 	$(CXX) $(CXXFLAGS) -o $@ $^
# 	@echo "\nSuccessful! To run server, type ./server"

# # Makes object directory if it doesn't exist
# $(OBJ_DIR):
# 	mkdir $(OBJDIR)

# Directly make target
$(APP_DIR)/$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ $^
	@echo "\nSuccessful! To run server, type ./server"

clean:
	@rm -f $(APP_DIR)/$(TARGET)
	@echo "\nClean up successful."

#rm -rf $(OBJ_DIR)/*