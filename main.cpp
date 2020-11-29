#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stack>
#include <string>
#include <sstream>
#include <vector>

#define PAGESIZE 256		//	size of each page in memory
#define MEMSIZE 16777216	//	total memory size - 16Mb
#define NUMPAGES 65536		//	total pages in memory
using namespace std;

const char* start;


//	freeList holds ineger values of all sectors that are unused
stack <int> freeList;


//	tokenizes a given string with respect to delimeter
vector<string> tokenize(string command, char delimiter) {

	string s;
	stringstream ss(command);
	vector<string> tokens;
	
	while (getline(ss, s, delimiter)) {

		if (s == "")
			continue;

		tokens.push_back(s);

	}

	return tokens;

}



/* 
	function getSector takes a sector's integer value 
	and returns a pointer to that sector 
*/
char* getSector(int x) {

	return (char*) start + (PAGESIZE * x);

}



/* 
	function getEntry takes a pointer as argument
	it finds the page number in which this pointer is located
	returns the page number as integer
*/
int getEntry(int* x) {

	return (x - (int*) start) / PAGESIZE;

}



/*
	explain function
*/
class FileNode {
public:
	string name;
	int* pgTblPtr;

	FileNode(string fileName){
		name = fileName;
		pgTblPtr = NULL;
	}
};



/*
	explain class
*/
class Folder {
public:
	string dirName;
	vector<Folder*> subdir;
	vector<FileNode*> files;
	Folder* parent;

	Folder(string name) {
	
		dirName = name;
		parent = NULL;
	
	}
};


// the default starting folder 
Folder* rootFolder = new Folder("root");


// keeps track of current position in directory
Folder  *current, *tempFolder;
FileNode *tempFile;
int filePosDir;
bool fileFound, found;



/*
	explain function
*/
int getFileNo(string name) {
	
	int i;
	bool found = false;

	for (i = 0; i < current->files.size(); i++) {
		if (current->files[i]->name == name) {
			found = true;
			break;
		}
	}

	if (found)
		return i;
	else 
		return -1;

}



/* 
	function getFileSize takes in filename as argument 
	accesses the files' page table and iterates it fully
	returns the file size in bytes
*/
int getFileSize(string filename) {

	if (getFileNo(filename) == -1) {
		return -1;

	} else {
		int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;

		if (pageTable == NULL) {
			return 0;
		}

		char* page = NULL;
		int size = 0, i = 0;


		// loop runs until the limit field found to be non-zero (last sector)
	    while (*(pageTable + i + 1) == 0) {
	        size += PAGESIZE;
	        i += 2;
	    }


		// i is now at the last entry of the files' page table
		// limit value obtained and added to size
		size += *(pageTable + i + 1);


		return size;
	}

}



/*
	File class object is created once the file is opened.

	A file can only be altered (read/ write/ truncate) if its object is created.

	The functions for alteration are all methods of this class to limit accessibility.

	Modes for opening file are read and write:

	Read allows user to view entire content of file, or can specify which byte to view from.

	Write allows user to fill up a new file, append to existing content, start overwriting from specified byte, or truncate size.
*/
class File {
private:
	string filename, mode;
	int* pageTable;
	char* page;
	int filesize;

public:
	File(string name, string md) {
		fileFound = false;
        for (int i = 0; i < current->files.size(); i++) {
            if (current->files[i]->name == name) {
                fileFound = true;
                filename = name;
                filesize = getFileSize(filename);
                mode = md;
                pageTable = current->files[i]->pgTblPtr;
                page = NULL;
            }
        }
        
        if (!fileFound) {
            cout << "The file does not exist." << endl;
        } else {
        	cout << "File opened: " << filename << ", size: " << filesize << " bytes." << endl;
        }
        
    }


	/* 
		Function read prints out entire content of file
	*/
	void read() {
		int i;

		if (pageTable == NULL) {
			cout << "The file has no content to display." << endl;
			return;
		}

		// loop runs until the limit field found to be non-zero (last sector)
		for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
			page = getSector(*(pageTable + i));


			// loops inside the sector pointed to by char pointer 'page'
			for (int j = 0; j < PAGESIZE; j++)
				cout << *(page + j);

		}

