#include "../include/config.h"
#include "../include/util.h"
#include "../include/globals.h"
#include "../include/file.h"
#include "../include/sema.h"
#include "../include/dat.h"

/* user guide. */
void 
help (int threadNo) 
{
	serverResponse[threadNo] += "-------------------------------------------User Guide-------------------------------------------\n";
	serverResponse[threadNo] += "Command\t\tDescription\t\t\t\t\t\t\tSyntax\n";
	serverResponse[threadNo] += "mkdir\t\tMake new directory in given path\t\t\t\tmkdir ../folder/subf/DirName\n";
	serverResponse[threadNo] += "ls\t\tList files in current directory\t\t\t\t\tls\n";
	serverResponse[threadNo] += "cd\t\tChange directory to given path\t\t\t\t\tcd ../folder/subf/\n";
	serverResponse[threadNo] += "cr\t\tCreate a new file at current working directory\t\t\tcr foo\n";
	serverResponse[threadNo] += "open\t\tOpen a specific file at current working directory\t\topen foo read|write\n";

	serverResponse[threadNo] += "wr\t\tWrite by appending to opened file\t\t\t\twr\n";
	serverResponse[threadNo] += "wrat\t\tWrite starting from given byte in opened file\t\t\twrat 207\n";
	serverResponse[threadNo] += "trun\t\tReduce opened file to given size\t\t\t\ttrun 1280\n";
	serverResponse[threadNo] += "mvin\t\tMove from 'start' byte of 'size' to byte 'to' (ints)\t\tmvin start size to\n";
	serverResponse[threadNo] += "rf\t\tReads from given start upto given number of characters\t\trf 312 50\n";
	serverResponse[threadNo] += "rd\t\tReturns entire content of opened file\t\t\t\trd\n";
	serverResponse[threadNo] += "chmod\t\tChanges mode of opened file\t\t\t\t\tchmod read|write\n";


	serverResponse[threadNo] += "close\t\tClose the opened file\t\t\t\t\t\tclose\n";
	serverResponse[threadNo] += "del\t\tDelete a file at the specified path\t\t\t\tdel ./folder/foo\n";
	serverResponse[threadNo] += "rmdir\t\tDelete all files and folders in specified folder\t\trem folder\n";
	serverResponse[threadNo] += "mv\t\tMove file from one location to another\t\t\t\tmv ./subf/filename ../sf/\n";
	serverResponse[threadNo] += "map\t\tDisplay memory map\t\t\t\t\t\tmap\n";
	serverResponse[threadNo] += "end\t\tTerminate program\t\t\t\t\t\ttend\n";
	serverResponse[threadNo] += "rdat\t\tRead and existing .dat file generated using this program\trdat\n";
}


/* Gets the input string and returns it as a vector of strings, tokenized by the 
   Delimiter assigned. */
vector<string> 
tokenize (string command, char delimiter) 
{
	string s;
	stringstream ss(command);
	vector<string> tokens;

	while (getline(ss, s, delimiter)) 
	{
		/* In case the delimiter is entered more than once, it introduces empty 
		   Strings into the vector. To get rid of these, whenever these are obtained 
		   The loop continues onto next iteration. */
		if (s == "")
			continue;

		tokens.push_back(s);
	}
	return tokens;
}



/* converts given string into an array of character */
char *
convertMessage (string message, int length)
{
	char * msg = (char *) malloc(length + 1);
    strcpy(msg,message.c_str());
	return msg;
}



/* Prints number of bytes available by checking free list*/
void
printSpace(int threadNo)
{
	serverResponse[threadNo] += "Memory available: " + to_string(freeList.size() * PAGESIZE) + "/" + 
			to_string(MEMSIZE) + " bytes\n";
}


/* Gets a char pointer to a page and returns the page number that it is supposed to 
   Be according to starting pointer. */
int 
getPageNum (char* page) 
{
	return (page - start) / PAGESIZE;
}


/* Returns string path from root upto the current working directory. */
string 
pathFromRoot (Folder* dir) 
{
	string path = dir->dirName;
	while (dir->parent != NULL) 
	{
		path = dir->parent->dirName + "/" + path;
		dir = dir->parent;
	}
	return path;
}


