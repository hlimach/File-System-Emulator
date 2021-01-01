#include <stdio.h> 
#include <stdlib.h> 
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <thread>
#define PORT 95

char *
convertMessage (std::string message)
{
    char *msg;
    strcpy(msg,message.c_str());
    return msg;
}

void thFunc() {
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *msgPtr; 
	 

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		std::cout << "\n Socket creation error" << std::endl;
		exit(EXIT_FAILURE); 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported" << std::endl;
		exit(EXIT_FAILURE); 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		std::cout << "\nConnection Failed" << std::endl;
		exit(EXIT_FAILURE); 
	}
    std::string msg = "";
    while(1)
    {
        char buffer[1024] = {0};
	    msg = "";
        
        
        valread = read(sock, buffer, 1024); 
	    std::cout << buffer << " ";
	    
        std::getline(std::cin,msg);
        msgPtr = convertMessage(msg);
	    send(sock, msgPtr, strlen(msgPtr), 0); 
        
        if(msg == "end")
            break;
	   
    }
    std::cout<<"leaving bye"<<std::endl;
}

int main() 
{ 
	thFunc();
} 
