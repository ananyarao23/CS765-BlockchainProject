CXX = g++ -w
CXXFLAGS = -g -Wall -Wextra -std=c++17 -O2
LDFLAGS = -lssl -lcrypto  # Add OpenSSL libraries

# Source files
SRC = $(filter-out visualiser.cpp, $(wildcard *.cpp))
OBJ = $(SRC:.cpp=.o)

# Output executable
TARGET = sim

# Default rule
all: $(TARGET)

# Linking step
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation step for each .cpp file
%.o: %.cpp
	$(CXX) -O0 $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)
