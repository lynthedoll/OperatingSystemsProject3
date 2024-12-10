#include "server.h"
#include "list.h"

// External variables
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;
extern struct node *head;
extern struct room *rooms;
extern char *server_MOTD;

// Function to handle client connections and commands
void *client_receive(void *ptr) {
    int client = *(int *) ptr;
    int received;
    char buffer[MAXBUFF], sbuffer[MAXBUFF];
    char tmpbuf[MAXBUFF];
    char cmd[MAXBUFF], username[20];
    char *arguments[80];

    // Generate a guest username and add the user to the global list
    sprintf(username, "guest%d", client);
    head = insertFirstU(head, client, username);

    // Add the user to the default "Lobby" room
    struct room *lobby = find_room(rooms, "Lobby");
    if (lobby) {
        add_user_to_room(lobby, findU(head, username));
    }

    // Send the server's message of the day to the client
    send(client, server_MOTD, strlen(server_MOTD), 0);

    // Main loop to handle client commands
    while (1) {
        if ((received = read(client, buffer, MAXBUFF)) != 0) {
            buffer[received] = '\0';
            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            // Parse the command and arguments
            arguments[0] = strtok(cmd, " \n");
            int i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, " \n");
            }

            // Handle "create" command
            if (strcmp(arguments[0], "create") == 0 && arguments[1] != NULL) {
                struct room *new_room = create_room(arguments[1]);
                if (new_room) {
                    new_room->next = rooms;
                    rooms = new_room;
                    sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Failed to create room.\nchat>");
                }
                send(client, buffer, strlen(buffer), 0);
            }
            // Handle "join" command
            else if (strcmp(arguments[0], "join") == 0 && arguments[1] != NULL) {
                struct room *room = find_room(rooms, arguments[1]);
                if (room) {
                    add_user_to_room(room, findU(head, username));
                    sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room not found.\nchat>");
                }
                send(client, buffer, strlen(buffer), 0);
            }
            // Handle "leave" command
            else if (strcmp(arguments[0], "leave") == 0 && arguments[1] != NULL) {
                struct room *room = find_room(rooms, arguments[1]);
                if (room) {
                    remove_user_from_room(room, findU(head, username));
                    sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Room not found.\nchat>");
                }
                send(client, buffer, strlen(buffer), 0);
            }
            // Handle "rooms" command
            else if (strcmp(arguments[0], "rooms") == 0) {
                list_rooms(rooms, client);
            }
            // Handle "users" command
            else if (strcmp(arguments[0], "users") == 0) {
                list_users(head, client);
            }
            // Handle "login" command
            else if (strcmp(arguments[0], "login") == 0 && arguments[1] != NULL) {
                struct node *user = findU(head, username);
                if (user) {
                    update_username(user, arguments[1]);
                    strcpy(username, arguments[1]);
                    sprintf(buffer, "Username updated to '%s'.\nchat>", arguments[1]);
                } else {
                    sprintf(buffer, "Failed to update username.\nchat>");
                }
                send(client, buffer, strlen(buffer), 0);
            }
            // Handle "exit" or "logout" command
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                remove_user(&head, client);
                close(client);
                break;
            }
            // Handle regular chat messages
            else {
                sprintf(tmpbuf, "\n::%s> %s\nchat>", username, sbuffer);
                struct node *current = head;
                while (current != NULL) {
                    if (client != current->socket) {
                        send(current->socket, tmpbuf, strlen(tmpbuf), 0);
                    }
                    current = current->next;
                }
            }
        }
    }
    return NULL;
}