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
	current = rootFolder;
	tempFolder = rootFolder;
	start = (char*)malloc(MEMSIZE);

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);

	printSpace();

	for(int i = 0; i < 1; i++)
		th[i] = thread(startProcess, i);

	for(int i = 0; i < 1; i++)
		th[i].join();

	free((char*)start);
	return 0;
}