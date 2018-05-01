#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <syslog.h>

int main(int argc , char *argv[]) {
    char HOST[] = "127.0.0.1";
    int PORT = 5555;

    char key[] = "SystemSoftware-Assignment2-Client";
    openlog(key, LOG_PID|LOG_CONS, LOG_USER);

    // Help
    if (argc == 1 || argc > 3 || strcmp(argv[1], "--help") == 0 || strcmp(argv[2], "-h") == 0) {
        printf("Usage: ift <location> <file>\n"
               "Locations:\n"
               " -> website\n"
               " -> sales\n"
               " -> promotions\n"
               " -> offers\n"
               " -> marketing\n");
        exit(0);
    }

    char* location = argv[1];
    char* file_name = argv[2];
    char* base_name = basename(file_name);

    int sock;
    struct sockaddr_in server;
    char server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        syslog(LOG_ERR, "Unable to create socket.");
        puts("Unable to create socket. Exiting...");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Socket created.");

    server.sin_addr.s_addr = inet_addr(HOST);
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t) PORT);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        syslog(LOG_ERR, "Failed to connect to server.");
        puts("Failed to connect to server. Exiting...");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Connected to server.");
    puts("Connected to server.");

    char user[300];
    printf("Username: ");
    fflush(stdout);
    if (fgets(user, 300, stdin) == NULL)
        exit(EXIT_FAILURE);

    strtok(user, "\n");
    strcat(user, ":");
    strcat(user, getpass("Password: "));

    puts(user);

    // Login
    if (send(sock , user, strlen(user), 0) < 0) {
//        memset(&username[0], 0, sizeof(username));
        syslog(LOG_ERR, "Failed to send user to server.");
        puts("Failed to send user to server. Exiting...");
        exit(EXIT_FAILURE);
    }
//    memset(&username[0], 0, sizeof(username));

    if (recv(sock, server_reply, 2000 , 0) < 0) {
        syslog(LOG_ERR, "Failed to retrieve confirmation from server.");
        puts("Failed to retrieve confirmation from server. Exiting...");
        exit(EXIT_FAILURE);
    }
    if (strcmp(server_reply, "OK") != 0) {
        puts("Failed to send login");
        puts(server_reply);
        exit(EXIT_FAILURE);
    }

    // Save location
    if (send(sock , location, strlen(location), 0) < 0) {
        syslog(LOG_ERR, "Error sending save location to server.");
        puts("Error sending save location to server. Exiting...");
        exit(EXIT_FAILURE);
    }

    if (recv(sock, server_reply, 2000 , 0) < 0) {
        syslog(LOG_ERR, "Failed to retrieve confirmation from server.");
        puts("Failed to retrieve confirmation from server. Exiting...");
        exit(EXIT_FAILURE);
    }
    if (strcmp(server_reply, "OK") != 0) {
        syslog(LOG_ERR, "Retrieved incorrect confirmation from server.");
        puts("Retrieved incorrect confirmation from server. Exiting...");
        puts(server_reply);
        exit(EXIT_FAILURE);
    }

    // Filename
    if (send(sock , base_name, strlen(base_name), 0) < 0) {
        syslog(LOG_ERR, "Error sending save location to server.");
        puts("Error sending save location to server.");
        exit(EXIT_FAILURE);
    }

    if (recv(sock, server_reply, 2000 , 0) < 0) {
        syslog(LOG_ERR, "Failed to retrieve confirmation from server.");
        puts("Failed to retrieve confirmation from server. Exiting...");
        exit(EXIT_FAILURE);
    }
    if (strcmp(server_reply, "OK") != 0) {
        syslog(LOG_ERR, "Retrieved incorrect confirmation from server.");
        puts("Retrieved incorrect confirmation from server. Exiting...");
        puts(server_reply);
        exit(EXIT_FAILURE);
    }

    // File
    char file_buffer[512];
    syslog(LOG_INFO, "Sending %s to the Server... ", file_name);
    printf("Sending %s to the Server...\n", file_name);
    FILE *file_open = fopen(file_name, "r");
    bzero(file_buffer, 512);
    size_t block_size;
    int i = 0;
    while((block_size = fread(file_buffer, sizeof(char), 512, file_open)) > 0) {
        printf("Data Sent %d = %zu\n", i, block_size);
        if(send(sock, file_buffer, block_size, 0) < 0) {
            syslog(LOG_ERR, "Error sending file to server.");
            puts("Error sending file to server.");
            exit(EXIT_FAILURE);
        }
        bzero(file_buffer, 512);
        i++;
    }

    close(sock);
    syslog(LOG_ERR, "Sent file to server.");
    puts("Sent file to server.");
    exit(0);
}