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
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) 
    {

        *head = new_node;
        
    } 
    else 
    {

        Node* temp = *head;


        while (temp->next != NULL) 
        {
            Node* next = temp->next;

        }
        temp->next = new_node;

    }
}

void list_insert_after(Node* prev_node, uint16_t data) {
        pthread_mutex_lock(&prev_node->lock);
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
    new_node->data = data;
    new_node->next = prev_node->next;

    pthread_mutex_init(&new_node->lock, NULL);
    prev_node->next = new_node;
    pthread_mutex_unlock(&prev_node->lock);
}

void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        fprintf(stderr, "Next node cannot be NULL\n");
        return;
    }
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp != NULL && temp->next != next_node) {
            temp = temp->next;
        }
        if (temp == NULL) {
            fprintf(stderr, "Next node not found in the list\n");
            mem_free(new_node);
            return;
        }
        new_node->next = next_node;
        temp->next = new_node;
    }
}

void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        return;
    }
    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && temp->data == data) {
        *head = temp->next;
        mem_free(temp);
        return;
    }

    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return;
    }

    prev->next = temp->next;
    mem_free(temp);
}

Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_display(Node** head) 
{
    if (head == NULL || *head == NULL) {
        printf("[]");
        return;
    }

    Node* current = *head;
    char buffer[1024] = "["; // Buffer to build the output string
    char temp[32]; // Temporary buffer for each node's data

    while (current != NULL) {
        snprintf(temp, sizeof(temp), "%d", current->data);
        strcat(buffer, temp);
        if (current->next != NULL) {
            strcat(buffer, ", ");
        }
        current = current->next;
    }
    strcat(buffer, "]");

    printf("%s", buffer); // Print the final output string
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

    if (head == NULL || *head == NULL) {
        return 0;
    }

    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

void list_cleanup(Node** head) {
    Node* current = *head;
    Node* next_node;
    while (current != NULL) {
        next_node = current->next;
        mem_free(current);
        current = next_node;
    }
    *head = NULL;
    mem_deinit(); // Clean up the memory manager (I forgot this thanks)
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