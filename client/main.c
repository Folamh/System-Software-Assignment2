#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

int main(int argc , char *argv[]) {
    char HOST[] = "127.0.0.1";
    int PORT = 5555;

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
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(HOST);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        exit(1);
    }

    puts("Connected\n");

    if( send(sock , location , strlen(location) , 0) < 0) {
        puts("Send failed");
        exit(1);
    }

    if( send(sock , base_name , strlen(base_name) , 0) < 0) {
        puts("Send failed");
        exit(1);
    }

    char file_buffer[512];
    printf("[Client] Sending %s to the Server... ", file_name);
    FILE *file_open = fopen(file_name, "r");
    bzero(file_buffer, 512);
    size_t block_size;
    int i = 0;
    while((block_size = fread(file_buffer, sizeof(char), 512, file_open)) > 0) {
        printf("Data Sent %d = %zu\n", i, block_size);
        if(send(sock, file_buffer, block_size, 0) < 0) {
            puts("Send failed");
            exit(1);
        }
        bzero(file_buffer, 512);
        i++;
    }

    //Receive a reply from the server
    if( recv(sock , server_reply , 2000 , 0) < 0) {
        puts("recv failed");
    }
    puts("Server reply :");
    puts(server_reply);

    close(sock);
    exit(0);
}