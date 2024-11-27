# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -pthread
LIB_NAME = libmemory_manager.so

# Source and Object Files
SRC = memory_manager.c linked_list.c
OBJ = $(SRC:.c=.o)

# Default target: build both the dynamic library and the linked list application
all: mmanager list

# Rule to create the dynamic library
$(LIB_NAME): memory_manager.o
    $(CC) -shared -o $@ $<

# Rule to compile source files into object files
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager as a dynamic library
mmanager: $(LIB_NAME)

# Build the linked list application and link it with the memory manager library
list: linked_list.o
    $(CC) $(CFLAGS) -o linked_list linked_list.o -L. -lmemory_manager

# Test target to build the memory manager test program
test_mmanager: $(LIB_NAME) test_memory_manager.o
    $(CC) $(CFLAGS) -o test_memory_manager test_memory_manager.o -L. -lmemory_manager

# Test target to build the linked list test program
test_list: $(LIB_NAME) linked_list.o test_linked_list.o
    $(CC) $(CFLAGS) -o test_linked_list linked_list.o test_linked_list.o -L. -lmemory_manager

# Run all tests
run_tests: run_test_mmanager run_test_list

# Run test cases for the memory manager
run_test_mmanager: test_mmanager
    ./test_memory_manager

# Run test cases for the linked list
run_test_list: test_list
    ./test_linked_list

# Clean target to clean up build files
clean:
    rm -f $(OBJ) $(LIB_NAME) linked_list test_memory_manager test_linked_list