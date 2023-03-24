# Compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread -lm -g

# Source files
SRCS = main.c sharedRegion.c

# Object files
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = bSort

# Build rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Object file rules
main.o: main.c sharedRegion.h
	$(CC) $(CFLAGS) -c main.c -o main.o -g

sharedRegion.o: sharedRegion.c sharedRegion.h
	$(CC) $(CFLAGS) -c sharedRegion.c -o sharedRegion.o -g

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
