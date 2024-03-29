#ifndef INTRANETFILETRANSFER_CONNECTION_HANDLER_H
#define INTRANETFILETRANSFER_CONNECTION_HANDLER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/file.h>
#include <crypt.h>
#include <shadow.h>

void *connection_handler(void *socket_desc) {
    // Setup of save locations
    char* intranet = "/var/www/html/intranet";
    char* sales = "/usr/intranet/sales";
    char* promotions = "/usr/intranet/promotions";
    char* offers = "/usr/intranet/offers";
    char* marketing = "/usr/intranet/marketing";
    char file_name[2000];
    struct spwd* sp;

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char client_message[2000];

    // Data from client
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve data.");
        if(send(sock , "Communication-Failed", strlen("Communication-Failed") , 0) < 0) {
            syslog(LOG_WARNING, "Sending Communication-Failed signal failed.");
            return 0;
        }
        return 0;
    }

    // Parse our information
    char token[2000];
    strcpy(token, client_message);
    puts(client_message);
    char* username = strtok(token, ":");
    char* password = strtok(NULL, ":");
    char* location = strtok(NULL, ":");
    char* file = strtok(NULL, ":");

    // Check username against /etc/shadow and get user details
    if((sp = getspnam(username)) == NULL) {
        if(send(sock , "Unauthorized.", strlen("Unauthorized.") , 0) < 0) {
            syslog(LOG_WARNING, "Sending unauthorized signal failed.");
            return 0;
        }
        return 0;
    }
    char *result;
    int ok;

    // hash password using user salt and check if the two hashes are the same
    result = crypt(password, sp->sp_pwdp);
    ok = strcmp (result, sp->sp_pwdp);
    if ( ok != 0 ) {
        syslog(LOG_ERR, "Access denied.");
        if(send(sock , "Unauthorized", strlen("Unauthorized") , 0) < 0) {
            syslog(LOG_WARNING, "Sending unauthorized signal failed.");
            return 0;
        }
        return 0;
    }

    // Check save location
    if (strcmp(location, "intranet") == 0) {
        strcpy(file_name, intranet);
    } else if (strcmp(location, "sales") == 0) {
        strcpy(file_name, sales);
    } else if (strcmp(location, "promotions") == 0) {
        strcpy(file_name, promotions);
    } else if (strcmp(location, "offers") == 0) {
        strcpy(file_name, offers);
    } else if (strcmp(location, "marketing") == 0) {
        strcpy(file_name, marketing);
    } else {
        syslog(LOG_ERR, "Incorrect locations sent.");
        if(send(sock , "Incorrect-Location", strlen("Incorrect-Location") , 0) < 0) {
            syslog(LOG_WARNING, "Sending incorrect-location signal failed.");
            return 0;
        }
        return 0;
    }

    // Create path for saving
    strcat(file_name, "/");
    strcat(file_name, file);

    if(send(sock , "Authorized", strlen("Authorized") , 0) < 0) {
        syslog(LOG_WARNING, "Sending authorized signal failed.");
          return 0;
    }

    // File transfer
    syslog(LOG_INFO, "Receiving file.");
    char file_buffer[512]; // Receiver buffer
    FILE *file_open = fopen(file_name, "w");
    while (flock(fileno(file_open), LOCK_EX) != 0) {
        sleep(1);
    }
    if(file_open == NULL)
        syslog(LOG_WARNING, "File %s Cannot be opened file on server.", file_name);
    else {
        puts("Beginning file.");
        bzero(file_buffer, 512);
        size_t block_size = 0;
        int i=0;
        while((block_size = (size_t) recv(sock, file_buffer, 512, 0)) > 0) {
            fwrite(file_buffer, sizeof(char), block_size, file_open);
            bzero(file_buffer, 512);
            i++;
        }
        syslog(LOG_INFO, "Transfer Complete.");
    }
    flock(fileno(file_open), LOCK_UN);
    fclose(file_open);

    //Free the socket pointer
    free(socket_desc);

    return 0;
}

#endif //INTRANETFILETRANSFER_CONNECTION_HANDLER_H