		// i is now at the last entry of the files' page table so its limit value is obtained
		int limit = *(pageTable + i + 1);

		// pointer to the last page is obtained 
		page = getSector(*(pageTable + i));

		// loops over the last page until the limit that was previously obtained
		for (int j = 0; j < limit; j++)
			cout << *(page + j);

		cout << endl;
	}



	/* 
		function readupto takes two arguments starting bite and size of bytes to be read, and
		prints out the content within that limit.
	*/
	void readUpto(int startFrom, int readUpTo) {

		if (pageTable == NULL) {
			cout << "The file has no content to display." << endl;
			return;
		}

		if (readUpTo - startFrom > filesize) {
			cout << "Out of bound exception. Given limit exceeds total file limit at " << filesize << " bytes." << endl;
			return;
		}

		//	which page number in the page table the byte will belong to
		int startPage = (startFrom / PAGESIZE);
		int startByte = startFrom % PAGESIZE;
		int endPage = (startFrom + readUpTo) / PAGESIZE;
		int limit = (startFrom + readUpTo) % PAGESIZE;
		int i;

		//	iterates all pages before this page and outputs their content
		for (i = startPage * 2; i < (endPage * 2); i += 2) {

			page = getSector(*(pageTable + i));

			for (int j = startByte; j < PAGESIZE; j++)
				cout << *(page + j);

		}

		//	gets last page and reads upto the specified byte
		page = getSector(*(pageTable + i));

		if (startPage == endPage) {

			for (int j = startByte; j < limit; j++)
				cout << *(page + j);

		}
		else {

			for (int j = 0; j < limit; j++)
				cout << *(page + j);
		}

		cout << endl;
	}



	/* 
		function write checks if file is empty or not. If it a new file then it starts to write from the start,
		else it starts to append from the last byte of file.
	*/
	void write() {
		
		string input, line;

		while (getline(cin, line)) {
			if (line == "-1")
				break;
			input += line;
			input += "\n";
		}

		cout << input.length() << endl;
		int numberOfSectors = 0, limit, appendPoint, appendPage, appendSector, remainder, countSectors;

		if (pageTable == NULL) {

			int freeSect = freeList.top();
			freeList.pop();

			char* sector = getSector(freeSect);
			current->files[filePosDir]->pgTblPtr = (int *)sector;
			pageTable = current->files[filePosDir]->pgTblPtr;

			numberOfSectors = input.length() / PAGESIZE + 1;
			limit = input.length() % PAGESIZE;
			
			for (int i = 0; i < numberOfSectors * 2; i += 2) {

				*(pageTable + i) = freeList.top();

				if (i == numberOfSectors * 2 - 2)
					* (pageTable + i + 1) = limit;
				else
					*(pageTable + i + 1) = NULL;

				freeList.pop();
			}

			for (int i = 0; i < numberOfSectors * 2; i += 2) {

				page = getSector(*(pageTable + i));

				if (i != numberOfSectors * 2 - 2) {
					for (int j = 0; j < PAGESIZE; j++) {
						*(page + j) = input[((i / 2) * PAGESIZE) + j];
					}
				}
				else {
					for (int j = 0; j < limit; j++) {
						*(page + j) = input[((i / 2) * PAGESIZE) + j];
					}
				}
			}
			

		} else {
			
			int i;
			countSectors = 0;

			for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
				countSectors++;

			}
			countSectors++;
			appendPoint = *(pageTable + i + 1);
			appendPage = *(pageTable + i);

			page = getSector(appendPage);

			if (input.length() < (PAGESIZE - appendPoint)) {

				for (int j = 0; j < input.length(); j++) {
					*(page + appendPoint + j) = input[j];
				}
				appendPoint += input.length();
				*(pageTable + i + 1) = appendPoint;

				return;
			}
			else {
			
				for (int j = appendPoint; j < PAGESIZE; j++) {
					*(page + j) = input[j - appendPoint];
				}
			
			}

			*(pageTable + i + 1) = NULL;
			remainder = PAGESIZE - appendPoint;

			numberOfSectors = (input.length() - remainder) / PAGESIZE + 1;
			limit = (input.length() - remainder) % PAGESIZE;
			numberOfSectors = numberOfSectors + countSectors;
			appendSector = i;
			for (i = appendSector + 2; i < numberOfSectors * 2; i += 2) {

				*(pageTable + i) = freeList.top();

				if (i == numberOfSectors * 2 - 2)
					* (pageTable + i + 1) = limit;
				else
					*(pageTable + i + 1) = 0;

				freeList.pop();
			}

			//writing

			for (i = appendSector + 2; i < numberOfSectors * 2; i += 2) {

				page = getSector(*(pageTable + i));

				if (i != numberOfSectors * 2 - 2) {
					for (int j = 0; j < PAGESIZE; j++) {
						cout << j << endl;
						*(page + j) = input[(((i - appendSector - 2) / 2) * PAGESIZE) + j + (remainder)];
					}
				}
				else {
					for (int j = 0; j < limit; j++) {
						*(page + j) = input[(((i - appendSector - 2) / 2) * PAGESIZE) + j + (remainder)];
					}
				}
			}
			
		}	
	}



	/* 
		function writeAt takes in positional byte as argumnent where the over writing should begin from, 
		truncates the file to that size and starts writing from that position.
	*/
	void writeAt(int writeAt) {

		if (pageTable == NULL) {
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;
			return;
		}

		if (writeAt > filesize) {
			cout << "Out of bound exception. Given byte is greater than file size of" << filesize << " bytes." << endl;
			return;
		}


		truncate(writeAt);
		write();
	}



	/* 
		function truncate takes in size as integer to which the file size must be reduced to,
		and updates the file to be that size and deletes the rest of it.
	*/
	void truncate(int size) {

		if (size > filesize) {
			cout << "Out of bound exception. Given byte is greater than file size of" << filesize << " bytes." << endl;
			return;
		}
		
		int truncSectors, truncLimit, i;
		truncSectors = size / PAGESIZE;
		truncLimit = size % PAGESIZE;
		bool isLastSect = false;

		if (*(pageTable + truncSectors * 2 + 1) != 0)
			isLastSect = true;

		*(pageTable + truncSectors * 2 + 1) = truncLimit;	

		for (i = truncSectors*2 + 2; *(pageTable + i + 1) == 0; i += 2) {

			freeList.push(*(pageTable + i));

		}
		
		if (!isLastSect) {
			freeList.push(*(pageTable + i));
		}
	}



	/* 
		Move within is a cut paste function within the file. It takes 3 arguments from (starting byte), 
		size (how many bytes to be cut starting from), and to (where the chunk should be pasted).
	*/
	void moveWithin(int from, int size, int to) {
		//the chunk specified is copied into a string var
		// the other text is copied 
		if (pageTable == NULL) {
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;
			return;
		}

		if (from + size > filesize) {
			cout << "Out of bound exception. Specified chunk exceeds out of file size of" << filesize << " bytes." << endl;
			return;
		}

		char *startPage, *endPage;
		int startPg = (from / PAGESIZE + 1) * 2 - 2;
		int endPg = ((from + to) / PAGESIZE + 1) * 2 - 2;


	}

};



