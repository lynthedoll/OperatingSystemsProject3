// Authors: Jamiliah Eubanks, Caitlyn Lynch, Rabia Mamo, Kristin Morgan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

extern pthread_mutex_t mutex;
extern pthread_mutex_t rw_lock;
extern int numReaders;

// Function to insert a new user at the beginning of the list
struct node* insertFirstU(struct node* head, int socket, char* username) {
    pthread_mutex_lock(&mutex);
    if(findU(head, username) == NULL) {
        struct node* link = (struct node*) malloc(sizeof(struct node));
        link->socket = socket;
        strncpy(link->username, username, sizeof(link->username) - 1);
        link->username[sizeof(link->username) - 1] = '\0';
        link->next = head;
        link->next_in_room = NULL;
        head = link;
    }
    pthread_mutex_unlock(&mutex);
    return head;
}

// Function to find a user by username
struct node* findU(struct node* head, char* username) {
    // Implement reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders++;
    if (numReaders == 1) {
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);

    struct node* current = head;
    while(current != NULL) {
        if(strcmp(current->username, username) == 0) {
            // Release reader lock
            pthread_mutex_lock(&rw_lock);
            numReaders--;
            if (numReaders == 0) {
                pthread_mutex_unlock(&mutex);
            }
            pthread_mutex_unlock(&rw_lock);
            return current;
        }
        current = current->next;
    }

    // Release reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders--;
    if (numReaders == 0) {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);
    return NULL;
}

// Function to create a new chat room
struct room* create_room(char* room_name) {
    pthread_mutex_lock(&mutex);
    struct room* new_room = (struct room*) malloc(sizeof(struct room));
    if (new_room == NULL) {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    strncpy(new_room->name, room_name, sizeof(new_room->name) - 1);
    new_room->name[sizeof(new_room->name) - 1] = '\0';
    new_room->users = NULL;
    new_room->next = NULL;
    pthread_mutex_unlock(&mutex);
    return new_room;
}

// Function to add a user to a chat room
void add_user_to_room(struct room* room, struct node* user) {
    pthread_mutex_lock(&mutex);
    if (room != NULL && user != NULL) {
        user->next_in_room = room->users;
        room->users = user;
    }
    pthread_mutex_unlock(&mutex);
}

// Function to remove a user from a chat room
void remove_user_from_room(struct room* room, struct node* user) {
    pthread_mutex_lock(&mutex);
    if (room != NULL && user != NULL && room->users != NULL) {
        if (room->users == user) {
            room->users = user->next_in_room;
        } else {
            struct node* current = room->users;
            while (current->next_in_room != NULL && current->next_in_room != user) {
                current = current->next_in_room;
            }
            if (current->next_in_room == user) {
                current->next_in_room = user->next_in_room;
            }
        }
        user->next_in_room = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

// Function to find a room by its name
struct room* find_room(struct room* rooms, char* room_name) {
    // Implement reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders++;
    if (numReaders == 1) {
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);

    struct room* current = rooms;
    while (current != NULL) {
        if (strcmp(current->name, room_name) == 0) {
            // Release reader lock
            pthread_mutex_lock(&rw_lock);
            numReaders--;
            if (numReaders == 0) {
                pthread_mutex_unlock(&mutex);
            }
            pthread_mutex_unlock(&rw_lock);
            return current;
        }
        current = current->next;
    }

    // Release reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders--;
    if (numReaders == 0) {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);
    return NULL;
}

// Function to list all available rooms
void list_rooms(struct room* rooms, int client_socket) {
    // Implement reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders++;
    if (numReaders == 1) {
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);

    char buffer[1024] = "Available rooms:\n";
    struct room* current = rooms;
    while (current != NULL) {
        strcat(buffer, current->name);
        strcat(buffer, "\n");
        current = current->next;
    }
    strcat(buffer, "chat>");
    send(client_socket, buffer, strlen(buffer), 0);

    // Release reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders--;
    if (numReaders == 0) {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);
}

// Function to list all connected users
void list_users(struct node* users, int client_socket) {
    // Implement reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders++;
    if (numReaders == 1) {
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);

    char buffer[1024] = "Connected users:\n";
    struct node* current = users;
    while (current != NULL) {
        strcat(buffer, current->username);
        strcat(buffer, "\n");
        current = current->next;
    }
    strcat(buffer, "chat>");
    send(client_socket, buffer, strlen(buffer), 0);

    // Release reader lock
    pthread_mutex_lock(&rw_lock);
    numReaders--;
    if (numReaders == 0) {
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&rw_lock);
}

// Function to update a user's username
void update_username(struct node* user, char* new_username) {
    pthread_mutex_lock(&mutex);
    if (user != NULL) {
        strncpy(user->username, new_username, sizeof(user->username) - 1);
        user->username[sizeof(user->username) - 1] = '\0';
    }
    pthread_mutex_unlock(&mutex);
}

// Function to remove a user from the global user list
void remove_user(struct node** head, int socket) {
    pthread_mutex_lock(&mutex);
    struct node* current = *head;
    struct node* prev = NULL;

    while (current != NULL && current->socket != socket) {
        prev = current;
        current = current->next;
    }

    if (current != NULL) {
        if (prev == NULL) {
            *head = current->next;
        } else {
            prev->next = current->next;
        }
        free(current);
    }
    pthread_mutex_unlock(&mutex);
}
