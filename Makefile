# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

# Source files
SRC = blocks.cpp simulator.cpp transactions.cpp
HEADERS = helper.h random_graph.h simulator.h
OBJ = $(SRC:.cpp=.o)

# Executable name
TARGET = sim

# Default rule: build the executable
all: $(TARGET)

# Linking step
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each source file into an object file
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJ) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug mode
debug: CXXFLAGS += -g
debug: clean all