/*
*/
bool fileExists(string filename) {
    for (int j = 0; j < tempFolder->files.size(); j++) {
        if (tempFolder->files[j]->name == filename) {
            tempFile = tempFolder->files[j];
            return true;// found;
        }
    }
    return false;
}



/*
*/
bool folderExists(string dirName) {
    for (int j = 0; j < tempFolder->subdir.size(); j++) {
        if (tempFolder->subdir[j]->dirName == dirName) {
            filePosDir = j;
            tempFolder = tempFolder->subdir[j];
            return true;
        }
    }
    return false;
}



/*
	function checkDuplicate takes in name as an arguemnt and checks in the 
	current working directory for a file or folder with same name
	if either one exists, the function returns true.
*/
bool checkDuplicate(string name) {
	bool sameFile = fileExists(name);
	bool sameFolder = folderExists(name);

	if (sameFile || sameFolder)
		return true;
	else
		return false;
}


/*
	explain class
*/
void createFolder(string path) {

	bool createable = true;
	tempFolder = current;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) {
		cout << "Invalid path entered." << endl;
		return;
	}
	
	for (int i = 0; i < tokens.size() - 1; i++) {

		if (tokens[i] == "..")
			tempFolder = tempFolder->parent;

		else {

			bool folderFound = folderExists(tokens[i]);

			if (!folderFound) {
				createable = false;
				break;
			}

		}
	
	}

	if (createable && !checkDuplicate(tokens.back())) {

		tempFolder->subdir.push_back(new Folder(tokens.back()));
		tempFolder->subdir.back()->parent = tempFolder;

	} else
		cout << "Error: cannot create directory in specified path." << endl;

}