/* Takes in the files' name as an argument and tells which index it will be found at. 
   If the file does not exist in the current directory, it returns -1 (error). */
int 
getFileNo (string name, int threadNo) 
{
	int i;
	bool found = false;

	for (i = 0; i < current[threadNo]->files.size(); i++) 
	{
		if (current[threadNo]->files[i]->name == name) 
		{
			found = true;
			break;
		}
	}

	if (found)
		return i;
	else
		return -1;
}


/* pushes the given argument in the freeList and making it mutually exclusive*/
void
pushStack(short int top)
{
	stackMtx.lock();
	freeList.push(top);
	stackMtx.unlock();
}


/* return the top of the freeList and making it mutually exclusive*/
short int
popStack()
{
	stackMtx.lock();
	short int temp = freeList.top();
	freeList.pop();
	stackMtx.unlock();
	return temp;
}


/* Function takes in the wanted files' name as argument accesses the temporary folders'
   File directory and iterates it fully returns true if it is found. */
bool 
fileExists (string filename, int threadNo) 
{
	for (int j = 0; j < tempFolder[threadNo]->files.size(); j++) 
	{
		if (tempFolder[threadNo]->files[j]->name == filename) 
		{
			filePosDir[threadNo] = j;
			tempFile[threadNo] = tempFolder[threadNo]->files[j];
			return true;
		}
	}
	return false;
}


/* Function folderExists takes in directory name as argument accesses the temporary 
   Folder's subdirectory and iterates it fully returns true if it is found. */
bool 
folderExists (string dirName, int threadNo, bool change) 
{
	for (int j = 0; j < tempFolder[threadNo]->subdir.size(); j++) 
	{
		if (tempFolder[threadNo]->subdir[j]->dirName == dirName) 
		{
			tempFolder[threadNo] = tempFolder[threadNo]->subdir[j];
			if(change)
				tempFolder[threadNo]->NumUsers++;
			return true;
		}
	}
	return false;
}


/* Lists all the files and folders in the current working directory. */
void 
listDir (int threadNo) 
{
	if (current[threadNo]->subdir.size() == 0 && current[threadNo]->files.size() == 0) 
	{
		serverResponse[threadNo] += "Directory is empty.\n";
		return;
	}
	else 
	{
		for (int i = 0; i < current[threadNo]->subdir.size(); i++)
			serverResponse[threadNo] += current[threadNo]->subdir[i]->dirName + "\t";

		for (int i = 0; i < current[threadNo]->files.size(); i++)
			serverResponse[threadNo] += current[threadNo]->files[i]->name + "\t";

		serverResponse[threadNo] += "\n";
	}
}


/* Checks if the given string is a number. */
bool 
isNumber (string s) 
{
	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;
	return true;
}


/* The function locateFile takes in tokenized path and a check if working is for
   Destination file. Iraverses the path to find specified file in tokens. If the path
   Exists, the fucntion updates pointers to tempFile.cIf the path exists, file does
   Not exist, and its a destination file, the function creates the file. */
void 
locateFile (vector<string> tokens, bool destFile, int threadNo) 
{
	found[threadNo] = true;
	tempFolder[threadNo] = current[threadNo];
	int i = 0;

	/* If the first token is '.', then it works in current directory. */
	if (tokens[0] == ".")
		i = 1;

	for (; i < tokens.size(); i++) 
	{
		if (tokens[i] == "..") 
		{
			if (tempFolder[threadNo]->parent == NULL) 
			{
				serverResponse[threadNo] += "Parent of root does not exist.\n";
				found[threadNo] = false;
				return;
			}
			tempFolder[threadNo] = tempFolder[threadNo]->parent;
		}
		else 
		{
			/* If the current token isnt the last one (file name), then check if folder
			   Exists. If it does not, then the path given is invalid, so function is
			   Exited. */
			if (i != tokens.size() - 1) 
			{
				bool checkFolder = folderExists(tokens[i],threadNo,false);
				if (!checkFolder) 
				{
					serverResponse[threadNo] += "Invalid path. A folder in the path does not exist.\n";
					found[threadNo] = false;
					return;
				}
			}
			/* If the current token is the last one i.e. file name, then check if file
			   Exists. If it does not, then check if working with destination file, in
			   Which case, make the file at the specified location. */
			else 
			{
				/* In case it exists. */
				if (fileExists(tokens[i],threadNo)) 
				{
					found[threadNo] = true;
					return;
				}
				/* In case it is a destination file and it does not exist yet. */
				else if (destFile) {
					serverResponse[threadNo] += "Creating destination file...\n";
					serverResponse[threadNo] += "New file created.\n";
					tempFile[threadNo] = NULL;
					found[threadNo] = true;
					return;
				}
				/* In case it is a source file and it does not exist. */
				else if (!destFile) {
					serverResponse[threadNo] += "The specified file does not exist.\n";
					found[threadNo] = false;
					return;
				}
			}
		}
	}
}


