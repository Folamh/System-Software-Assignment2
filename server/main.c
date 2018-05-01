#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "connection_handler.h"
#include "daemonize.h"

int main(int argc , char *argv[]) {
    char key[] = "SystemSoftware-Assignment2";

    // Setup logging and make program a daemon
    openlog(key, LOG_PID|LOG_CONS, LOG_DAEMON);
    daemonize();

    int PORT = 5555;

    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        syslog(LOG_ERR, "Unable to create socket.");
    }
    syslog(LOG_INFO, "Socket created.");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        syslog(LOG_ERR, "Binding failed.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Bind complete.");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection passing it to new thread
    syslog(LOG_INFO, "Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t*) &c))) {
        syslog(LOG_INFO, "Connection accepted.");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*) new_sock) < 0) {
            syslog(LOG_ERR, "Unable to create thread.");
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "Handler assigned.");
    }

    if (client_sock < 0) {
        syslog(LOG_ERR, "Accepting connection failed.");
        exit(EXIT_FAILURE);
    }

    return 0;
}