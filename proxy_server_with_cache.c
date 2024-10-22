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

#define MAX_BYTES 4096    //max allowed size of request/response


typedef struct cache_element cache_element;

// LRU policy -> using timestamp of cache element
struct cache_element
{
    char *data;
    int len;
    char *url;
    time_t lru_time_track;
    cache_element *next;
};

cache_element *find(char *url);

int add_cache_element(char *data, int size, char *url);

void remove_cache_element();

int port_number = 8080;

int proxy_socket_id;

pthread_t tid[MAX_CLIENTS];

// for every thread with
sem_t semaphore;

pthread_mutex_t lock;

cache_element *head;
int cache_size;


void thread_fn(void * socketNew){
    sem_wait(&semaphore); 
	int p;
	sem_getvalue(&semaphore,&p);
	printf("semaphore value:%d\n",p);

    int* t= (int*)(socketNew);
	int socket=*t;           // Socket is socket descriptor of the connected Client
	int bytes_sent_from_client,len;	  // Bytes Transferred

	
	char *buffer = (char*)calloc(MAX_BYTES,sizeof(char));	// Creating buffer of 4kb for a client
	
	
	bzero(buffer, MAX_BYTES);								// Making buffer zero
	bytes_sent_from_client = recv(socket, buffer, MAX_BYTES, 0); // Receiving the Request of client by proxy server
	
	while(bytes_sent_from_client > 0)
	{
		len = strlen(buffer);
        //loop until u find "\r\n\r\n" in the buffer , it is the EOF ( End of file ) for buffer
        // Loop until you reach end of buffer 
		if(strstr(buffer, "\r\n\r\n") == NULL)
		{	
			bytes_sent_from_client = recv(socket, buffer + len, MAX_BYTES - len, 0);
		}
		else{
			break;
		}
	}
}
int main(int argc, char *argv[])
{
    int client_socket_id;
    int client_len;
    struct sockaddr_in server_addr, client_addr;
    sem_init(&semaphore, 0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);

    if (argv == 2)
    {
        // ./proxy 3000
        // extract the port number from input
        port_number = atoi(argv[1]);
    }
    else
    {
        printf("Too few arguments\n");
        exit(1);
    }

    printf("Starting proxy server at port : %d\n", port_number);
    proxy_socket_id = socket(AF_INET, SOCK_STREAM, 0);

    if (proxy_socket_id < 0)
    {
        perror("Failed to create socket for proxy server\n");
        exit(1);
    }
    // Allows multiple sockets to bind to the same port. (make it reusable)
    int reuse = 1;
    if (setsockopt(proxy_socket_id, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        perror("SetSockOpt failed\n");
        exit(1);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    // bzero set a block of memory to zero
    // can be replaced with memset((char*)&server_addr , 0 , sizeof(server_addr))

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number); // Assigning port to the Proxy
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Any available address assigned

    // Binding the socket
    if (bind(proxy_socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Port is not free\n");
        exit(1);
    }
    printf("Binding on port: %d\n", port_number);

    // Proxy socket listening to the requests
    int listen_status = listen(proxy_socket_id, MAX_CLIENTS);

    if (listen_status < 0)
    {
        perror("Error while Listening !\n");
        exit(1);
    }


	int i = 0; // Iterator for thread_id (tid) and Accepted Client_Socket for each thread
	int Connected_socketId[MAX_CLIENTS];   // This array stores socket descriptors of connected clients

    // Infinite Loop for accepting connections
    while (1)
    {

        bzero((char *)&client_addr, sizeof(client_addr)); // Clears struct client_addr
        client_len = sizeof(client_addr);

        // Accepting the connections
        client_socket_id = accept(proxy_socket_id, (struct sockaddr *)&client_addr, (socklen_t *)&client_len); // Accepts connection
        if (client_socket_id < 0)
        {
            fprintf(stderr, "Error in Accepting connection !\n");
            exit(1);
        }
        else
        {
            Connected_socketId[i] = client_socket_id; // Storing accepted client into array
        }

        // Getting IP address and port number of client
        struct sockaddr_in *client_pt = (struct sockaddr_in *)&client_addr; // creating a copy
        struct in_addr ip_addr = client_pt->sin_addr; // extract ip address

        char str[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN: Default ip address size
        inet_ntop(AF_INET, &ip_addr, str, INET_ADDRSTRLEN);// store the ip address in str char array 
        // we are doing this just to print it 

        printf("Client is connected with port number: %d and ip address: %s \n", ntohs(client_addr.sin_port), str);


        // printf("Socket values of index %d in main function is %d\n",i, client_socketId);
        pthread_create(&tid[i], NULL, thread_fn, (void *)&Connected_socketId[i]); // Creating a thread for each client accepted
        i++;
    }
    close(proxy_socket_id);
    return 0;
}
