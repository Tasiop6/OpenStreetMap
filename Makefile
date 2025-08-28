# Makefile for HW5 OpenStreetMap Graph Project

CXX       = g++
CXXFLAGS  = -std=c++17 -O2 -Wall -Wextra -fsanitize=address

SRCS      = HW5.cpp Graph.cpp Vertex.cpp Edge.cpp tinyxml2.cpp
OBJS      = $(SRCS:.cpp=.o)
TARGET    = hw5

all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile rule for .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean