#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "server.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>

int epollfd; 
int listening_sockets[LISTENING_SOCKETS] = {0}; 
thread_info threads[LISTENING_SOCKETS] = {0}; 
int port; 

void init_server(int port)
{
    port = port; 
    for (int i = 0; i < LISTENING_SOCKETS; ++i)
    {
        if (pthread_create(&threads[i].thread_id, NULL, &server_worker, &threads[i]) != 0)
        {
            perror("Creating worker thread"); 
            exit(-1); 
        }
    }

    sigset_t mask; 
    sigemptyset(&mask); 
    sigaddset(&mask, SIGINT); 
    sigprocmask(SIG_BLOCK, &mask, NULL); //block the signal mask 
    int sig; 
    sigwait(&mask, &sig); //block execution untill a signal is received 
    printf("\nClosing server...\n"); 
}

void* server_worker(void* arg)
{
    thread_info* thread = (thread_info*) arg; 
    struct epoll_event ev; 
    thread->thread_id = pthread_self(); 

    //create epoll file descriptor 
    thread->epollfd = epoll_create1(0); 

    //create and bind socket 
    setup_listening_socket(&thread->sockfd);

    //create event that epoll should listen to 
    ev.events = EPOLLET | EPOLLIN | EPOLLERR ; //edge triggered notification
    ev.data.fd = thread->sockfd; 

    //add event to the event list monitored by epoll
    if (epoll_ctl(thread->epollfd, EPOLL_CTL_ADD, thread->sockfd, &ev) == -1)
    {
        perror("Adding socket file descriptor to epoll"); 
        exit(-1); 
    }

    run_event_loop(thread); 

}

void setup_listening_socket(int* sockfd)
{
    int flags; 
    int opt = 1; 
    int rcvbuf = 1024 * 1024;
    struct sockaddr_in addr;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Creating socket"); 
        exit(-1); 
    }

    //add non blocking behaviour 
    flags = fcntl(*sockfd, F_GETFD, 0); 
    fcntl(*sockfd, F_SETFL, flags | O_NONBLOCK);

    //rebind to the same port during time wait period 
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEADDR"); 
        exit(-1); 
    }

    //make more than one socket bind to the same port 
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt SO_REUSEPORT"); 
        exit(-1); 
    }

    //increase receive buffer 
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEPORT, &rcvbuf, sizeof(rcvbuf)) < 0)
    {
        perror("setsockopt SO_REUSEPORT"); 
        exit(-1); 
    }

    //setup listening address
    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all network interfaces 
    addr.sin_port = htons(port); 

    //bind socket 
    if (bind(*sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        perror("binding socket"); 
        exit(-1); 
    }
}

void run_event_loop(thread_info* thread)
{

}