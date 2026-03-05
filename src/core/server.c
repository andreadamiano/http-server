#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "server.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>

int epollfd; 
int listening_sockets[LISTENING_SOCKETS] = {0}; 
thread_info threads[LISTENING_SOCKETS] = {0}; 
int port; 
bool running = false; 
struct sockaddr_in addr;

void init_server(int _port)
{
    port = _port; 
    running = true; 
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
    struct epoll_event* ev; 
    thread->thread_id = pthread_self(); 

    //create epoll file descriptor 
    thread->epollfd = epoll_create1(0); 

    //create and bind socket 
    setup_listening_socket(&thread->server_fd);

    //create event that epoll should listen to 
    add_event(thread->epollfd, thread->server_fd); 

    run_event_loop(thread); 

}

void setup_listening_socket(int* sockfd)
{
    int flags; 
    int opt = 1; 
    int rcvbuf = 1024 * 1024;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Creating socket"); 
        exit(-1); 
    }

    //add non blocking behaviour 
    set_non_blocking(*sockfd);

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
    if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0)
    {
        perror("setsockopt SO_RCVBUF"); 
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
    if (listen(*sockfd, SOMAXCONN) < 0)
    {
        perror("listen"); 
        exit(-1); 
    }
    printf("Listening on port %d\n", port); 
}

void run_event_loop(thread_info* thread)
{
    int n_events; 
    int conn_fd; 
    struct epoll_event* ev; 
    socklen_t addr_len = sizeof(addr);

    while (running) //prevent spurious awakes 
    {
        n_events = epoll_wait(thread->epollfd, thread->events, MAX_EVENTS, -1); //blocking call 

        for (int i = 0; i < n_events; ++i)
        {
            if (thread->events[i].data.fd == thread->server_fd) //new connection received 
            {
                if (thread->events[i].events & EPOLLIN)  
                {
                    if ((conn_fd = accept(thread->server_fd, (struct sockaddr*) &addr, &addr_len)) == -1)
                    {
                        perror("Accepting incoming connection"); 
                    }
                    set_non_blocking(conn_fd);

                    //add new event to epoll
                    add_event(thread->epollfd, conn_fd); 
                }

            }
            else
            {
                if (thread->events[i].events & EPOLLIN)
                {
                    handle_event(thread->epollfd, thread->events[i].data.fd);
                }
                // if (thread->events[i].events & (EPOLLERR | EPOLLHUP))
                // {
                //     handle_client_close(thread, thread->events[i].data.fd);
                // }
            }
        }
    }
    
}

void handle_event(int epollfd, int fd)
{
    char receive_buffer[256]; 
    int n_bytes; 
    
    n_bytes = read(fd, receive_buffer, sizeof(receive_buffer) -1); 

    if (n_bytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            printf("No data available now, try again later\n");
        }
        else 
        {
            perror("read error");
            remove_event(epollfd, fd);
            close(fd);  
        }
    }
    else if (n_bytes > 0)
    {
        fwrite(receive_buffer, 1, n_bytes, stdout);
    }
    else if (n_bytes == 0)
    {
        //received end of file 
    } 

}

void add_event(int epollfd, int fd)
{
    struct epoll_event ev;
    //create event that epoll should listen to 
    ev.events = EPOLLET | EPOLLIN | EPOLLERR ; //edge triggered notification
    ev.data.fd = fd; 

    if ((epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev)) == -1)
    {
        perror("Adding event to epoll"); 
    }
}

void remove_event(int epollfd, int fd)
{
    if ((epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL)) == -1)
    {
        perror("Removing event from epoll"); 
    }
}

void set_non_blocking(int fd)
{
    int flags; 
    flags = fcntl(fd, F_GETFL, 0); 
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}