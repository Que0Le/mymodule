// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include "common.h"
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static unsigned long get_nsecs(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

#define PORT     8080
#define MAXLINE 1024
  
// Driver code
int main() {
    /* Allocate mem for log */
    unsigned long *log_time_stamps = (unsigned long *) malloc(MAX_LOG_ENTRY*8);
    if (!log_time_stamps) {
        printf("Malloc log_time_stamps failed\n!");
        return -1;
    }
    memset(log_time_stamps, 0, MAX_LOG_ENTRY*8);
    for (int i=0; i<MAX_LOG_ENTRY; i++) {
        if (log_time_stamps[i] != 0) {
            printf("memset log_time_stamps failed\n!");
            return -1;
        }
    }

    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
        
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
        
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
        
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
        
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGTSTP, sig_handler);
    //Ctrl+C - SIGINT
    //Ctrl+\ - SIGQUIT
    //Ctrl+Z - SIGTSTP

    printf("Server listening ... Ctrl+Z to break while(). Ctrl+C to terminate the program.\n");
    while(keep_running) {
        int len, n;
        bzero(buffer, MAXLINE);
        len = sizeof(cliaddr);  //len is value/resuslt

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);
        unsigned long now = get_nsecs();
        struct Payload pl;
        memcpy(&pl, buffer, sizeof(struct Payload));
        if (pl.uid <MAX_LOG_ENTRY && pl.uid >= 0) {
            log_time_stamps[pl.uid] = now;
        }
#ifdef DEBUG_US_INCOMING_PACKETS
        printf("-------------------------------------------------------\n");
        printf("Client_id[%lu] uid[%lu] type[%lu] create_time[%lu]\n",
                pl.client_uid, pl.uid, pl.type, pl.created_time);
        printf("             ks_1[%lu] ks_2[%lu] us_1[%lu] us_2[%lu]\n", 
                pl.ks_time_arrival_1, pl.ks_time_arrival_2,
                pl.us_time_arrival_1, pl.us_time_arrival_2);
#endif
    }

    /* Export log to text file */
    printf("\nExporting log file ...\n");   // enter new line to avoid the Ctrl+C (^C) char
    FILE *fp;
    // char buf[128];
    fp = fopen(path_log_export_us, "w");
    for (unsigned long i=0; i<MAX_LOG_ENTRY; i++) {
        // memset(buf, '\0', 128);
        // snprintf(buf, 100, "%lu\n", log_time_stamps[i]);
        fprintf(fp, "%lu\n", log_time_stamps[i]);
        // fprintf(fp, (char *) buf);
    }
    fclose(fp);
        
    return 0;
}