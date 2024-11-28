# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -pthread
LIB_NAME = libmemory_manager.so

# Source and Object Files
MEM_MANAGER_SRC = memory_manager.c
MEM_MANAGER_OBJ = $(MEM_MANAGER_SRC:.c=.o)
LINKED_LIST_SRC = linked_list.c
TEST_LINKED_LIST_SRC = test_linked_list.c
LINKED_LIST_OBJ = $(LINKED_LIST_SRC:.c=.o)

# Default target: builds both the dynamic library and the linked list application
all: gitinfo mmanager list

# Rule to create the dynamic library
$(LIB_NAME): $(MEM_MANAGER_OBJ)
	$(CC) -shared -o $@ $(MEM_MANAGER_OBJ)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

gitinfo:
	@echo "const char *git_date = \"$(GIT_DATE)\";" > gitdata.h
	@echo "const char *git_sha = \"$(GIT_COMMIT)\";" >> gitdata.h

# Build the memory manager
mmanager: $(MEM_MANAGER_OBJ)
	$(CC) -shared -o $(LIB_NAME) $(MEM_MANAGER_OBJ)

# Build the linked list application and link it with the memory manager library
list: $(TEST_LINKED_LIST_SRC) $(LINKED_LIST_SRC) $(LIB_NAME)
	$(CC) $(CFLAGS) -o test_linked_list $(TEST_LINKED_LIST_SRC) $(LINKED_LIST_SRC) -L. -lmemory_manager -I. -Wl,-rpath,. -lm

# Test target to run the memory manager test program
test_mmanager: $(LIB_NAME)
	$(CC) $(CFLAGS) -o test_memory_manager test_memory_manager.c -L. -lmemory_manager -lm

# Test target to run the linked list test program
test_list: $(LIB_NAME) $(LINKED_LIST_OBJ)
	$(CC) $(CFLAGS) -o test_linked_list $(LINKED_LIST_SRC) $(TEST_LINKED_LIST_SRC) -L. -lmemory_manager -lm

# Run tests
run_tests: run_test_mmanager run_test_list

# Run test cases for the memory manager
run_test_mmanager: test_mmanager
	LD_LIBRARY_PATH=. ./test_memory_manager 0

# Run test cases for the linked list
run_test_list: test_list
	LD_LIBRARY_PATH=. ./test_linked_list 0

# Clean target to clean up build files
clean:
	rm -f $(MEM_MANAGER_OBJ) $(LINKED_LIST_OBJ) $(LIB_NAME) test_memory_manager test_linked_list