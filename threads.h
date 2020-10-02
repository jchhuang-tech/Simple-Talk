//#ifndef _THREADS_H_
//#define _THREADS_H_

void talk_init(char** args);

void talk_shutdown();

void talk_waitForShutdown();

void* receiveThread(void* unused);

void* transmitThread(void* unused);

void* keyboardThread(void* unused);

void* screenThread(void* unused);