/*
	explain function
*/
void listDir() {

	if (current->subdir.size() == 0 && current->files.size() == 0) {
		cout << "Directory is empty." << endl;
		return;
	}
	
	cout << "Folders: ";
	for (int i = 0; i < current->subdir.size(); i++)
		cout << current->subdir[i]->dirName << "\t";
	cout << endl;
	
	cout << "Files: ";
	for (int i = 0; i < current->files.size(); i++)
		cout << current->files[i]->name << "\t";
	cout << endl;	
}



/*
*/
void changeDir(string path) {

	bool changable = true;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) {
		cout << "Invalid path entered." << endl;
		return;
	}

	int j;
	tempFolder = current;
	
	for (int i = 0; i < tokens.size(); i++) {

		if (tokens[i] == "..")
			tempFolder = tempFolder->parent;

		else {
			bool folderFound = folderExists(tokens[i]);

			if (!folderFound) {
				changable = false;
				break;
			}
		}
	}

	if (changable)
		current = tempFolder;
	else
		cout << "Error: cannot change directory to specified path." << endl;

}



/*
	explain function
*/
bool isNumber(string s) {

	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;

}



/*
*/
void create(string filename) {

	if (!checkDuplicate(filename))
		current->files.push_back(new FileNode(filename));
	else
		cout << "A file or folder of same name already exists." << endl;

}



void locateFile(vector<string> tokens, bool destFile) {

	found = true;

    for (int i = 0; i < tokens.size(); i++) {

        if (tokens[i] == "..") {
            tempFolder = current->parent;
        } else if (tokens[i] == ".") {
            tempFolder = current;
        } else {

            if (i != tokens.size() - 1) {
                bool checkFolder = folderExists(tokens[i]);

                if (!checkFolder) {
                    cout << "Invalid path. A folder in the specified path does not exist." << endl;
                    found = false;
                    return;
                }

            } else {
                bool checkFile = fileExists(tokens[i]);

                // in case it is found
                if (checkFile) 
                	return;

                // in case it is a destination file and it does not exist yet
                else if (!checkFile && destFile) {
                    cout << "Creating destination file..." << endl;
                    current = tempFolder;
                    create(tokens[tokens.size() - 1]);
                    filePosDir = current->files.size() - 1;
                    tempFile = current->files[filePosDir];
                    return;
                }

                // in case it is a source file and is not found
                else if (!checkFile && !destFile) {
                    cout << "The specified file does not exist." << endl;
                    found = false;
                    return;
                }
                
            }

        }
    }

}



/*
    takes in two arguments source file path and destination file path
    copies the source file to destination file
*/
void move(string srcPath, string destPath) {

    vector<string> tokenSrcFile = tokenize(srcPath, '/');
    vector<string> tokenDestFile = tokenize(destPath, '/');

    int srcPos, destPos;
    FileNode *srcFile, *destFile;
    Folder *srcFolder, *destFolder;

    locateFile(tokenSrcFile, false);
    bool srcFileFound = found;
    if (srcFileFound) {
        srcFile = tempFile;
        srcFolder = tempFolder;
        srcPos = filePosDir;
    }
    else
        return;


    locateFile(tokenDestFile, true);
    bool destFileFound = found;
    if (destFileFound) {
        destFile = tempFile;
        destFolder = tempFolder;
        destPos = filePosDir;
    }
    else
        return;


    if (srcFile != destFile) {
        if (destFile->pgTblPtr != NULL) {
            current = destFolder;
            File openFile(tokenDestFile[tokenDestFile.size() - 1], "write");
            openFile.truncate(0);
        }
        
        destFile->pgTblPtr = srcFile->pgTblPtr;
        srcFile->pgTblPtr = NULL;
        (srcFolder->files).erase(srcFolder->files.begin() + srcPos);


    } else {
        cout << "Invalid Arguments. To move within a file, plese open the file first." << endl;
        return;
    }

}



