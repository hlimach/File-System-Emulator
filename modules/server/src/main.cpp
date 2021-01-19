#include "../include/config.h"
#include "../include/util.h"
#include "../include/mem_struct.h"
#include "../include/globals.h"
#include "../include/dat.h"
#include "../include/file.h"
#include "../include/threads.h"
#include "../include/server.h"

void 
signalHandler (int signum) 
{
	cout << " signal received." << endl << endl;
	cout << "Initiating server shut down protocol..." << endl;

	//sem_close(writer_sema);
	//sem_unlink("writer_sema");
	free((char*)start);

	cout << "Server shutdown protocol complete." << endl;

	exit(signum);	
}


int 
main (int argc, const char* argv[]) 
{
	// Attach signal handler to SIGINT for proper shutdown
	signal(SIGINT, signalHandler);  
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
	
	current.push_back(rootFolder);
	tempFolder.push_back(rootFolder);
	tempFile.push_back(new FileNode());
	filePosDir.push_back(-1);
	fileFound.push_back(false);
	found.push_back(false);

	readDat();
	
	current.erase(current.begin());
	tempFolder.erase(tempFolder.begin());
	tempFile.erase(tempFile.begin());
	filePosDir.erase(filePosDir.begin());
	fileFound.erase(fileFound.begin());
	found.erase(found.begin());

	cout << "Connection established! ready for users..." << endl;
	cout << "To shut down server, press CTRL+C (^C)" << endl << endl;
    
	int i = 0;

	/* Initialize variables for new connection and launches separate
	   Thread for this client. */
	while (1) 
	{
		// Pushing this socket, current and temp folder, temp file
        sockets.push_back(getSocket(address, server_fd));
        serverResponse.push_back("");
		current.push_back(rootFolder);
		tempFolder.push_back(rootFolder);
		tempFile.push_back(new FileNode());
		filePosDir.push_back(-1);
		fileFound.push_back(false);
		found.push_back(false);
		rootFolder->NumUsers++;

		// Pushing this thread into vector and starting communication
        threads.push_back(thread(startProcess, i));
		i++;
	}

	return 0;
}