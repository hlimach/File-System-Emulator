#include "../include/server.h"

int 
establishConn (struct sockaddr_in &address)
{
    // Creating socket file descriptor 
    int server_fd;
    int opt = 1;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket failed");
        return -1;
    }
	
	// Forcefully attaching socket to the port 95
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        return -1;
    }

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if ((::bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        perror("bind failed");
        return -1;
    }

	if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        return -1;
    }

    return server_fd;
}


/* Accepts new connection from incoming client and returns the socket num. */
int
getSocket (struct sockaddr_in &address, int server_fd)
{
    int addrlen = sizeof(address); 
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
						(socklen_t*)&addrlen)) < 0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

    return new_socket;    
}