/*
	explain function
*/
void deleteFile(string filename) {

	if (getFileNo(filename) == -1) {
		cout << "Error: file does not exist" << endl;
		
	} else {
		int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;
		int pageNumber;
		int i;
		stack <int> temp;


		for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
			pageNumber = *(pageTable + i);
			temp.push(pageNumber);
		}


		pageNumber = *(pageTable + i);
		temp.push(pageNumber);


		while (!temp.empty()) {

			freeList.push(temp.top());
			temp.pop();
		}


		freeList.push(getEntry(pageTable));
		cout << freeList.top() << endl;

		current->files.erase(current->files.begin() + filePosDir);
	}
}



/*
	explain function
*/
void help() {

	//for macos

	// cout << "-------------------------------------------User Guide-------------------------------------------" << endl;
	// cout << "Command\t\tDescription\t\t\t\t\t\t\t\t\t\t\t\tSyntax" << endl;
	// cout << "mkdir\t\tMake new directory in given path\t\t\t\t\t\tmkdir ../folder/subf/DirName" << endl;
	// cout << "ls\t\t\tList files in current directory\t\t\t\t\t\t\tls" << endl;
	// cout << "cd\t\t\tChange directory to given path\t\t\t\t\t\t\tcd ../folder/subf/" << endl;
	// cout << "cr\t\t\tCreate a new file at current working directory\t\t\tcr foo" << endl;
	// cout << "open\t\tOpen the specified file\t\t\t\t\t\t\t\t\topen foo" << endl;

	// cout << "wr\t\t\tWrite by appending to opened file\t\t\t\t\t\twr" << endl;
	// cout << "wrat\t\tWrite starting from given byte in opened file\t\t\twrat 207" << endl;
	// cout << "rd\t\t\tReturns entire content of opened file\t\t\t\t\trd" << endl;
	// cout << "rf\t\t\tReads from start upto given number of characters\t\trf 312" << endl;
	// cout << "trun\t\tReduce opened file to given size\t\t\t\t\t\ttrun 1280" << endl;


	// cout << "close\t\tClose the opened file\t\t\t\t\t\t\t\t\tclose" << endl;
	// cout << "del\t\t\tDelete a file at the specified path\t\t\t\t\t\tdel ./folder/foo" << endl;
	// cout << "mv\t\t\tMove file from one location to another\t\t\t\t\tmv ./subf/filename ../sf/" << endl;
	// cout << "map\t\t\tDisplay memory map\t\t\t\t\t\t\t\t\t\tmap" << endl;
	// cout << "end\t\t\tTerminate program\t\t\t\t\t\t\t\t\t\tend" << endl;

	//for windows

	cout << "-------------------------------------------User Guide-------------------------------------------" << endl;
	cout << "Command\t\t\tDescription\t\t\t\t\t\tSyntax" << endl;
	cout << "mkdir\t\t\tMake new directory in given path\t\t\tmkdir ../folder/subf/DirName" << endl;
	cout << "ls\t\t\tList files in current directory\t\t\t\tls" << endl;
	cout << "cd\t\t\tChange directory to given path\t\t\t\tcd ../folder/subf/" << endl;
	cout << "cr\t\t\tCreate a new file at current working directory\t\tcr foo" << endl;
	cout << "open\t\t\tOpen the specified file\t\t\t\t\topen foo" << endl;

	cout << "wr\t\t\twrite by appending to opened file\t\t\twr" << endl;
	cout << "wrat\t\t\twrite starting from given byte in opened file\t\twrat 207" << endl;
	cout << "rd\t\t\treturns entire content of opened file\t\t\trd" << endl;
	cout << "rf\t\t\treads from start upto given number of characters\trf 312" << endl;
	cout << "trun\t\t\treduce opened file to given size\t\t\trun 1280" << endl;


	cout << "close\t\t\tClose the opened file\t\t\t\t\tclose" << endl;
	cout << "del\t\t\tDelete a file at the specified path\t\t\tdel ./folder/foo" << endl;
	cout << "mv\t\t\tMove file from one location to another\t\t\tmv ./subf/filename ../sf/" << endl;
	cout << "map\t\t\tDisplay memory map\t\t\t\t\tmap" << endl;
	cout << "end\t\t\tTerminate program\t\t\t\t\tend" << endl;

}



