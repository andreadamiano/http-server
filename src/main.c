#include <stdio.h>
#include "core/server.h"
#include "core/router.h"
#include <sys/socket.h>

ROUTE(POST, /) 
{
    printf("Handler for POST /\n");
    char buffer[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from POST /\n";
    send(client_fd, buffer, sizeof(buffer) - 1, 0);
}


int main()
{
    init_server(8080); 
}