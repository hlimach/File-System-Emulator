#include "headers/server.h"



void thFunc(int new_socket)
{
    char *hello;
	std::string command= "";
    int valread;
    std::cout<<"inthreaad"<<std::endl;

    while(1)
    {
        char buffer[1024] = {0};
	    valread = read(new_socket, buffer, 1024); 
    
        if(buffer[0] == 'e' && buffer[1] == 'n' && buffer[2] == 'd')
                break;
        
        command = buffer;
        std::cout << command << std::endl;
        send(new_socket, hello, strlen(hello), 0); 
        std::cout << "Hello message sent" << std::endl;
    
    }
    std::cout << new_socket << " user left" << std::endl;
        
    
}

int 
establishConn (struct sockaddr_in &address)
{
// Creating socket file descriptor 
    int server_fd;
    int opt = 1;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cout << "conn done1" << std::endl;
        return -1;
    }
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        std::cout << "conn done2" << std::endl;
        return -1;
    }

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if ( (::bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0){
        perror("bind failed");
        return -1;
    }
	if (listen(server_fd, 3) < 0){
        std::cout << "conn done4" << std::endl;
        return -1;
    }
    return server_fd;
}

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





