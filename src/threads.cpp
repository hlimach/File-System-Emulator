#include "headers/config.h"
#include "headers/globals.h"
#include "headers/threads.h"
#include "headers/util.h"

void
startProcess(int i)
{

	bool loop = true;
	cout << "user " + to_string(i+1) + " added" << endl;
	try{
		while (loop) 
		{
			vector<string> tokens = getCommand(threadIn[i],i);
			loop = processCommand(tokens, threadIn[i],i);
		}
		cout << "user " + to_string(i+1) + " left" << endl;
	}
	catch (...) {
		cout << "user " + to_string(i+1) + " left" << endl;
	}
	//threadOut[i].close();
}