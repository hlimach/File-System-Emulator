#include "headers/config.h"
#include "headers/util.h"
#include "headers/mem_struct.h"
#include "headers/globals.h"
#include "headers/dat.h"
#include "headers/file.h"
#include "headers/threads.h"
#include "headers/server.h"


int 
main (int argc, const char* argv[]) 
{
	int new_socket; 
	struct sockaddr_in address; 
	start = (char*) malloc(MEMSIZE);

	// Generate free list at start of program
	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);
	
	// Establishing connection with socket
	cout << "Establishing connection ... " << endl; 
	int server_fd = establishConn(address);
    
    // In case there is error in connection establishment
    if (server_fd == -1)
    {
        cout << "conn failed" << endl;
        return 0;
    }

	cout << "Connection established! ready for users..." << endl;
    
	int i = 0;

	/* Initialize variables for new connection and launches separate
	   Thread for this client. */
	while (1) 
	{
		// Pushing this socket, current and temp folder, temp file
        sockets.push_back(getSocket(address, server_fd));
		current.push_back(rootFolder);
		tempFolder.push_back(rootFolder);
		tempFile.push_back(new FileNode());

		// Pushing this thread into vector and starting communication
        threads.push_back(thread(startProcess, i));
		i++;
	}


	free((char*)start);
	return 0;
}