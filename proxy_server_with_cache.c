#include "proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_CLIENTS 10 

typedef struct cache_element cache_element ; 


// LRU policy -> using timestamp of cache element 
struct cache_element{
    char * data ;
    int len ; 
    char * url ; 
    time_t lru_time_track ; 
    cache_element * next ; 
};

cache_element * find(char * url) ; 

int add_cache_element(char * data , int size , char * url) ;

void remove_cache_element() ;

int port_number = 8080 ; 

int proxy_socket_id ; 

pthread_t tid[MAX_CLIENTS]  ;


// for every thread with 
sem_t semaphore ; 

pthread_mutex_t lock ; 

cache_element * head ; 
int cache_size ; 

int main(int argc , char * argv[]){
    int client_socket_id ;
    int client_len ; 
    struct sockaddr server_addr , client_addr;
    sem_init(&semaphore , 0 , MAX_CLIENTS) ;
    pthread_mutex_init(&lock , NULL) ; 

    if(argv == 2){
        // ./proxy 3000 
        // extract the port number from input 
        port_number = atoi(argv[1]); 
    }
    else{
        printf("Too few arguments\n") ; 
        exit(1) ; 
    }

    printf("Starting proxy server at port : %d\n" , port_number) ; 
    proxy_socket_id = socket(AF_INET , SOCK_STREAM , 0) ; 

    if(proxy_socket_id < 0){
        perror("Failed to create socket for proxy server\n") ;
        exit(1) ; 
    }
    // Allows multiple sockets to bind to the same port.
    int reuse = 1; 
    if(setsockopt(proxy_socket_id , SOL_SOCKET , SO_REUSEADDR , (const char *)&reuse , sizeof(reuse) < 0){
        perror("SetSockOpt failed\n")  ;
        exit(1) ; 
    }
    return 0 ; 
}
