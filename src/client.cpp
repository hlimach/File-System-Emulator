#include <stdio.h> 
#include <stdlib.h> 
#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <thread>
#include <vector>
#include <sstream>
#include <ctype.h>

#define PORT 95


/* Gets the input string and returns it as a vector of strings, tokenized by the 
   Delimiter assigned. */
bool
checkIP (std::string ipAddr) 
{
	std::string s;
	std::stringstream ss(ipAddr);
	std::vector<std::string> nums;

	while (getline(ss, s, '.')) 
	{
		nums.push_back(s);
		
		for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;
	}

	if (nums.size() != 4)
		return false;
	else 
		return true;
}

/* Converts message into char pointer. */
char*
convertMessage (std::string message, int length)
{
    char* msg = (char*) malloc(length);
    strcpy(msg, message.c_str());
    return msg;
}

/* Used to write into an opened file. */
std::string
writeFile()
{
	std::string input = "", line;
	while (std::getline(std::cin>>std::ws, line)) 
	{
		if (line == "-1")
			break;

		input += line;
		input += "\n";
	}
	input = input.substr(0, input.size() - 1);
	return input;
}


int 
main() 
{ 
	// Obtain username and ip address from client
	std::string user = "";
	std::cout << "Welcome! ";
	std::cout << "Please enter username: (20 or less characters, cannot contain spaces)"
		 << std::endl;
	std::cin >> user;
	
	std::string ipAddr;
	
	bool isIP = true;
	
	do
	{
		std::cout << "Please specify ip address:" << std::endl;
		std::cin >> ipAddr;
		isIP = checkIP(ipAddr);
		
		if (isIP == false)
			std::cout << "Invalid IP provided." << std::endl;

	} while (isIP == false);

	std::cout << "Sending over IP " << ipAddr << std::endl;
	char* ipPtr = convertMessage(ipAddr, ipAddr.size());

	int sock = 0; 
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
	if(inet_pton(AF_INET, ipPtr, &serv_addr.sin_addr) <= 0) 
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
	msgPtr = convertMessage(user, user.size());
	send(sock, msgPtr, strlen(msgPtr), 0); 

    std::string msg = "";
    while(1)
    {
        char buffer[16384] = {0};
	    msg = "";

        if(read(sock, buffer, 16384) == 0)
        {
        	std::cout << "Server unresponsive. Please try again later." << std::endl;
        	break;
        }

        if (buffer[0] == '%' && buffer[1] == '^')
        	msg = writeFile();
        
        else
        {
	    	std::cout << buffer << " ";
	    	std::getline(std::cin >> std::ws, msg);
        }
        
        msgPtr = convertMessage(msg, msg.size());
	    send(sock, msgPtr, strlen(msgPtr), 0);
        
        if(msg == "end")
            break;
    }
} 
