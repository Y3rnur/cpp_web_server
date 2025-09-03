# Define the C++ and C compilers
CXX = g++
CC = gcc
# Define compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -g
CFLAGS = -Wall -Wextra -g
LDFLAGS = -pthread

# Define the name of your executable
TARGET = server

# List all your source files
SRCS_CPP = starter_server.cpp
SRCS_C = sqlite3/sqlite3.c

# Automatically generate a list of object files
OBJS_CPP = $(SRCS_CPP:.cpp=.o)
OBJS_C = $(SRCS_C:.c=.o)
OBJS = $(OBJS_CPP) $(OBJS_C)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Rule to compile C++ source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile C source files with the C compiler
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)