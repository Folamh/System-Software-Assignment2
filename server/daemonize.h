#ifndef INTRANETFILETRANSFER_DEMONIZE_H
#define INTRANETFILETRANSFER_DEMONIZE_H

#include <stdio.h>
#include <sys/syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

void daemonize() {
    syslog(LOG_INFO, "Daemonizing program.");
    // Step 1: Create orphan process.
    syslog(LOG_DEBUG, "Step 1: Creating orphan process.");
    int pid = fork();

    if (pid < 0) { // Check that we have a valid pid
        syslog(LOG_ERR, "Invalid pid generated. Exiting...");
        closelog();
        exit(EXIT_FAILURE);
    }

    if (pid > 0) { // Exit the parent process
        syslog(LOG_DEBUG, "Terminating parent process.");
        exit(EXIT_SUCCESS);
    }

    // Step 2: Elevate orphan.
    syslog(LOG_DEBUG, "Step 2 :Elevating orphan.");
    if (setsid() < 0) {
        syslog(LOG_ERR, "Failed to elevate orphan. Exiting...");
        closelog();
        exit(EXIT_FAILURE);
    }

    // Step 3: Give read and write permissions.
    syslog(LOG_DEBUG, "Step 3: Giving read and write permissions.");
    umask(0);

    // Step 4: Change working dir to root.
    syslog(LOG_DEBUG, "Step 4: Changing working directory to root.");
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "Failed to change working directory to root. Exiting...");
        closelog();
        exit(EXIT_FAILURE);
    }

    // Step 5: Close open file descriptors.
    syslog(LOG_DEBUG, "Step 5: Closing open file descriptors.");
    for (int i = (int) sysconf(_SC_OPEN_MAX); i >= 0 ; i--)
        close (i);

    syslog(LOG_INFO, "Daemon successfully created.");
}

#endif //INTRANETFILETRANSFER_DEMONIZE_H
