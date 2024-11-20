#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h> // For size_t
#include <stdint.h> // For uintptr_t
#include <pthread.h> // For pthread_mutex_t
#include <math.h> // For pow



void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t size);
void mem_deinit(void);
void print_blocks_ADMIN(void);
void print_blocks_USR(void);
uintptr_t calculate_distance(void* ptr1, void* ptr2);
void* read_pointer(void);
void test_calculate_distance(void);



#endif // MEMORY_MANAGER_H