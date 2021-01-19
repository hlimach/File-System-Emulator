#include "../include/config.h"
#include "../include/globals.h"
#include "../include/threads.h"
#include "../include/util.h"


/* Keeps looping for getting and processing commands until the 
   program is terminated. */
void
startProcess (int i)
{
	bool loop = true;
	int valread;
	string user;
	char buffer[20] = {0};
	
	valread = read(sockets[i], buffer, 20);
	user = buffer; 

	users.push_back(user);
	cout << "User " + user + " added." << endl;

	try
	{
		while (loop) 
		{
			vector<string> tokens = getCommand(i);
			loop = processCommand(tokens, i);
		}
		cout << "user " + user + " left" << endl;
	}
	catch (exception &exc) 
	{
		std::cerr << exc.what();
	}
}