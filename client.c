// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define PORT     8080
#define MAXLINE 1024
#define PAYLOAD_SIZE 128

static unsigned long get_nsecs(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from client: ";
    char payload[PAYLOAD_SIZE] = "";
    struct sockaddr_in     servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
        
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.25");
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    
    while(1) {
        /* Prepare payload */
        bzero(payload, PAYLOAD_SIZE);

        unsigned long now = get_nsecs();
        memcpy(payload, hello, strlen(hello));
        int written = sprintf(payload+strlen(hello), "%lu", now);
        //payload[strlen(hello)+written] = 0;
        // memcpy(payload+strlen(hello)+written, 0, 1);
        // bzero(payload+strlen(hello)+written, 1);
        // memcpy(payload+127, "\0", 1);


        // int n, len;
        int len;
        int sent = sendto(sockfd, (const char *)payload, strlen(payload),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));
        printf("Hello message sent (%lu of %d bytes): %lu\n", strlen(payload), sent, now);
                
        // n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
        //             MSG_WAITALL, (struct sockaddr *) &servaddr,
        //             &len);
        // buffer[n] = '\0';
        // printf("Server : %s\n", buffer);
        sleep(1);
    }

    close(sockfd);
    return 0;
}