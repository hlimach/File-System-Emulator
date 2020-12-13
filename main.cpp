#include "config.h"
#include "util.h"
#include "memory_structure.h"
#include "globals.h"
#include "dat.h"
#include "file.h"


void
startProcess(int i)
{
	string path = "thread inputs/" + to_string(i+1) + ".txt";
	threadIn[i].open(path);
	bool loop = true;
	while (loop) 
	{
		vector<string> tokens = getCommand(threadIn[i]);
		loop = processCommand(tokens,threadIn[i]);
	}
}


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