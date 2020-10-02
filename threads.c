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
#include "threads.h"
#include "list.h"
#include <assert.h>

#define MAXLINE 512

static pthread_cond_t s_syncOkToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_cond_t s_syncOkToPrintCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOkToTxListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_syncOkToRxListMutex = PTHREAD_MUTEX_INITIALIZER;

static char* myPort;
static char* remotePort;
static char* remoteHostName;

static int sockfd;

static struct addrinfo *servinfo;

static pthread_t threadPID;
static pthread_t threadPID2;
static pthread_t threadPID3;
static pthread_t threadPID4;

static List* TxList;
static List* RxList;

static char* RxMessage = NULL; 
static char* TxMessage = NULL;
static char* keyboardMessage = NULL;
static char* screenMessage = NULL;

static bool receiveClose = false;
static bool transmitClose = false;
static bool keyboardClose = false;
static bool screenClose = false;

//for List_free
static void FreeFn(void* pItem) 
{
    free(pItem);
}

// Initialize s-talk
void talk_init(char** args)
{
    //set ports
    myPort = args[1];
    remoteHostName = args[2];
    remotePort = args[3];

    // creat the List structure to exchange data between threads
    if((TxList = List_create()) == NULL)
    {
        puts("List_create failed");
        exit(EXIT_FAILURE);
    }
    if((RxList = List_create()) == NULL)
    {
        puts("List_create failed");
        exit(EXIT_FAILURE);
    }

    //check if socket works
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

    if (pthread_create(
        &threadPID3,                   // PID (by pointer)
        NULL,                          // Attributes
        keyboardThread,                // Function
        NULL) != 0)          // Arguments
    {
        puts("pthread_create failed");
        exit(EXIT_FAILURE);
    }   

    if(pthread_create(
        &threadPID4,                    // PID (by pointer)
        NULL,                           // Attributes
        screenThread,                   // Function
        NULL) != 0)           // Arguments
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
    if (pthread_join(threadPID3, NULL) != 0)
    {
        puts("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(threadPID4, NULL) != 0)
    {
        puts("pthread_join failed");
        exit(EXIT_FAILURE);
    }

    
    //free the last allocated memory when shutdown
    if(RxMessage != NULL){
        free(RxMessage);
    }
    if(TxMessage != NULL){
        free(TxMessage);
    }
    if(screenMessage != NULL){
        free(screenMessage);
    }
    if(keyboardMessage != NULL){
        free(keyboardMessage);
    }

    
    //closing the sockets
    if (close(sockfd) < 0)
    {
        perror("socket closing failed");
        exit(EXIT_FAILURE);
    }


    //freeing lists
    List_free(TxList, FreeFn);
    List_free(RxList, FreeFn);



    //freeing mutex & condition variables
    if (pthread_cond_destroy(&s_syncOkToSendCondVar) != 0)
    {
        puts("pthread_cond_destroy failed");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_destroy(&s_syncOkToPrintCondVar) != 0)
    {
        puts("pthread_cond_destroy failed");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_mutex_destroy(&s_syncOkToTxListMutex) != 0)
    {
        puts("pthread_mutex_destroy Tx failed");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_destroy(&s_syncOkToRxListMutex) != 0)
    {
        puts("pthread_mutex_destroy Rx failed");
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
        RxMessage = malloc(sizeof(char) *MAXLINE);

        //begin to receive a message
        if (recvfrom(sockfd, (char *)RxMessage, MAXLINE,  
            0, ( struct sockaddr *) &cliaddr, 
            &len) < 0)
        {
            perror("recvfrom failed"); 
            exit(EXIT_FAILURE); 
        }
        

        // Critical Section
        // mutex part: control writing to list
        pthread_mutex_lock(&s_syncOkToRxListMutex);
        {
            //return error massage when list is full & add message to list
            if(List_prepend(RxList, RxMessage) < 0)
            {
                puts("List_prepend failed");
                exit(EXIT_FAILURE);
            }

            if (RxMessage[0]=='!'&&RxMessage[1]=='\n')
            {
                pthread_cond_signal(&s_syncOkToPrintCondVar);
                pthread_mutex_unlock(&s_syncOkToRxListMutex);
                receiveClose = true;
                pthread_exit(NULL);
            }
            


            // conditional variable 
            // use conditional signal/wait synchronize their actions in relation to each other
            pthread_cond_signal(&s_syncOkToPrintCondVar);
        }
        pthread_mutex_unlock(&s_syncOkToRxListMutex);
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
        
        // Critical Section
        // mutex part: control read from list 
        pthread_mutex_lock(&s_syncOkToTxListMutex);
        {
            //if there is no message to send, wait here for keyboard
            if (List_count(TxList) == 0)
            {
                // conditional variable 
                // use conditional signal/wait synchronize their actions in relation to each other
                pthread_cond_wait(&s_syncOkToSendCondVar, &s_syncOkToTxListMutex);
            }
            //get message from list
            TxMessage = List_trim(TxList);
        }
        pthread_mutex_unlock(&s_syncOkToTxListMutex);
        
        if(transmitClose){
            screenClose = true;
            if(!receiveClose){
                if(pthread_cancel(threadPID) != 0)
                {
                    puts("pthread_cancel failed");
                    exit(EXIT_FAILURE);
                }
            }
            pthread_cond_signal(&s_syncOkToPrintCondVar);
            pthread_exit(NULL);
        }

        

        //begin to send a message
        if (sendto(sockfd, (char *)TxMessage, MAXLINE, 
            0, (struct sockaddr *) servinfo_addr,  
            servinfo_addrlen) < 0)
        {
            perror("sendto failed"); 
            exit(EXIT_FAILURE);
        }  


        if (TxMessage[0]=='!'&&TxMessage[1]=='\n')
        {
            screenClose = true;
            if(!receiveClose){
                if(pthread_cancel(threadPID) != 0)
                {
                    puts("pthread_cancel failed");
                    exit(EXIT_FAILURE);
                }
            }
            pthread_cond_signal(&s_syncOkToPrintCondVar);
            pthread_exit(NULL);
        }


        //free the memory allocated by sending thread
        free(TxMessage);
        TxMessage = NULL; //double check

    }


    return NULL; 
}

// Thread for keyboard to input message
void* keyboardThread(void* unused)
{
    while(1) 
    {

         //allocate memory for sending message
        keyboardMessage = malloc(sizeof(char)*MAXLINE);


        //get message from keyboard
        if(fgets(keyboardMessage, MAXLINE, stdin))
        {
            // Critical Section
            // mutex part: control write to list
            pthread_mutex_lock(&s_syncOkToTxListMutex);
            {
                //return error massage when list is full & add message to list
                if(List_prepend(TxList, keyboardMessage) < 0)
                {
                    puts("List_prepend failed");
                    exit(EXIT_FAILURE);
                }
                if (keyboardMessage[0]=='!'&&keyboardMessage[1]=='\n')
                {
                    // transmitClose = true;
                    pthread_cond_signal(&s_syncOkToSendCondVar);
                    pthread_mutex_unlock(&s_syncOkToTxListMutex);
                    keyboardClose = true;
                    pthread_exit(NULL);
                }
                

                // conditional variable 
                // use conditional signal/wait synchronize their actions in relation to each other
                pthread_cond_signal(&s_syncOkToSendCondVar);
            }
            pthread_mutex_unlock(&s_syncOkToTxListMutex);
            
        }

    }

    return NULL;
}

// Thread for print message to screen
void* screenThread(void* unused)
{
    while(1) 
    {

        // Critical Section
        // mutex part: control read from list 
        pthread_mutex_lock(&s_syncOkToRxListMutex);
        {
            //if there is no message to print, wait here for receiver
            if (List_count(RxList) == 0)
            {
                // conditional variable 
                // use conditional signal/wait synchronize their actions in relation to each other
                pthread_cond_wait(&s_syncOkToPrintCondVar, &s_syncOkToRxListMutex);
            }
            //get message from list
            screenMessage = List_trim(RxList);
        }
        pthread_mutex_unlock(&s_syncOkToRxListMutex);

        if(screenClose){
            transmitClose = true;
            if(!keyboardClose){
                if(pthread_cancel(threadPID3) != 0)
                {
                    puts("pthread_cancel failed");
                    exit(EXIT_FAILURE);
                }
            }
            pthread_cond_signal(&s_syncOkToSendCondVar);
            pthread_exit(NULL);
        }

        //print message to screen
        if (fputs(screenMessage, stdout) < 0){
            puts("fputs failed"); 
            exit(EXIT_FAILURE);
        }

        if (screenMessage[0]=='!'&&screenMessage[1]=='\n')
        {
            transmitClose = true;
            if(!keyboardClose){
                if(pthread_cancel(threadPID3) != 0)
                {
                    puts("pthread_cancel failed");
                    exit(EXIT_FAILURE);
                }
            }
            pthread_cond_signal(&s_syncOkToSendCondVar);
            pthread_exit(NULL);
        }



        //free the memory allocated by receiving thread
        free(screenMessage);
        screenMessage = NULL;

    }

    return NULL;
}



