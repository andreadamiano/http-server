#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <sys/epoll.h>
#include <stdbool.h>

#define LISTENING_SOCKETS 8
#define MAX_EVENTS 64

typedef struct
{
    int server_fd; 
    int epollfd; 
    struct epoll_event events[MAX_EVENTS]; 
    pthread_t thread_id; 
}thread_info;



extern int epollfd; 
extern int listening_sockets[LISTENING_SOCKETS];
extern thread_info threads[LISTENING_SOCKETS];
extern int port; 
extern bool running; 


void init_server(int port); 
void* server_worker(void* args); 
void setup_listening_socket(int* sockfd); 
void run_event_loop(thread_info* thread); 
void handle_event(int epollfd, int fd); 
void add_event (int epollfd, int fd); 
void remove_event(int epollfd, int fd); 
void set_non_blocking(int fd); 

#endif