/* Lists all files in current directory along with their information. */
void 
listFiles (Folder* dir, int threadNo) 
{
	if (dir->files.size() == 0)
		return;

	short int* pgTbl;
	string name, pgtbles="", pgnums = "", limit, disp = "";

	for (int i = 0; i < dir->files.size(); i++) 
	{
		name = dir->files[i]->name;
		disp += "{\n\t";
		pgnums = "";
		pgtbles = "";

		if (dir->files[i]->pgTblPtr != NULL) 
		{
			File openFile(dir->files[i]->name, "read", false,threadNo);
            pgTbl = dir->files[i]->pgTblPtr;
            int nextPageTableNum = getPageNum((char *) pgTbl);

            do
            {
                pgTbl = (short int*) openFile.getPagePtr(nextPageTableNum);
				pgtbles += to_string(getPageNum((char *)pgTbl)) + ", ";
                openFile.setPageTablePtr(pgTbl);

                for (int i = 2; i <= openFile.getPageCount() + 1; i++)
                    pgnums += to_string(*(pgTbl + i)) + ", ";

                nextPageTableNum = openFile.getNextPageTableNum();

            } while (nextPageTableNum != -1);

            pgnums = pgnums.substr(0, pgnums.size() - 2);
            pgtbles = pgtbles.substr(0, pgtbles.size() - 2);
            limit = to_string(openFile.getByteLimit());

            disp += "Name: " + name + "\n\tPage numbers: " + pgnums + 
				"\n\tPage Tables: " + pgtbles + "\n\tLimit on last page: "
            	 + limit + "\n\tTotal file size: " + 
            	 to_string(openFile.getFileSize()) + "\n\tPath: " + pathFromRoot(dir) 
            	 + "\n}\n";
		}
		else 
		{
			disp += "Name: " + name + "\n\tPage numers: empty\n\tLimit on last page: "
				 + "-\n\tTotal file size: 0\n\tPath: " + pathFromRoot(dir) + "\n}\n";
		}	
	}

	serverResponse[threadNo] += disp;
}


/* Recursive function for memory map. Lists all files in a directory, then is called
   On each subdirectory. */
void 
memMap (Folder* dir, int threadNo) 
{	
	tempFolder[threadNo] = dir;
	current[threadNo] = dir;
	listFiles(dir, threadNo);

	if (dir->subdir.size() == 0)
		return;

	for (int i = 0; i < dir->subdir.size(); i++)
		memMap(dir->subdir[i], threadNo);
}


/* Returns the response of the command to the client. */
void
sendResponse (int threadNo) 
{
	char *msgPtr;
	serverResponse[threadNo] += users[threadNo] + "$ " + pathFromRoot(current[threadNo]) + ">#$";
    int resSize = serverResponse[threadNo].size(), itr = 1;
    string chunk;

    // If response larger than buffer, it is sent in packets (chunks)
	do
	{
		if (resSize > BUFFER)
		{
			chunk = serverResponse[threadNo].substr(BUFFER * (itr - 1), BUFFER);
			itr++;
			resSize -= BUFFER;
			msgPtr = convertMessage(chunk, BUFFER);
			send(sockets[threadNo], msgPtr, strlen(msgPtr), 0);
		}
		else
		{
			chunk = serverResponse[threadNo].substr(BUFFER * (itr - 1), resSize);
			msgPtr = convertMessage(chunk, chunk.size());
			send(sockets[threadNo], msgPtr, strlen(msgPtr), 0);
			resSize = 0;
		}
	} while (resSize != 0);
		
    serverResponse[threadNo] = "";
	free(msgPtr);
	return;
}


