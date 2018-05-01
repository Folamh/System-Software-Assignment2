#ifndef INTRANETFILETRANSFER_CONNECTION_HANDLER_H
#define INTRANETFILETRANSFER_CONNECTION_HANDLER_H

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
    char* intranet = "/var/www/html/intranet";
    char* sales = "/usr/intranet/sales";
    char* promotions = "/usr/intranet/promotions";
    char* offers = "/usr/intranet/offers";
    char* marketing = "/usr/intranet/marketing";
    char file_name[2000];

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char client_message[2000];

    // Username
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve user.");
        if(send(sock , "FAIL-Login", strlen("FAIL-Login") , 0) < 0) {
            syslog(LOG_WARNING, "Sending FAIL-Login signal failed.");
            return(NULL);
        }
        return(NULL);
    }

    char* token;
    strcpy(token, client_message);

    char* username = strtok(token, ":");

    char* password = strtok(NULL, ":");

    struct spwd* sp;

    puts("Hello?");
    if( ( sp = getspnam(username) ) == NULL) {
        return(NULL);
    }
    puts("Hi");
    char *result;
    int ok;
    result = crypt(password, sp->sp_pwdp);
    ok = strcmp (result, sp->sp_pwdp);
    if ( ok != 0 ) {
        puts ("Access denied\n");
        syslog(LOG_WARNING, "Failed to retrieve user.");
        if(send(sock , "FAIL-User", strlen("FAIL-User") , 0) < 0) {
            syslog(LOG_WARNING, "Sending FAIL-User signal failed.");
            return(NULL);
        }
        return(NULL);
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
        return(NULL);
    }

    // Location
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve location to save.");
        syslog(LOG_WARNING, "Failed to retrieve user.");
        if(send(sock , "FAIL-Location", strlen("FAIL-Location") , 0) < 0) {
            syslog(LOG_WARNING, "Sending FAIL-Location signal failed.");
            return(NULL);
        }
        return(NULL);
    } else {
        syslog(LOG_DEBUG, "%s", client_message);
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
            syslog(LOG_WARNING, "Incorrect location.");
            if(send(sock , "FAIL", strlen("FAIL") , 0) < 0) {
                syslog(LOG_WARNING, "Sending FAIL signal failed.");
                return(NULL);
            }
            return(NULL);
        }
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
          return(NULL);
    }

    // Filename
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve filename.");
          return(NULL);
    } else {
        syslog(LOG_DEBUG, "%s", client_message);
        strcat(file_name, "/");
        strcat(file_name, client_message);
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
          return(NULL);
    }

    // File
    syslog(LOG_INFO, "Receiving file.");
    char file_buffer[512]; // Receiver buffer
    FILE *file_open = fopen(file_name, "w");
    while (flock(fileno(file_open), LOCK_EX) != 0) {
        sleep(1);
    }
    if(file_open == NULL)
        syslog(LOG_WARNING, "File %s Cannot be opened file on server.", file_name);
    else {
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

      return(NULL);
}

#endif //INTRANETFILETRANSFER_CONNECTION_HANDLER_H
