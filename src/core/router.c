#include "router.h"
#include <string.h>

int route_count;
Route routes[MAX_ROUTES];

bool dispatch_route(Request* request, int client_fd)
{
    for (int i = 0; i < route_count; ++i)
    {
        if (strcmp(request->method, routes[i].http_method) == 0 && strcmp(request->path, routes[i].path) == 0)
        {
            routes[i].handler(request->request_body, client_fd); 
            return true; 
        }
    }
    return false; 
}