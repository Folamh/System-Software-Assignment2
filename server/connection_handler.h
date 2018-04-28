#ifndef INTRANETFILETRANSFER_CONNECTION_HANDLER_H
#define INTRANETFILETRANSFER_CONNECTION_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

void *connection_handler(void *socket_desc) {
    char* intranet = "/var/www/html/intranet";
    char* sales = "/usr/intranet/sales";
    char* promotions = "/usr/intranet/promotions";
    char* offers = "/usr/intranet/offers";
    char* marketing = "/usr/intranet/marketing";
    char* test = "./test/intranet";
    char file_name[2000];

    puts("in handler");

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char client_message[2000];

    puts("Get save location");
    if (recv(sock , client_message , 2000 , 0) < 0) {
        puts("failed to receive loc");
    } else {
        puts(client_message);
        if (strcmp(client_message, "intranet") == 0) {
            strcpy(file_name, intranet);
        } else if (strcmp(client_message, "sales") == 0) {
            strcpy(file_name, sales);
        } else if (strcmp(client_message, "promotions") == 0) {
            strcpy(file_name, promotions);
        } else if (strcmp(client_message, "offers") == 0) {
            strcpy(file_name, offers);
        } else if (strcmp(client_message, "marketing") == 0) {
            strcpy(file_name, marketing);
        } else if (strcmp(client_message, "test") == 0) {
            strcpy(file_name, test);
        } else {
            puts("failed cmp");
        }
    }

    puts(file_name);

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        puts("Send failed");
    }

    if (recv(sock , client_message , 2000 , 0) < 0) {
        puts("failed to recv basename");
    } else {
        puts(client_message);
        strcat(file_name, "/");
        strcat(file_name, client_message);
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        puts("Send failed");
    }

    puts("Relieving file.");

    char file_buffer[512]; // Receiver buffer

    FILE *file_open = fopen(file_name, "w");
    if(file_open == NULL)
        printf("File %s Cannot be opened file on server.\n", file_name);
    else {
        bzero(file_buffer, 512);
        int block_size = 0;
        int i=0;
        while((block_size = recv(sock, file_buffer, 512, 0)) > 0) {
            printf("Data Received %d = %d\n",i,block_size);
            int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
            bzero(file_buffer, 512);
            i++;
        }

    }
    printf("Transfer Complete!\n");
    fclose(file_open);

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

#endif //INTRANETFILETRANSFER_CONNECTION_HANDLER_H