/* Prompts user to enter in their command, and once it is taken, it is tokenized based
   On spaces and the vector of resulting strings is returned. */
vector<string> 
getCommand (int threadNo) 
{
	// Response of prev command sent
	sendResponse(threadNo);

	// Buffer created for receiving commands
	char buffer[BUFFER] = {0};
	string command = "";

	int valread;

	// If command received is larger than buffer, it is sent in packets (chunks)
	do 
	{
		bzero(buffer, BUFFER);
		valread = read(sockets[threadNo], buffer, BUFFER); 
		if (valread == 0)
			command = "\a\a\a#$";
		else
			command += buffer;
		
	} while (command.substr(command.size() - 2, 2) != "#$");
	
	command = command.substr(0, command.size() - 2);
	cout << users[threadNo] << ": " << command << endl;
	return tokenize(command, ' ');
}


void
invalidCmdMsg(int threadNo)
{
	serverResponse[threadNo] += "Invalid command. Type help for user guide.\n";
}


int 
getCmdIndex (vector<string> cmds, string cmd)
{
	int i;
	for (i = 0; i < cmds.size(); i++)
	{
		if (cmds[i] == cmd)
			return i;
	}
	return i;
}


void
fileCmds1Call(int index, vector<string> tokens, int threadNo, bool &loop)
{
	switch (index)
	{
		/* wr */
		case 0:
			openedFiles.write(openedFiles.getInput(threadNo), true);
			break;

		/* rd */
		case 1:
			serverResponse[threadNo] += 
				openedFiles.read(0, openedFiles.getFileSize()) + "\n";
			break;

		/* help */
		case 2: 
			help(threadNo);
			break;

		/* close */
		case 3:
			serverResponse[threadNo] += "File closed.\n";
			leaveFile(tempFile[threadNo], openedFiles.mode);
			loop = false;
			break;

		/* end */
		case 4: 
			serverResponse[threadNo] += "Close file before ending program.\n";
			break;

		/* \a\a\a */
		case 5: 
			leaveFile(tempFile[threadNo], openedFiles.mode);
			loop = false;
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}
}


void
fileCmds2Call(int index, vector<string> tokens, int threadNo)
{
	switch (index)
	{
		/* wrat */
		case 0:
			if (isNumber(tokens[1]))
				openedFiles.writeAt(openedFiles.getInput(threadNo), 
					stoi(tokens[1]) - 1);
			else
				invalidCmdMsg(threadNo);
			break;

		/* chmod */
		case 1:
			openedFiles.changeMode(tokens[1]);
			break;

		/* trun */
		case 2: 
			if (isNumber(tokens[1]))
				openedFiles.truncate(stoi(tokens[1]));
			else
				invalidCmdMsg(threadNo);
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}
}


void
fileCmdProcessing(vector<string> tokens, int threadNo)
{
	enterFile(tempFile[threadNo], threadNo, tokens[2]);

	openedFiles = File(tokens[1], tokens[2], true, threadNo);
	bool inLoop = true;

	vector<string> tokens1 = {"wr", "rd", "help", "close", "end", "\a\a\a"};
	vector<string> tokens2 = {"wrat", "chmod", "trun"};
	vector<string> tokens3 = {"rf"};
	vector<string> tokens4 = {"mvin"};
	
	while (inLoop) 
	{
		vector<string> tokens = getCommand(threadNo);
		int cmdLength = tokens.size();
		int index;

		switch (cmdLength)
		{
			case 1:
				index = getCmdIndex(tokens1, tokens[0]);
				fileCmds1Call(index, tokens, threadNo, inLoop);
				break;

			case 2:
				index = getCmdIndex(tokens2, tokens[0]);
				fileCmds2Call(index, tokens, threadNo);
				break;

			case 3:
				index = getCmdIndex(tokens3, tokens[0]);

				if(index == 0 && isNumber(tokens[1]) && isNumber(tokens[2]))
					serverResponse[threadNo] += openedFiles.read(stoi(tokens[1]) - 1, 
						stoi(tokens[2])) + "\n";
				else
					invalidCmdMsg(threadNo);

				break;

			case 4:
				index = getCmdIndex(tokens4, tokens[0]);

				if(index == 0 && isNumber(tokens[1]) && isNumber(tokens[2]) 
					&& isNumber(tokens[3]))
						openedFiles.moveWithin(stoi(tokens[1]), stoi(tokens[2]), 
							stoi(tokens[3]));
				else
					invalidCmdMsg(threadNo);

				break;

			default:
				invalidCmdMsg(threadNo);
				break;
		}
	}
	filePosDir[threadNo] = -1;
}


