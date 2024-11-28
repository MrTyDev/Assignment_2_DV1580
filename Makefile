# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -pthread
LIB_NAME = libmemory_manager.so

# Source and Object Files
MEM_MANAGER_SRC = memory_manager.c
MEM_MANAGER_OBJ = $(MEM_MANAGER_SRC:.c=.o)
LINKED_LIST_SRC = linked_list.c
LINKED_LIST_OBJ = $(LINKED_LIST_SRC:.c=.o)
TEST_MEM_MANAGER_SRC = test_memory_manager.c
TEST_MEM_MANAGER_OBJ = $(TEST_MEM_MANAGER_SRC:.c=.o)
TEST_LINKED_LIST_SRC = test_linked_list.c
TEST_LINKED_LIST_OBJ = $(TEST_LINKED_LIST_SRC:.c=.o)

# Targets
all: mmanager list test_linked_list test_memory_manager

mmanager: $(MEM_MANAGER_OBJ)
	gcc -o $(LIB_NAME) $(MEM_MANAGER_OBJ) $(CFLAGS) -shared

list: $(LINKED_LIST_OBJ)
	gcc -o liblinked_list.so $(LINKED_LIST_OBJ) $(CFLAGS) -shared -lm

run_test_mmanager: $(TEST_MEM_MANAGER_OBJ) $(MEM_MANAGER_OBJ)
	gcc -o test_memory_manager $(TEST_MEM_MANAGER_OBJ) $(MEM_MANAGER_OBJ) $(CFLAGS) -lm && taskset -c 0-$(shell expr $(shell nproc) - 1) ./test_memory_manager 0

run_test_list: $(TEST_LINKED_LIST_OBJ) $(LINKED_LIST_OBJ) $(MEM_MANAGER_OBJ)
	gcc -o test_linked_list $(TEST_LINKED_LIST_OBJ) $(LINKED_LIST_OBJ) $(MEM_MANAGER_OBJ) $(CFLAGS) -lm && taskset -c 0-$(shell expr $(shell nproc) - 1) ./test_linked_list 0

test_memory_manager: $(TEST_MEM_MANAGER_OBJ) $(MEM_MANAGER_OBJ)
	gcc -o test_memory_manager $(TEST_MEM_MANAGER_OBJ) $(MEM_MANAGER_OBJ) $(CFLAGS) -lm
	taskset -c 0-$(shell expr $(shell nproc) - 1) ./test_memory_manager 0

test_linked_list: $(TEST_LINKED_LIST_OBJ) $(LINKED_LIST_OBJ) $(MEM_MANAGER_OBJ)
	gcc -o test_linked_list $(TEST_LINKED_LIST_OBJ) $(LINKED_LIST_OBJ) $(MEM_MANAGER_OBJ) $(CFLAGS) -lm
	taskset -c 0-$(shell expr $(shell nproc) - 1) ./test_linked_list 0

clean:
	rm -f *.o *.so test_memory_manager test_linked_list