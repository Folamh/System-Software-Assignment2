#ifndef INTRANETFILETRANSFER_CONNECTION_HANDLER_H
#define INTRANETFILETRANSFER_CONNECTION_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>

void *connection_handler(void *socket_desc) {
    char* intranet = "/var/www/html/intranet";
    char* sales = "/usr/intranet/sales";
    char* promotions = "/usr/intranet/promotions";
    char* offers = "/usr/intranet/offers";
    char* marketing = "/usr/intranet/marketing";
    char file_name[2000];

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char client_message[2000];
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve location to save.");
        pthread_exit(NULL);
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
        } else {
            syslog(LOG_WARNING, "Failed to retrieve location to save.");
            pthread_exit(NULL);
        }
    }

    puts(file_name);

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
    }

    if (recv(sock , client_message , 2000 , 0) < 0) {
        puts("failed to recv basename");
    } else {
        puts(client_message);
        strcat(file_name, "/");
        strcat(file_name, client_message);
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
    }

    syslog(LOG_INFO, "Receiving file.");
    char file_buffer[512]; // Receiver buffer
    FILE *file_open = fopen(file_name, "w");
    if(file_open == NULL)
        syslog(LOG_WARNING, "File %s Cannot be opened file on server.", file_name);
    else {
        bzero(file_buffer, 512);
        int block_size = 0;
        int i=0;
        while((block_size = recv(sock, file_buffer, 512, 0)) > 0) {
            int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
            bzero(file_buffer, 512);
            i++;
        }
    }
    syslog(LOG_INFO, "Transfer Complete.");
    fclose(file_open);

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

#endif //INTRANETFILETRANSFER_CONNECTION_HANDLER_H
