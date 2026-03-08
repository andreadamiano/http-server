#ifndef ROUTE_H
#define ROUTE_H

#include <stdbool.h>

#define MAX_ROUTES 100


typedef struct Route
{
    char* http_method; 
    char* path; 
    void (*handler) (const char* request_body, int client_fd); 
}Route;

typedef struct Request
{
    char* method; 
    char* path; 
    char* request_body;
}Request;

extern Route routes []; 
extern int route_count; 


//this macro does:
//forward declaration of the route handler  
//forward declaration of the route register  (the attribute constructor will run this function before main)
//definition of the implementation of the route register 
//definition of the start of the handler function which will be completed afterwards 
#define CONCAT(a, b) a##b //## is a preprocessor operator to concat 2 tokens 
#define STRINGIFY(x) #x  //# is a preprocessor operator to stringify a token

#define ROUTE(method, path) ROUTE_HELPER(method, STRINGIFY(path), __COUNTER__)
#define ROUTE_HELPER(method, path_str, counter) \
    static void CONCAT(_handler_, counter)(const char*, int); \
    static void CONCAT(_register_, counter)(void) __attribute__((constructor)); \
    static void CONCAT(_register_, counter)(void) { \
        routes[route_count].http_method = #method; \
        routes[route_count].path = path_str; \
        routes[route_count].handler     = CONCAT(_handler_, counter); \
        route_count++; \
    } \
    static void CONCAT(_handler_, counter)(const char* request_body, int client_fd)


bool dispatch_route(Request* request, int client_fd);


#endif