#include "config.h"
#include "globals.h"
#include "threads.h"
#include "util.h"

void
startProcess(int i)
{
	string path = "thread inputs/" + to_string(i + 1) + ".txt";
	threadIn[i].open(path);
	bool loop = true;
	while (loop) 
	{
		vector<string> tokens = getCommand(threadIn[i]);
		loop = processCommand(tokens, threadIn[i]);
	}
}