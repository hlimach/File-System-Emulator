#include "headers/config.h"
#include "headers/util.h"
#include "headers/memory_structure.h"
#include "headers/globals.h"
#include "headers/dat.h"
#include "headers/file.h"
#include "headers/threads.h"


int 
main (int argc, const char* argv[]) 
{
	int numberOfThreads;
	current = rootFolder;
	tempFolder = rootFolder;
	start = (char*)malloc(MEMSIZE);

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);

	printSpace();

	cout<<"Enter number of users (1-10): ";
	cin >> numberOfThreads;
	for(int i = 0; i < numberOfThreads; i++)
		th[i] = thread(startProcess, i);

	for(int i = 0; i < numberOfThreads; i++)
		th[i].join();

	free((char*)start);
	return 0;
}