void
cmds1Call (int index, vector<string> tokens, int threadNo, bool &loop)
{
	Folder *currentFolder;
	switch (index)
	{
		/* ls */
		case 0:
			listDir(threadNo);
			break;

		/* map */
		case 1:
			printSpace(threadNo);
			currentFolder = current[threadNo];
			memMap(rootFolder, threadNo);
			current[threadNo] = currentFolder;
			break;

		/* help */
		case 2: 
			help(threadNo);
			break;

		/* rdat */
		case 3:
			serverResponse[threadNo] += "reading .dat file ...\n";
			readDat();
			serverResponse[threadNo] += "Complete\n";
			printSpace(threadNo);
			break;

		/* end */
		case 4: 
			loop = false;
			break;

		/* \a\a\a */
		case 5: 
			loop = false;
			cout << "user " + users[threadNo] + " crashed!" << endl;
			serverResponse[threadNo] = "";
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}
}


void
cmds2Call(int index, vector<string> tokens, int threadNo)
{
	switch (index)
	{
		/* cd */
		case 0:
			changeDir(tokens[1], threadNo);
			break;

		/* cr */
		case 1:
			create(tokens[1], true, threadNo);
			break;

		/* del */
		case 2: 
			deleteFile(tokens[1], threadNo);
			break;

		/* mkdir */
		case 3:
			createFolder(tokens[1], true, threadNo);
			break;

		/* rmdir */
		case 4: 
			deleteFolder(tokens[1], threadNo);
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}
}


void 
cmds3Call(int index, vector<string> tokens, int threadNo)
{
	switch (index)
	{
		/* mv */
		case 0:
			move(tokens[1], tokens[2], threadNo);
			break;

		/* open */
		case 1:
			if (tokens[2] != "read" && tokens[2] != "write")
				serverResponse[threadNo] += "Please specify input mode (read | write)\n";
			else
			{
				filePosDir[threadNo] = getFileNo(tokens[1], threadNo);
				tempFolder[threadNo] = current[threadNo];
				
				if (fileExists(tokens[1], threadNo)) 
					fileCmdProcessing(tokens, threadNo);
				else
					serverResponse[threadNo] += "The file does not exist.\n";
			}
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}
}


/* ProcessCommand takes in commands vector as arugment, forwards the commands to their
   Respective functions, and returns boolean to main to update running status of 
   Program. */
bool 
processCommand (vector<string> tokens, int threadNo) 
{
	vector<string> tokens1 = {"ls", "map", "help", "rdat", "end", "\a\a\a"};
	vector<string> tokens2 = {"cd", "cr", "del", "mkdir", "rmdir"};
	vector<string> tokens3 = {"mv", "open"};

	string filename;
	bool loop = true;
	int cmdLength = tokens.size();
	int index;

	switch (cmdLength)
	{
		case 1:
			index = getCmdIndex(tokens1, tokens[0]);
			cmds1Call(index, tokens, threadNo, loop);
			break;

		case 2:
			index = getCmdIndex(tokens2, tokens[0]);
			cmds2Call(index, tokens, threadNo);
			break;

		case 3:
			index = getCmdIndex(tokens3, tokens[0]);
			cmds3Call(index, tokens, threadNo);
			break;

		default:
			invalidCmdMsg(threadNo);
			break;
	}

	return loop;
}