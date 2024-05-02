# Define the compiler
CC = gcc

# Define any compile-time flags
CFLAGS = -Wall -g

# Define the name of the executable output
TARGET = a.out

# Default target
all: $(TARGET)

# Rule for building the target
$(TARGET): ttsh.c
	$(CC) $(CFLAGS) -o $(TARGET) ttsh.c

# Rule for cleaning up the directory
clean:
	$(RM) $(TARGET)