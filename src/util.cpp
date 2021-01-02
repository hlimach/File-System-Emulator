#include "headers/config.h"
#include "headers/util.h"
#include "headers/globals.h"
#include "headers/file.h"
#include "headers/dat.h"

/* user guide. */
void 
help () 
{
	serverResponse += "-------------------------------------------User Guide-------------------------------------------\n";
	serverResponse += "Command\t\tDescription\t\t\t\t\t\t\tSyntax\n";
	serverResponse += "mkdir\t\tMake new directory in given path\t\t\t\tmkdir ../folder/subf/DirName\n";
	serverResponse += "ls\t\tList files in current directory\t\t\t\t\tls\n";
	serverResponse += "cd\t\tChange directory to given path\t\t\t\t\tcd ../folder/subf/\n";
	serverResponse += "cr\t\tCreate a new file at current working directory\t\t\tcr foo\n";
	serverResponse += "open\t\tOpen a specific file at current working directory\t\topen foo read|write\n";

	serverResponse += "wr\t\tWrite by appending to opened file\t\t\t\twr\n";
	serverResponse += "wrat\t\tWrite starting from given byte in opened file\t\t\twrat 207\n";
	serverResponse += "trun\t\tReduce opened file to given size\t\t\t\ttrun 1280\n";
	serverResponse += "mvin\t\tMove from 'start' byte of 'size' to byte 'to' (ints)\t\tmvin start size to\n";
	serverResponse += "rf\t\tReads from given start upto given number of characters\t\trf 312 50\n";
	serverResponse += "rd\t\tReturns entire content of opened file\t\t\t\trd\n";
	serverResponse += "chmod\t\tChanges mode of opened file\t\t\t\t\tchmod read|write\n";


	serverResponse += "close\t\tClose the opened file\t\t\t\t\t\tclose\n";
	serverResponse += "del\t\tDelete a file at the specified path\t\t\t\tdel ./folder/foo\n";
	serverResponse += "rem\t\tDelete all files and folders in specified folder\t\trem folder\n";
	serverResponse += "mv\t\tMove file from one location to another\t\t\t\tmv ./subf/filename ../sf/\n";
	serverResponse += "map\t\tDisplay memory map\t\t\t\t\t\tmap\n";
	serverResponse += "end\t\tTerminate program\t\t\t\t\t\ttend\n";
	serverResponse += "rdat\t\tRead and existing .dat file generated using this program\trdat\n";
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
convertMessage (string message,int length)
{
	char * msg = (char *) malloc(length);
    strcpy(msg,message.c_str());
	return msg;
}



/* Prints number of bytes available by checking free list*/
void
printSpace(int threadNo)
{
	serverResponse += "Memory available: " + to_string(freeList.size() * PAGESIZE) + "/" + 
			MEMSIZE + " bytes\n";
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
folderExists (string dirName, int threadNo) 
{
	for (int j = 0; j < tempFolder[threadNo]->subdir.size(); j++) 
	{
		if (tempFolder[threadNo]->subdir[j]->dirName == dirName) 
		{
			tempFolder[threadNo] = tempFolder[threadNo]->subdir[j];
			return true;
		}
	}
	return false;
}


/* Lists all the files and folders in the current working directory. */
void 
listDir (int threadNo) 
{

	char *op;

	if (current[threadNo]->subdir.size() == 0 && current[threadNo]->files.size() == 0) 
	{
		serverResponse += "Directory is empty.\n";
		return;
	}
	else 
	{
		for (int i = 0; i < current[threadNo]->subdir.size(); i++)
			serverResponse += current[threadNo]->subdir[i]->dirName + "\t";

		for (int i = 0; i < current[threadNo]->files.size(); i++)
			serverResponse += current[threadNo]->files[i]->name + "\t";

		serverResponse += "\n";
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

	for (i; i < tokens.size(); i++) 
	{
		if (tokens[i] == "..") 
		{
			if (tempFolder[threadNo]->parent == NULL) 
			{
				serverResponse += "Parent of root does not exist.\n";
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
				bool checkFolder = folderExists(tokens[i],threadNo);
				if (!checkFolder) 
				{
					serverResponse += "Invalid path. A folder in the path does not exist.\n";
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
					serverResponse += "Creating destination file...\n";
					serverResponse += "New file created.\n";
					tempFile[threadNo] = NULL;
					found[threadNo] = true;
					return;
				}
				/* In case it is a source file and it does not exist. */
				else if (!destFile) {
					serverResponse += "The specified file does not exist.\n";
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
            int nextPageTableNum = getPageNum((char *) pgTbl), temp = 0;

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

            disp += "Name: " + name + "\n\tPage numers: " + pgnums + 
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

	serverResponse += disp;
}


/* Recursive function for memory map. Lists all files in a directory, then is called
   On each subdirectory. */
void 
memMap (Folder* dir, int threadNo) 
{	
	tempFolder[threadNo] = dir;
	current[threadNo] = dir;
	listFiles(dir,threadNo);

	if (dir->subdir.size() == 0)
		return;

	for (int i = 0; i < dir->subdir.size(); i++)
		memMap(dir->subdir[i],threadNo);
}


/* Returns the response of the command to the client. */
void
sendResponse (int threadNo) 
{
	char *op;
	serverResponse += users[threadNo] + "$ " + pathFromRoot(current[threadNo]) + ">";
	op = convertMessage(serverResponse, serverResponse.size());
	send(sockets[threadNo], op, strlen(op), 0);
	serverResponse = "";
}


/* Prompts user to enter in their command, and once it is taken, it is tokenized based
   On spaces and the vector of resulting strings is returned. */
vector<string> 
getCommand (int threadNo) 
{
	sendResponse(threadNo);

	char buffer[1024] = {0};
	int valread;
	string command;
	
	valread = read(sockets[threadNo], buffer, 1024);
	command = buffer; 
	cout << command << endl;
	return tokenize(command, ' ');
}



/* ProcessCommand takes in commands vector as arugment, forwards the commands to their
   Respective functions, and returns boolean to main to update running status of 
   Program. */
bool 
processCommand (vector<string> tokens, int threadNo) 
{
	string filename;
	bool loop = true;

	if (tokens[0] == "open") 
	{

		if (tokens.size() == 2 || (tokens[2] != "read" && tokens[2] != "write"))
			serverResponse += "Please specify input mode (read|write)\n";

		else if (tokens.size() == 3 && tokens[0] == "open" && (tokens[2] == "write"
			 || tokens[2] == "read")) 
		{

			filePosDir[threadNo] = getFileNo(tokens[1],threadNo);

			if (fileExists(tokens[1],threadNo)) 
			{
				
				openedFiles = File(tokens[1], tokens[2], true, threadNo);
				bool inLoop = true;
				
				while (inLoop) 
				{
					vector<string> tokens = getCommand(threadNo);

					if (tokens.size() == 1 && tokens[0] == "wr")
						openedFiles.write(openedFiles.getInput(input),true);

					else if (tokens.size() == 2 && tokens[0] == "wrat" && isNumber(tokens[1]))
						openedFiles.writeAt(openedFiles.getInput(input), stoi(tokens[1]) - 1);

					else if (tokens.size() == 2 && tokens[0] == "chmod")
						openedFiles.changeMode(tokens[1]);

					else if (tokens.size() == 1 && tokens[0] == "rd")
						serverResponse += openedFiles.read(0, openedFiles.getFileSize()) + "\n";

					else if (tokens.size() == 3 && tokens[0] == "rf" && isNumber(tokens[1])
						 && isNumber(tokens[2]))
						serverResponse += openedFiles.read(stoi(tokens[1]) - 1, stoi(tokens[2])) + "\n";

					else if (tokens.size() == 2 && tokens[0] == "trun" && isNumber(tokens[1]))
						openedFiles.truncate(stoi(tokens[1]));

					else if (tokens.size() == 4 && tokens[0] == "mvin" && isNumber(tokens[1])
						 && isNumber(tokens[2]) && isNumber(tokens[3]))
						openedFiles.moveWithin(stoi(tokens[1]), stoi(tokens[2]), stoi(tokens[3]));

					else if (tokens.size() == 1 && tokens[0] == "end")
						serverResponse += "Close file before ending program.\n";

					else if (tokens.size() == 1 && tokens[0] == "help")
						help();

					else if (tokens.size() == 1 && tokens[0] == "close") 
					{
						serverResponse += "File closed.\n";
						inLoop = false;
					}

					else
						serverResponse += "Invalid command. Type help for user guide.\n";
				}

				filePosDir[threadNo] = -1;
			}
			else
			{
				serverResponse += "The file does not exist.\n";
				return loop;
			}
		}
	}

	else if (tokens.size() == 1 && tokens[0] == "ls") 
		listDir(threadNo);

	else if (tokens.size() == 2 && tokens[0] == "cd")
		changeDir(tokens[1],threadNo);
	
	else if (tokens.size() == 2 && tokens[0] == "cr")
		create(tokens[1],true,threadNo);
	
	else if (tokens.size() == 3 && tokens[0] == "mv") 
		move(tokens[1], tokens[2],threadNo);
	
	else if (tokens.size() == 2 && tokens[0] == "del")
		deleteFile(tokens[1],threadNo);

	else if (tokens.size() == 2 && tokens[0] == "rem")
		deleteFolder(tokens[1],threadNo);	
	
	else if (tokens.size() == 2 && tokens[0] == "mkdir")
		createFolder(tokens[1],true,threadNo);
	
	else if (tokens.size() == 1 && tokens[0] == "map"){
		printSpace(threadNo);
		Folder *currentFolder = current[threadNo];
		memMap(rootFolder,threadNo);
		current[threadNo] = currentFolder;
	}

	else if (tokens.size() == 1 && tokens[0] == "help")
		help();

	else if (tokens.size() == 1 && tokens[0] == "rdat") 
	{
		serverResponse += "reading .dat file ...\n";
		readDat();
		serverResponse += "Complete\n";
		printSpace(threadNo);
	}
	
	else if (tokens.size() == 1 && tokens[0] == "end") 
		loop = false;
	
	else 
		serverResponse += "Invalid command. Type help for user guide.\n";

	return loop;
}