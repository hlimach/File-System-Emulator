#include "headers/config.h"
#include "headers/globals.h"
#include "headers/threads.h"
#include "headers/util.h"

void
startProcess(int i)
{
	string path = "../threads/" + to_string(i + 1) + ".txt";
	threadIn[i].open(path);
	bool loop = true;
	while (loop) 
	{
		vector<string> tokens = getCommand(threadIn[i],i);
		loop = processCommand(tokens, threadIn[i],i);
	}
}