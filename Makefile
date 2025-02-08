CXX = g++
CXXFLAGS = -g -Wall -Wextra -std=c++17 -O2

# Source files
SRC = $(filter-out visualiser.cpp, $(wildcard *.cpp))
OBJ = $(SRC:.cpp=.o)

# Output executable
TARGET = sim

# Default rule
all: $(TARGET)

# Linking step
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation step for each .cpp file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)