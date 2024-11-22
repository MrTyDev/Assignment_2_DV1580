#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "memory_manager.h"
#include "linked_list.h"

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

void list_init(Node** head, size_t size) {
    mem_init(size);
    *head = NULL;
    // No need to initialize global_lock again since it's statically initialized
}

void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&new_node->lock, NULL);
    new_node->data = data;
    new_node->next = NULL;

    pthread_mutex_lock(&global_lock);
    if (*head == NULL) {
        *head = new_node;
        pthread_mutex_unlock(&global_lock);
    } else {
        Node* temp = *head;
        pthread_mutex_lock(&temp->lock);
        pthread_mutex_unlock(&global_lock);

        while (temp->next != NULL) {
            Node* next = temp->next;
            pthread_mutex_lock(&next->lock);
            pthread_mutex_unlock(&temp->lock);
            temp = next;
        }
        temp->next = new_node;
        pthread_mutex_unlock(&temp->lock);
    }
}

void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        fprintf(stderr, "Previous node cannot be NULL\n");
        return;
    }

    pthread_mutex_lock(&prev_node->lock);
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        pthread_mutex_unlock(&prev_node->lock);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&new_node->lock, NULL);
    new_node->data = data;
    new_node->next = prev_node->next;

    prev_node->next = new_node;
    pthread_mutex_unlock(&prev_node->lock);
}

void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        fprintf(stderr, "Next node cannot be NULL\n");
        return;
    }

    pthread_mutex_lock(&global_lock);
    if (*head == next_node) {
        Node* new_node = (Node*)mem_alloc(sizeof(Node));
        if (new_node == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            pthread_mutex_unlock(&global_lock);
            exit(EXIT_FAILURE);
        }
        pthread_mutex_init(&new_node->lock, NULL);
        new_node->data = data;
        new_node->next = *head;
        *head = new_node;
        pthread_mutex_unlock(&global_lock);
        return;
    }

    Node* temp = *head;
    pthread_mutex_lock(&temp->lock);
    pthread_mutex_unlock(&global_lock);

    while (temp->next != NULL && temp->next != next_node) {
        Node* next = temp->next;
        pthread_mutex_lock(&next->lock);
        pthread_mutex_unlock(&temp->lock);
        temp = next;
    }

    if (temp->next == NULL) {
        fprintf(stderr, "Next node not found in the list\n");
        pthread_mutex_unlock(&temp->lock);
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        pthread_mutex_unlock(&temp->lock);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&new_node->lock, NULL);
    new_node->data = data;
    new_node->next = next_node;

    temp->next = new_node;
    pthread_mutex_unlock(&temp->lock);
}

void list_delete(Node** head, uint16_t data) {
    pthread_mutex_lock(&global_lock);
    if (*head == NULL) {
        pthread_mutex_unlock(&global_lock);
        return;
    }

    Node* temp = *head;
    pthread_mutex_lock(&temp->lock);
    pthread_mutex_unlock(&global_lock);

    if (temp->data == data) {
        *head = temp->next;
        pthread_mutex_unlock(&temp->lock);
        pthread_mutex_destroy(&temp->lock);
        mem_free(temp);
        return;
    }

    Node* prev = temp;
    temp = temp->next;

    while (temp != NULL) {
        pthread_mutex_lock(&temp->lock);
        if (temp->data == data) {
            prev->next = temp->next;
            pthread_mutex_unlock(&temp->lock);
            pthread_mutex_destroy(&temp->lock);
            mem_free(temp);
            pthread_mutex_unlock(&prev->lock);
            return;
        }
        pthread_mutex_unlock(&prev->lock);
        prev = temp;
        temp = temp->next;
    }
    pthread_mutex_unlock(&prev->lock);
}

Node* list_search(Node** head, uint16_t data) {
    pthread_mutex_lock(&global_lock);
    if (*head == NULL) {
        pthread_mutex_unlock(&global_lock);
        return NULL;
    }

    Node* temp = *head;
    pthread_mutex_lock(&temp->lock);
    pthread_mutex_unlock(&global_lock);

    while (temp != NULL) {
        if (temp->data == data) {
            pthread_mutex_unlock(&temp->lock);
            return temp;
        }
        Node* next = temp->next;
        if (next != NULL) {
            pthread_mutex_lock(&next->lock);
        }
        pthread_mutex_unlock(&temp->lock);
        temp = next;
    }
    return NULL;
}

void list_display(Node** head) {
    pthread_mutex_lock(&global_lock);
    if (head == NULL || *head == NULL) {
        printf("[]\n");
        pthread_mutex_unlock(&global_lock);
        return;
    }

    Node* current = *head;
    pthread_mutex_lock(&current->lock);
    pthread_mutex_unlock(&global_lock);

    char buffer[1024] = "[";
    char temp_str[32];

    while (current != NULL) {
        snprintf(temp_str, sizeof(temp_str), "%d", current->data);
        strcat(buffer, temp_str);

        Node* next = current->next;
        if (next != NULL) {
            strcat(buffer, ", ");
            pthread_mutex_lock(&next->lock);
        }
        pthread_mutex_unlock(&current->lock);
        current = next;
    }
    strcat(buffer, "]");
    printf("%s\n", buffer);
}

int list_count_nodes(Node** head) {
    int count = 0;

    pthread_mutex_lock(&global_lock);
    if (head == NULL || *head == NULL) {
        pthread_mutex_unlock(&global_lock);
        return count;
    }

    Node* current = *head;
    pthread_mutex_lock(&current->lock);
    pthread_mutex_unlock(&global_lock);

    while (current != NULL) {
        count++;
        Node* next = current->next;
        if (next != NULL) {
            pthread_mutex_lock(&next->lock);
        }
        pthread_mutex_unlock(&current->lock);
        current = next;
    }
    return count;
}

void list_cleanup(Node** head) {
    pthread_mutex_lock(&global_lock);
    Node* current = *head;
    *head = NULL;
    pthread_mutex_unlock(&global_lock);

    while (current != NULL) {
        pthread_mutex_lock(&current->lock);
        Node* next_node = current->next;
        pthread_mutex_unlock(&current->lock);
        pthread_mutex_destroy(&current->lock);
        mem_free(current);
        current = next_node;
    }
    mem_deinit();
}