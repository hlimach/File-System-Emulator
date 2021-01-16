CXX=g++
CXXFLAGS=-std=c++17 -Wall -I.

SRC_DIR = .

SRC := $(addprefix $(SRC_DIR)/, client.cpp)

client: $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o client