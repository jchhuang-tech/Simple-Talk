#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <assert.h>
#include "threads.h"
#include "list.h"

#define MAXLINE 512

static char* myPort;
static char* remotePort;
static char* remoteHostName;

static int sockfd;

char* TxMessage = NULL;
char* RxMessage = NULL;

static struct addrinfo *servinfo;

static pthread_t threadPID;
static pthread_t threadPID2;

// Initialize s-talk
void talk_init(char** args)
{
    //set ports
    myPort = args[1];
    remoteHostName = args[2];
    remotePort = args[3];

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
    { 
        perror("socket creation failed");
        exit(EXIT_FAILURE); 
    }
    
    //Creat four threads
    if (pthread_create(
        &threadPID,                // PID (by pointer)
        NULL,                      // Attributes
        receiveThread,             // Function
        NULL) != 0)      // Arguments
    {
        puts("pthread_create failed");
        exit(EXIT_FAILURE);
    }              
   
    if (pthread_create(
        &threadPID2,                // PID (by pointer)
        NULL,                       // Attributes
        transmitThread,             // Function
        NULL) !=0)        // Arguments
    {
        puts("pthread_create failed");
        exit(EXIT_FAILURE);
    } 
}

// Wait for shutdown
void talk_waitForShutdown()
{
    if (pthread_join(threadPID, NULL) != 0)
    {
        puts("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(threadPID2, NULL) != 0)
    {
        puts("pthread_join failed");
        exit(EXIT_FAILURE);
    }

    if(RxMessage!=nullptr){
        delete[] RxMessage;
    }
    if(TxMessage!=nullptr){
        delete[] TxMessage;
    }
    
    //closing the sockets
    if (close(sockfd) < 0)
    {
        perror("socket closing failed");
        exit(EXIT_FAILURE);
    }

}

// Thread for receiving message
void* receiveThread(void* unused)
{

    struct sockaddr_in servaddr, cliaddr; 

    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(atoi(myPort)); 

    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    //len is value/resuslt 
    socklen_t len = sizeof(cliaddr);  

    while(1) 
    {
        //allocate memory for receive message
        RxMessage = new char[MAXLINE];

        //begin to receive a message
        if (recvfrom(sockfd, (char *)RxMessage, MAXLINE,  
            0, ( struct sockaddr *) &cliaddr, 
            &len) < 0)
        {
            perror("recvfrom failed"); 
            exit(EXIT_FAILURE); 
        }

        if (fputs(RxMessage, stdout) < 0){
            puts("fputs failed"); 
            exit(EXIT_FAILURE);
        }

        if (RxMessage[0]=='!'&&RxMessage[1]=='\n')
        {
            // delete[] RxMessage;
            pthread_cancel(threadPID2);
            pthread_exit(NULL);
        }

        delete[] RxMessage;

    }

      
    return NULL;
}

// Thread for sending message
void* transmitThread(void* unused)
{
    //set up variables
    struct addrinfo hints;
    servinfo = 0;
    
    memset(&hints, 0, sizeof hints);
    //Filling information 
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;
    //getaddrinfo
    if(getaddrinfo(remoteHostName, remotePort, &hints, &servinfo) != 0)
    {
        puts("getaddrinfo failed"); 
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr* servinfo_addr = servinfo->ai_addr;
    socklen_t servinfo_addrlen = servinfo->ai_addrlen;

    //free addrinfo
    freeaddrinfo(servinfo);

    while(1) 
    {
        TxMessage = new char[MAXLINE];

        //get message from keyboard
        if(fgets(TxMessage, MAXLINE, stdin))
        {
            if (sendto(sockfd, (char *)TxMessage, MAXLINE, 
                0, (struct sockaddr *) servinfo_addr,  
                servinfo_addrlen) < 0)
            {
                perror("sendto failed"); 
                exit(EXIT_FAILURE);
            }  
            if (TxMessage[0]=='!'&&TxMessage[1]=='\n')
            {
                // delete[] TxMessage;
                pthread_cancel(threadPID);
                pthread_exit(NULL);
            }
        }
        delete[] TxMessage;
    }
    return NULL; 
}