/* 
	function getCommand takes no arguments in
	asks for user input (command to be run)
	takes the entered string and breaks it up by space char
	returns vector of strings containing command words
*/
vector<string> getCommand() {

	string command;

	cout << "> ";
	getline(cin, command);

	return tokenize(command,' ');

}



/* 
	processCommand takes in commands vector as arugment
	forwards the commands to their respective functions
	and returns boolean to main to update running status of program
*/
bool processCommand(vector<string> tokens) {

	string filename;
	bool loop = true;


	if (tokens.size() == 3 && tokens[0] == "open") {

		File openedFile(tokens[1], tokens[2]);

        if (!fileFound)
        	return loop;

		filePosDir = getFileNo(tokens[1]);
        bool inLoop = true;

        while (inLoop) {
	        vector<string> tokens = getCommand();

	        if (tokens.size() == 1 && tokens[0] == "wr")
	            openedFile.write();

	        else if (tokens.size() == 2 && tokens[0] == "wrat" && isNumber(tokens[1]))
	            openedFile.writeAt(stoi(tokens[1]));

	        else if (tokens.size() == 1 && tokens[0] == "rd")
	            openedFile.read();

	        else if (tokens.size() == 3 && tokens[0] == "rf" && isNumber(tokens[1]) && isNumber(tokens[2]))
	            openedFile.readUpto(stoi(tokens[1]), stoi(tokens[2]));

	        else if (tokens.size() == 2 && tokens[0] == "trun" && isNumber(tokens[1]))
	            openedFile.truncate(stoi(tokens[1]));

	        else if (tokens.size() == 1 && tokens[0] == "close") {
	            cout << "File closed." << endl;
	            inLoop = false;
	        }

	        else if (tokens.size() == 1 && tokens[0] == "end")
	            cout << "Close file before ending program." << endl;

	        else if (tokens.size() == 1 && tokens[0] == "help")
	        	help();

	    	else
	            cout << "Invalid command. Type help for user guide." << endl;
	    }   	
		

    } else if (tokens.size() == 1 && tokens[0] == "ls") {
        listDir();
    } else if (tokens.size() == 2 && tokens[0] == "cd") {
        changeDir(tokens[1]);
    } else if (tokens.size() == 2 && tokens[0] == "cr") {
        create(tokens[1]);
    } else if (tokens.size() == 3 && tokens[0] == "mv") {
        move(tokens[1], tokens[2]);
    } else if (tokens.size() == 2 && tokens[0] == "del") {
        deleteFile(tokens[1]);
    } else if (tokens.size() == 2 && tokens[0] == "mkdir") {
        createFolder(tokens[1]);
    } else if (tokens.size() == 1 && tokens[0] == "map") {
        //memMap();
    } else if (tokens.size() == 1 && tokens[0] == "help") {
        help();
    } else if (tokens.size() == 1 && tokens[0] == "end") {
        loop = false;
    } else {
        cout << "Invalid command. Type help for user guide." << endl;
    }

	return loop;
}



/* 
	explain
	function
*/
int main(int argc, const char* argv[]) {
	
	current = rootFolder;
	tempFolder = rootFolder;
	start = (char*) malloc(MEMSIZE);


	for (int i = NUMPAGES; i >= 0; i--)
		freeList.push(i);


	bool loop = true;

	while (loop) {

		// take user input and store each word in vector
		vector<string> tokens = getCommand();

 
		// process command and call the respective function
		loop = processCommand(tokens);

	}


	free((char*) start);

	return 0;

}