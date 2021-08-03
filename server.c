// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "common.h"

#define PORT     8080
#define MAXLINE 1024
  
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
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
    
    while(1) {
        int len, n;
        bzero(buffer, MAXLINE);
        len = sizeof(cliaddr);  //len is value/resuslt

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);

        struct Payload pl;
        memcpy(&pl, buffer, sizeof(struct Payload));
        printf("-------------------------------------------------------\n");
        printf("Client_id[%lu] uid[%lu] type[%lu] create_time[%lu]\n",
                pl.client_uid, pl.uid, pl.type, pl.created_time);
        printf("             ks_1[%lu] ks_2[%lu] us_1[%lu] us_2[%lu]\n", 
                pl.ks_time_arrival_1, pl.ks_time_arrival_2,
                pl.us_time_arrival_1, pl.us_time_arrival_2);

        // buffer[n] = '\0';
        // printf("Msg : %s\n", buffer);



        // sendto(sockfd, (const char *)hello, strlen(hello), 
        //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
        //         len);
        // printf("Hello message sent.\n"); 
    }
        
    return 0;
}