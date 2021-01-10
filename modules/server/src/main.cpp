#include "../include/config.h"
#include "../include/util.h"
#include "../include/mem_struct.h"
#include "../include/globals.h"
#include "../include/dat.h"
#include "../include/file.h"
#include "../include/threads.h"
#include "../include/server.h"


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
		filePosDir.push_back(-1);
		fileFound.push_back(false);
		found.push_back(false);

		// Pushing this thread into vector and starting communication
        threads.push_back(thread(startProcess, i));
		i++;
	}

	cout << " 100" << endl;
	//free((char*)start);
	return 0;
}