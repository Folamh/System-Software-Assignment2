#ifndef INTRANETFILETRANSFER_CONNECTION_HANDLER_H
#define INTRANETFILETRANSFER_CONNECTION_HANDLER_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/file.h>
#include <shadow.h>
#include <pwd.h>
#include <crypt.h>
#include <errno.h>
#include <stdbool.h>

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

    // Login
    if (recv(sock , client_message , 2000 , 0) < 0) {
        pthread_exit(NULL);
    } else {
        char *token, *username, *password, *encrypted, *p;
        struct passwd *pwd;
        struct spwd *spwd;
        bool authOk;

        token = strtok(client_message, ":");
        username = token;
        puts(username);
        token = strtok(NULL, ":");
        password = token;

        for (p = token; *p != '\0'; )
            *p++ = '\0';

        pwd = getpwnam(username);
        if (pwd == NULL)
            pthread_exit(NULL);
        spwd = getspnam(username);
        if (spwd == NULL && errno == EACCES)
            pthread_exit(NULL);

        if (spwd != NULL) // If there is a shadow password record
            pwd->pw_passwd = spwd->sp_pwdp;     // Use the shadow password

        // Encrypt password and erase cleartext version immediately

        encrypted = crypt(password, pwd->pw_passwd);
        for (p = password; *p != '\0'; )
            *p++ = '\0';

        if (encrypted == NULL)
            pthread_exit(NULL);

        authOk = strcmp(encrypted, pwd->pw_passwd) == 0;
        if (!authOk) {
            pthread_exit(NULL);
        }
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
        pthread_exit(NULL);
    }

    // Location
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
            syslog(LOG_WARNING, "Failed to retrieve location where to save.");
            pthread_exit(NULL);
        }
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
        pthread_exit(NULL);
    }

    // Filename
    if (recv(sock , client_message , 2000 , 0) < 0) {
        syslog(LOG_WARNING, "Failed to retrieve filename.");
        pthread_exit(NULL);
    } else {
        strcat(file_name, "/");
        strcat(file_name, client_message);
    }

    if(send(sock , "OK", strlen("OK") , 0) < 0) {
        syslog(LOG_WARNING, "Sending OK signal failed.");
        pthread_exit(NULL);
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

    pthread_exit(NULL);
}

#endif //INTRANETFILETRANSFER_CONNECTION_HANDLER_H
