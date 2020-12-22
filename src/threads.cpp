#include "headers/config.h"
#include "headers/globals.h"
#include "headers/threads.h"
#include "headers/util.h"

void
startProcess(int i)
{
	string pathIn = "../threads/" + to_string(i + 1) + "in.txt";
	string pathOut = "../threads/" + to_string(i + 1) + "out.txt";
	threadIn[i].open(pathIn);
	threadOut[i].open(pathOut);
	bool loop = true;
	while (loop) 
	{
		vector<string> tokens = getCommand(threadIn[i],i);
		loop = processCommand(tokens, threadIn[i],i);
	}
	
	//threadOut[i].close();
}