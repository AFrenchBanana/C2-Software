# Variables
CC = gcc
CFLAGS = 
LDFLAGS = -lssl -lcrypto

# Source and object files
SRC_DIR = Linux Generic
SRC = main.c $(wildcard $(addsuffix /*.c, $(SRC_DIR)))
OBJ = $(SRC:.c=.o)

# Target 
TARGET = client.elf

# Rules
all: $(TARGET) # Name of the target

# Linking
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) 

# Object files compilation rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Removes only object files
cleanobj:
	rm -f $(OBJ)

# Removes object files and the target
clean: cleanobj
	rm -f $(TARGET)

# Phony targets indicate that the target is not a file
.PHONY: all clean cleanobj
