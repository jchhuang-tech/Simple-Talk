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
#include "threads.h"
#include "list.h"


int main(int argc, char** args)
{
    if (argc < 4){
        puts("error: too few arguments"); 
        exit(EXIT_FAILURE); 
    }

    talk_init(args);

    talk_waitForShutdown();

    return 0;
}