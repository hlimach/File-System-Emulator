#include <stdio.h> 
#include <stdlib.h> 
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <thread>
#define PORT 95


/* Gets the input string and returns it as a vector of strings, tokenized by the 
   Delimiter assigned. */
bool
checkIP (string ipAddr) 
{
	string s;
	stringstream ss(ipAddr);
	vector<string> nums;

	while (getline(ss, s, '.')) 
	{
		if (isdigit(s) == false)
			return false;

		nums.push_back(s);
	}

	if (nums.size() != 4)
		return false;
	else 
		return true;
}

char *
convertMessage (std::string message)
{
    char *msg;
    strcpy(msg,message.c_str());
    return msg;
}


int 
main() 
{ 
	// Obtain username and ip address from client
	std::string user, ipAddr;
	bool isIP;

	std::cout << "Welcome! "; 
	std::cout << "Please enter username: (20 or less characters, cannot contain spaces)"
		 << std::endl;
	std::cin >> user;
	
	do
	{
		std::cout << "Please specify ip address:" << std::endl;
		std::cin >> ipAddr;
		isIP = checkIP(ipAddr);

		if (isIP == false)
			std::cout << "Invalid IP provided." << std::endl;
	} while (isIP == false);

	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *msgPtr; 
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		perror("socket failed");
		exit(EXIT_FAILURE); 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, ipAddr, &serv_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported" << std::endl;
		exit(EXIT_FAILURE); 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		std::cout << "\nConnection Failed" << std::endl;
		exit(EXIT_FAILURE); 
	}

	// First message sent to server is the username
	msgPtr = convertMessage(user);
	send(sock, msgPtr, strlen(msgPtr), 0); 

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

    std::cout << "leaving bye" << std::endl;
} 
