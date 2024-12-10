#ifndef LIST_H
#define LIST_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Structure to represent a user in the chat server
struct node {
   char username[30];  // Username of the user (max 30 characters)
   int socket;         // Socket descriptor for the user's connection
   struct node *next;  // Pointer to the next user in the global list
   struct node *next_in_room;  // Pointer to the next user in the room list
};

// Structure to represent a chat room
struct room {
    char name[50];     // Name of the chat room (max 50 characters)
    struct node* users;  // Pointer to the list of users in this room
    struct room* next;   // Pointer to the next room in the list
};

// Function to insert a new user at the beginning of the user list
struct node* insertFirstU(struct node* head, int socket, char* username);

// Function to find a user by their username
struct node* findU(struct node* head, char* username);

// Function to create a new chat room
struct room* create_room(char* room_name);

// Function to add a user to a specific chat room
void add_user_to_room(struct room* room, struct node* user);

// Function to remove a user from a specific chat room
void remove_user_from_room(struct room* room, struct node* user);

// Function to find a room by its name
struct room* find_room(struct room* rooms, char* room_name);

// Function to list all available rooms to a client
void list_rooms(struct room* rooms, int client_socket);

// Function to list all connected users to a client
void list_users(struct node* users, int client_socket);

// Function to update a user's username
void update_username(struct node* user, char* new_username);

// Function to remove a user from the global user list
void remove_user(struct node** head, int socket);

#endif