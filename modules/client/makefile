CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Ofast
SRC_DIR  := .
TARGET   := client
SRC      := $(addprefix $(SRC_DIR)/, client.cpp)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "\nSuccessful! To run client, type ./client"

clean:
	rm -f $(TARGET)