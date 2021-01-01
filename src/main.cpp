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

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);
	
	// Creating socket file descriptor 
	int server_fd = establishConn(address);
    
    if(server_fd == -1)
    {
        cout << "conn failed" << endl;
        return 0;
    }

	cout << "Listening" << endl;
    
	int i = 0;

	while (1) {
		
        sockets.push_back(getSocket(address,server_fd));
		current.push_back(rootFolder);
		tempFolder.push_back(rootFolder);
		tempFile.push_back(new FileNode("empty"));
        threads.push_back(thread(startProcess,i));
		i++;
	}


	free((char*)start);
	return 0;
}