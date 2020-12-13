#include "config.h"
#include "util.h"
#include "globals.h"
#include "file.h"
#include "dat.h"

/* user guide. */
void 
help () 
{
	cout << "-------------------------------------------User Guide-------------------------------------------" << endl;
	cout << "Command\t\tDescription\t\t\t\t\t\t\tSyntax" << endl;
	cout << "mkdir\t\tMake new directory in given path\t\t\t\tmkdir ../folder/subf/DirName" << endl;
	cout << "ls\t\tList files in current directory\t\t\t\t\tls" << endl;
	cout << "cd\t\tChange directory to given path\t\t\t\t\tcd ../folder/subf/" << endl;
	cout << "cr\t\tCreate a new file at current working directory\t\t\tcr foo" << endl;
	cout << "open\t\tOpen a specific file at current working directory\t\topen foo read|write" << endl;

	cout << "wr\t\tWrite by appending to opened file\t\t\t\twr" << endl;
	cout << "wrat\t\tWrite starting from given byte in opened file\t\t\twrat 207" << endl;
	cout << "trun\t\tReduce opened file to given size\t\t\t\ttrun 1280" << endl;
	cout << "mvin\t\tMove from 'start' byte of 'size' to byte 'to' (ints)\t\tmvin start size to" << endl;
	cout << "rf\t\tReads from given start upto given number of characters\t\trf 312 50" << endl;
	cout << "rd\t\tReturns entire content of opened file\t\t\t\trd" << endl;
	cout << "chmod\t\tChanges mode of opened file\t\t\t\t\tchmod read|write" << endl;


	cout << "close\t\tClose the opened file\t\t\t\t\t\tclose" << endl;
	cout << "del\t\tDelete a file at the specified path\t\t\t\tdel ./folder/foo" << endl;
	cout << "rem\t\tDelete all files and folders in specified folder\t\trem folder" << endl;
	cout << "mv\t\tMove file from one location to another\t\t\t\tmv ./subf/filename ../sf/" << endl;
	cout << "map\t\tDisplay memory map\t\t\t\t\t\tmap" << endl;
	cout << "end\t\tTerminate program\t\t\t\t\t\ttend" << endl;
	cout << "rdat\t\tRead and existing .dat file generated using this program\trdat" << endl;
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


/* Prints number of bytes available by iterating over free list*/
void
printSpace()
{
	cout << "Memory available: " << freeList.size() * PAGESIZE << "/" << MEMSIZE <<
		 " bytes" << endl;
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
getFileNo (string name) 
{
	int i;
	bool found = false;

	for (i = 0; i < current->files.size(); i++) 
	{
		if (current->files[i]->name == name) 
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
fileExists (string filename) 
{
	for (int j = 0; j < tempFolder->files.size(); j++) 
	{
		if (tempFolder->files[j]->name == filename) 
		{
			filePosDir = j;
			tempFile = tempFolder->files[j];
			return true;
		}
	}
	return false;
}


/* Function folderExists takes in directory name as argument accesses the temporary 
   Folder's subdirectory and iterates it fully returns true if it is found. */
bool 
folderExists (string dirName) 
{
	for (int j = 0; j < tempFolder->subdir.size(); j++) 
	{
		if (tempFolder->subdir[j]->dirName == dirName) 
		{
			tempFolder = tempFolder->subdir[j];
			return true;
		}
	}
	return false;
}


/* Lists all the files and folders in the current working directory. */
void 
listDir () 
{
	if (current->subdir.size() == 0 && current->files.size() == 0) 
	{
		cout << "Directory is empty." << endl;
		return;
	}
	else 
	{
		for (int i = 0; i < current->subdir.size(); i++)
			cout << current->subdir[i]->dirName << "\t";

		for (int i = 0; i < current->files.size(); i++)
			cout << current->files[i]->name << "\t";

		cout << endl;
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
locateFile (vector<string> tokens, bool destFile) 
{
	found = true;
	tempFolder = current;
	int i = 0;

	/* If the first token is '.', then it works in current directory. */
	if (tokens[0] == ".")
		i = 1;

	for (i; i < tokens.size(); i++) 
	{
		if (tokens[i] == "..") 
		{
			if (tempFolder->parent == NULL) 
			{
				cout << "Parent of root does not exist." << endl;
				found = false;
				return;
			}
			tempFolder = tempFolder->parent;
		}
		else 
		{
			/* If the current token isnt the last one (file name), then check if folder
			   Exists. If it does not, then the path given is invalid, so function is
			   Exited. */
			if (i != tokens.size() - 1) 
			{
				bool checkFolder = folderExists(tokens[i]);
				if (!checkFolder) 
				{
					cout << "Invalid path. A folder in the specified path does not exist."
						 << endl;
					found = false;
					return;
				}
			}
			/* If the current token is the last one i.e. file name, then check if file
			   Exists. If it does not, then check if working with destination file, in
			   Which case, make the file at the specified location. */
			else 
			{
				/* In case it exists. */
				if (fileExists(tokens[i])) 
				{
					found = true;
					return;
				}
				/* In case it is a destination file and it does not exist yet. */
				else if (destFile) {
					cout << "Creating destination file..." << endl;
					cout << "New file created." << endl;
					tempFile = NULL;
					found = true;
					return;
				}
				/* In case it is a source file and it does not exist. */
				else if (!destFile) {
					cout << "The specified file does not exist." << endl;
					found = false;
					return;
				}
			}
		}
	}
}


/* Lists all files in current directory along with their information. */
void 
listFiles (Folder* dir) 
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
			File openFile(dir->files[i]->name, "read", false);
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

	cout << disp;
}


/* Recursive function for memory map. Lists all files in a directory, then is called
   On each subdirectory. */
void 
memMap (Folder* dir) 
{	
	tempFolder = dir;
	current = dir;
	listFiles(dir);

	if (dir->subdir.size() == 0)
		return;

	for (int i = 0; i < dir->subdir.size(); i++)
		memMap(dir->subdir[i]);
}


/* Prompts user to enter in their command, and once it is taken, it is tokenized based
   On spaces and the vector of resulting strings is returned. */
vector<string> 
getCommand (ifstream& input) 
{
	string command;
	//cout << pathFromRoot(current) << "> ";
	//getline(cin, command);
	getline(input,command);
	return tokenize(command, ' ');
}



/* ProcessCommand takes in commands vector as arugment, forwards the commands to their
   Respective functions, and returns boolean to main to update running status of 
   Program. */
bool 
processCommand (vector<string> tokens, ifstream& input) 
{
	string filename;
	bool loop = true;

	if (tokens[0] == "open") 
	{
		if (tokens.size() == 2 || (tokens[2] != "read" && tokens[2] != "write"))
			cout << "Please input mode (read|write)" << endl;

		else if (tokens.size() == 3 && tokens[0] == "open" && (tokens[2] == "write"
			 || tokens[2] == "read")) 
		{

			filePosDir = getFileNo(tokens[1]);

			if (fileExists(tokens[1])) 
			{
				File openedFile(tokens[1], tokens[2], true);

				bool inLoop = true;

				while (inLoop) 
				{
					vector<string> tokens = getCommand(input);

					if (tokens.size() == 1 && tokens[0] == "wr")
						openedFile.write(openedFile.getInput(input),true);

					else if (tokens.size() == 2 && tokens[0] == "wrat" && isNumber(tokens[1]))
						openedFile.writeAt(openedFile.getInput(input), stoi(tokens[1]) - 1);

					else if (tokens.size() == 2 && tokens[0] == "chmod")
						openedFile.changeMode(tokens[1]);

					else if (tokens.size() == 1 && tokens[0] == "rd")
						cout << openedFile.read(0, openedFile.getFileSize()) << endl;

					else if (tokens.size() == 3 && tokens[0] == "rf" && isNumber(tokens[1])
						 && isNumber(tokens[2]))
						cout << openedFile.read(stoi(tokens[1]) - 1, stoi(tokens[2]))
							 << endl;

					else if (tokens.size() == 2 && tokens[0] == "trun" && isNumber(tokens[1]))
						openedFile.truncate(stoi(tokens[1]));

					else if (tokens.size() == 4 && tokens[0] == "mvin" && isNumber(tokens[1])
						 && isNumber(tokens[2]) && isNumber(tokens[3]))
						openedFile.moveWithin(stoi(tokens[1]), stoi(tokens[2]), stoi(tokens[3]));

					else if (tokens.size() == 1 && tokens[0] == "end")
						cout << "Close file before ending program." << endl;

					else if (tokens.size() == 1 && tokens[0] == "help")
						help();

					else if (tokens.size() == 1 && tokens[0] == "close") 
					{
						cout << "File closed." << endl;
						inLoop = false;
					}

					else
						cout << "Invalid command. Type help for user guide." << endl;
				}

				filePosDir = -1;
			}
			else
			{
				cout << "The file does not exist." << endl;
				return loop;
			}
		}
	}

	else if (tokens.size() == 1 && tokens[0] == "ls") 
		listDir();

	else if (tokens.size() == 2 && tokens[0] == "cd")
		changeDir(tokens[1]);
	
	else if (tokens.size() == 2 && tokens[0] == "cr")
		create(tokens[1],true);
	
	else if (tokens.size() == 3 && tokens[0] == "mv") 
		move(tokens[1], tokens[2]);
	
	else if (tokens.size() == 2 && tokens[0] == "del")
		deleteFile(tokens[1]);

	else if (tokens.size() == 2 && tokens[0] == "rem")
		deleteFolder(tokens[1]);	
	
	else if (tokens.size() == 2 && tokens[0] == "mkdir")
		createFolder(tokens[1],true);
	
	else if (tokens.size() == 1 && tokens[0] == "map"){
		printSpace();
		Folder *currentFolder = current;
		memMap(rootFolder);
		current = currentFolder;
	}

	else if (tokens.size() == 1 && tokens[0] == "help")
		help();

	else if (tokens.size() == 1 && tokens[0] == "rdat") 
	{
		cout << "reading .dat file ..." << endl;
		readDat();
		cout << "Complete" << endl;
		printSpace();
	}
	
	else if (tokens.size() == 1 && tokens[0] == "end") 
		loop = false;
	
	else 
		cout << "Invalid command. Type help for user guide." << endl;

	return loop;
}