#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "memory_manager.h"
#include "linked_list.h"

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

void list_init(Node** head, size_t size) {
    mem_init(size);
    *head = NULL;
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
    
    if (temp->data == data) {
        *head = temp->next;
        pthread_mutex_unlock(&global_lock);
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
            pthread_mutex_unlock(&global_lock);
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
    pthread_mutex_unlock(&global_lock);
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

void list_display(Node** head) 
{
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

void list_display_range(Node** head, Node* start_node, Node* end_node) 
{
    if (head == NULL || *head == NULL) {
        printf("[]");
        return;
    }

    Node* current = *head;
    char buffer[1024] = "["; // Buffer to build the output string
    char temp[32]; // Temporary buffer for each node's data
    int start_found = (start_node == NULL); // Start from the beginning if start_node is NULL

    while (current != NULL) {
        if (current == start_node) {
            start_found = 1;
        }

        if (start_found) {
            snprintf(temp, sizeof(temp), "%d", current->data);
            strcat(buffer, temp);
            if (current->next != NULL && current != end_node) {
                strcat(buffer, ", ");
            }
        }

        if (current == end_node) {
            break;
        }

        current = current->next;
    }
    strcat(buffer, "]");

    printf("%s", buffer); // Print the final output string
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


int mainB() {
    Node* head = NULL;
    list_init(&head, sizeof(Node) * 10);

    list_insert(&head, 10);
    list_insert(&head, 20);
    list_insert(&head, 30);
    list_insert(&head, 40);
    list_insert(&head, 50);

    // Call your test functions here


  
    printf("Displaying full list:\n");
    list_display(&head); // Should print: [10, 20, 30, 40, 50]

    printf("testing null in range list\n");
    list_display_range(&head, NULL, NULL); // Should print: [10, 20, 30, 40, 50]


    printf("Displaying range from second to fourth node:\n");
    list_display_range(&head, head->next, head->next->next->next); // Should print: [20, 30, 40]

    int count = list_count_nodes(&head); // Should return 5
    printf("Number of nodes: %d\n", count);

    list_cleanup(&head);

    return 0;
}