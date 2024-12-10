// Authors: Jamiliah Eubanks, Caitlyn Lynch, Rabia Mamo,  Kristian Morgan
#include "server.h"
#include "list.h"
#include <signal.h>

// Global variables
int chat_serv_sock_fd;
int numReaders = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;
char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";
struct node *head = NULL;
struct room *rooms = NULL;

int main(int argc, char **argv) {
    // Set up signal handler for graceful shutdown
    signal(SIGINT, sigintHandler);

    // Create the default "Lobby" room
    rooms = create_room("Lobby");
    if (!rooms) {
        fprintf(stderr, "Failed to create Lobby\n");
        exit(1);
    }

    // Get the server socket
    chat_serv_sock_fd = get_server_socket();

    // Start the server
    if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
        printf("start server error\n");
        exit(1);
    }

    printf("Server Launched! Listening on PORT: %d\n", PORT);

    // Main server loop
    while(1) {
        // Accept new client connections
        int new_client = accept_client(chat_serv_sock_fd);
        if(new_client != -1) {
            // Create a new thread for each client
            pthread_t new_client_thread;
            pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
        }
    }

    // Close the server socket (this part is unreachable in the current implementation)
    close(chat_serv_sock_fd);
    return 0;
}

// Signal handler for graceful shutdown
void sigintHandler(int sig_num) {
    printf("Server shutting down...\n");

    // Free all user nodes
    struct node *current = head;
    while (current != NULL) {
        close(current->socket);
        struct node *temp = current;
        current = current->next;
        free(temp);
    }

    // Free all room structures
    struct room *current_room = rooms;
    while (current_room != NULL) {
        struct room *temp = current_room;
        current_room = current_room->next;
        free(temp);
    }

    printf("All connections closed and resources freed.\n");
    close(chat_serv_sock_fd);
    exit(0);
}

// TODO: Implement get_server_socket(), start_server(), and accept_client() functions
