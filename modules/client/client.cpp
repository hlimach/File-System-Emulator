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
#include <chrono>

#define PORT 95
#define BUFFER 64


/* Converts message into char pointer. */
char*
convertMessage (std::string message, int length)
{
    char* msg = (char*) malloc(length + 1);
    strcpy(msg, message.c_str());
    return msg;
}


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


/* Gets the input string and returns it as a vector of strings, tokenized by the 
   Delimiter assigned. */
void
sendMessage (int sock, std::string msg) 
{
	char *msgPtr = convertMessage(msg, msg.size());
	send(sock, msgPtr, strlen(msgPtr), 0);
	free(msgPtr);
}


/* Gets the input string and returns it as a vector of strings, tokenized by the 
   Delimiter assigned. */
int
connectToServer (std::string ipAddr) 
{
	char* ipPtr = convertMessage(ipAddr, ipAddr.size());

	int sock = 0; 
	struct sockaddr_in serv_addr; 
	
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

	return sock;
}


/* Used to write into an opened file. */
std::string
writeFile()
{
	std::string input = "", line = "";
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


/* Prompt user to enter IP address */
void
getIPAddr(std::string &ipAddr)
{
	bool isIP = true;
	// Prompt for IP Address until a proper one is input
	do
	{
		std::cout << "Please specify ip address:" << std::endl;
		std::cin >> ipAddr;
		isIP = checkIP(ipAddr);
		
		if (isIP == false)
			std::cout << "Invalid IP provided." << std::endl;

	} while (isIP == false);
}


int 
main() 
{ 
	// Obtain username and ip address from client
	std::string user = "";
	std::cout << "Welcome!" << std::endl;
	std::cout << "Enter username: (20 characters or less, cannot contain spaces)"
		 << std::endl;
	std::cin >> user;
	
	std::string ipAddr;
	bool fileOpened = false;
	getIPAddr(ipAddr);

	std::cout << "Sending over IP " << ipAddr << std::endl;
	int sock = connectToServer(ipAddr);

	// First message sent to server is the username
	sendMessage(sock, user);

	// Buffer created for response receival
    char buffer[BUFFER] = {0};
    std::string msg = "", response = "";

    while(1)
    {
        bzero(buffer, BUFFER);
	    msg = "";
	    response = "";

	    // If response exceeds buffer size, it is received in chunks (packets)
	    do 
		{
			bzero(buffer, BUFFER);
	        if (read(sock, buffer, BUFFER) <= 0)
	        {
	        	std::cout << "Server unresponsive. Please try again later." << std::endl;
	        	return -1;
	        }
	        else
	        {
	        	std::string check = buffer;
	        	if (check.size() > BUFFER)
	        		check = check.substr(0, BUFFER);
	        	response += check;
	        }
        } while (response.substr(response.size() - 2, 2) != "#$");

		// set flag to true when file is opened and appropriate response is recieved
		if(fileOpened == false && response.substr(0, 11) == "File opened")
			fileOpened = true;
		
		// set flag to false when file is closed and appropriate response is recieved
		if(fileOpened == true && response.substr(0, 13) == "File closed.\n")
			fileOpened = false;	

		// In case server is shut down, the client instance is also terminated
		// if(buffer[0] == '~' && buffer[1] == '~')
		// {
		// 	std::cout << "Server Down." << std::endl;
		// 	std::cout << "Please try again later." << std::endl;
		// 	break;
		// }

        // Server returns %^ for for file writing function permissions
        if (buffer[0] == '%' && buffer[1] == '^')
        	msg = writeFile();
        else
        {
	    	std::cout << response.substr(0, response.size() - 2) << " ";
	    	std::getline(std::cin >> std::ws, msg);
        }
        
        // The sending msg is appended with ending #$
        msg += "#$";
        int msgSize = msg.size(), itr = 1;
        std::string chunk;
        do
        {
        	// Message broken up into packets if size larger than buffer
        	if (msgSize > BUFFER)
	        {
	        	chunk = msg.substr(BUFFER * (itr - 1), BUFFER);
	        	itr++;
	        	msgSize -= BUFFER;
	        	sendMessage(sock, chunk);
	        }
	        else
	        {
	        	chunk = msg.substr(BUFFER * (itr - 1), msgSize);
	        	sendMessage(sock, chunk);
	        	msgSize = 0;
	        }

        } while (msgSize != 0);
        
        // If message sent is end, it means client wants to terminate
        if (msg == "end#$" && fileOpened == false)
            break;

        // Sleep after sending so that incoming packets are not lost
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
} 
