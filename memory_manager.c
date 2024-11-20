#include "memory_manager.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

uintptr_t calculate_distance(void* ptr1, void* ptr2) {
    uintptr_t address1 = (uintptr_t)ptr1;
    uintptr_t address2 = (uintptr_t)ptr2;
    return (address1 > address2) ? (address1 - address2) : (address2 - address1);
}

void* read_pointer() {
    void* ptr;
    printf("Enter pointer address (in hexadecimal, e.g., 0x7ffee4b8c8c0): ");
    scanf("%p", &ptr);
    return ptr;
}

void test_calculate_distance() {
    void* ptr1 = read_pointer();
    void* ptr2 = read_pointer();

    uintptr_t distance = calculate_distance(ptr1, ptr2);
    printf("Distance between %p and %p is %lu bytes\n", ptr1, ptr2, distance);
}

typedef struct Block {
    size_t size;
    bool free;
    void* memory;
    struct Block *next;
} Block;

Block* block_array = NULL;
size_t metadata_count = 0;
size_t memory_pool_size = 0;
void* memory_pool = NULL;

pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;

void mem_init(size_t size) {
    pthread_mutex_lock(&memory_mutex);
    memory_pool = malloc(size);
    if (memory_pool == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    memory_pool_size = size;

    // Initialize the metadata array with a single large block
    block_array = malloc(sizeof(Block));
    if (!block_array) {
        printf("Failed to allocate metadata array\n");
        exit(1);
    }

    block_array->size = size;
    block_array->free = true;
    block_array->memory = memory_pool;
    block_array->next = NULL;
    pthread_mutex_unlock(&memory_mutex);
}

void* mem_alloc(size_t size) {
    pthread_mutex_lock(&memory_mutex);
    Block* current = block_array;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Split the block if it's larger than needed
            if (current->size > size) {
                Block* new_block = malloc(sizeof(Block));
                if (!new_block) {
                    printf("Failed to allocate new block metadata\n");
                    pthread_mutex_unlock(&memory_mutex);
                    exit(1);
                }
                new_block->size = current->size - size;
                new_block->free = true;
                new_block->memory = (void*)((uintptr_t)current->memory + size);
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }
            current->free = false;
            void* allocated_memory = current->memory;
            memset(allocated_memory, 0, size); // Initialize allocated memory to zero
            pthread_mutex_unlock(&memory_mutex);
            return allocated_memory;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL; // No suitable block found
}

void mem_free(void* block) {
    pthread_mutex_lock(&memory_mutex);
    Block* current = block_array;
    Block* prev = NULL;
    while (current != NULL) {
        if (current->memory == block) {
            current->free = true;

            // Coalesce with next free blocks
            while (current->next != NULL && current->next->free) {
                Block* temp = current->next;
                current->size += temp->size;
                current->next = temp->next;
                free(temp);
            }

            // Coalesce with previous free blocks
            if (prev != NULL && prev->free) {
                prev->size += current->size;
                prev->next = current->next;
                free(current);
            }

            pthread_mutex_unlock(&memory_mutex);
            return;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&memory_mutex);
}

void* mem_resize(void* block, size_t size) {
    pthread_mutex_lock(&memory_mutex);
    Block* current = block_array;
    while (current != NULL) {
        if (current->memory == block) {
            if (current->size == size) {
                pthread_mutex_unlock(&memory_mutex);
                return block;
            } else if (current->size > size) {
                // Shrink the block
                Block* new_block = malloc(sizeof(Block));
                if (!new_block) {
                    printf("Failed to allocate new block metadata\n");
                    pthread_mutex_unlock(&memory_mutex);
                    exit(1);
                }
                new_block->size = current->size - size;
                new_block->free = true;
                new_block->memory = (void*)((uintptr_t)current->memory + size);
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;

                pthread_mutex_unlock(&memory_mutex);
                return current->memory;
            } else {
                // Check if next block is free and adjacent
                if (current->next != NULL && current->next->free &&
                    (uintptr_t)current->memory + current->size == (uintptr_t)current->next->memory &&
                    current->size + current->next->size >= size) {
                    // Merge with next block
                    Block* next_block = current->next;
                    current->size += next_block->size;
                    current->next = next_block->next;
                    free(next_block);

                    // Attempt to resize again
                    if (current->size >= size) {
                        // Split if larger than needed
                        if (current->size > size) {
                            Block* new_block = malloc(sizeof(Block));
                            if (!new_block) {
                                printf("Failed to allocate new block metadata\n");
                                pthread_mutex_unlock(&memory_mutex);
                                exit(1);
                            }
                            new_block->size = current->size - size;
                            new_block->free = true;
                            new_block->memory = (void*)((uintptr_t)current->memory + size);
                            new_block->next = current->next;

                            current->size = size;
                            current->next = new_block;
                        }
                        pthread_mutex_unlock(&memory_mutex);
                        return current->memory;
                    }
                }
                pthread_mutex_unlock(&memory_mutex);
                // Allocate a new block
                void* new_block_memory = mem_alloc(size);
                if (new_block_memory) {
                    memcpy(new_block_memory, block, current->size);
                    mem_free(block);
                }
                return new_block_memory;
            }
        }
        current = current->next;
    }
    pthread_mutex_unlock(&memory_mutex);
    return NULL; // Block not found
}

void mem_deinit() {
    pthread_mutex_lock(&memory_mutex);
    free(memory_pool);
    memory_pool = NULL;
    memory_pool_size = 0;

    free(block_array);
    block_array = NULL;
    metadata_count = 0;
    pthread_mutex_unlock(&memory_mutex);
}

void print_blocks_ADMIN() {
    pthread_mutex_lock(&memory_mutex);
    Block* current = block_array;
    while (current != NULL) {
        printf("Block at %p: size = %zu, free = %s, Memory at = %p\n",
               (void*)current, current->size, current->free ? "true" : "false",
               current->memory);
        current = current->next;
    }
    pthread_mutex_unlock(&memory_mutex);
}

void print_blocks_USR() {
    pthread_mutex_lock(&memory_mutex);
    Block* current = block_array;
    while (current != NULL) {
        printf("Block at %p: size = %zu, free = %s\n",
               current->memory, current->size, current->free ? "true" : "false");
        current = current->next;
    }
    pthread_mutex_unlock(&memory_mutex);
}

int main() {

    printf("Memory Manager\n");
    printf("How much memory do you want to allocate? ");
    size_t size;
    scanf("%lu", &size);
    mem_init(size);
    printf("Memory pool is allocated\n");
    printf("Memory pool address: %p\n", memory_pool);
    print_blocks_USR();

    while (true)
    {
        printf("1. Allocate memory\n");
        printf("2. Free memory\n");
        printf("3. Resize memory\n");
        printf("4. Print memory blocks\n");
        printf("5. Exit\n");
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Enter the size of memory to allocate: ");
            size_t alloc_size;
            scanf("%lu", &alloc_size);
            void* block = mem_alloc(alloc_size);
            if (block == NULL) {
                printf("Memory allocation failed\n");
            } else {
                printf("Memory allocated at %p\n", block);
            }
            print_blocks_USR();
            break;
        case 2:
            printf("Enter the address of memory to free: ");
            void* free_block;
            scanf("%p", &free_block);
            printf("Freeing memory at %p\n", free_block);
            mem_free(free_block);
            print_blocks_USR();
            break;
        case 3:
            printf("Enter the address of memory to resize: ");
            void* resize_block;
            scanf("%p", &resize_block);
            printf("Enter the new size of memory: ");
            size_t new_size;
            scanf("%lu", &new_size);
            void* new_block = mem_resize(resize_block, new_size);
            if (new_block == NULL) {
                printf("Memory resize failed\n");
            } else {
                printf("Memory resized at %p\n", new_block);
            }
            print_blocks_USR();
            break;
        case 4:
            print_blocks_ADMIN();
            break;
        case 5:
            mem_deinit();
            return 0;
        case 6:
            test_calculate_distance();
            break;
        default:
            printf("Invalid choice\n");
            break;
        
        }
    }
    mem_deinit();
    return 0;
}