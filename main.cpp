#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stack>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

#define PAGESIZE 256						//	size of each page in memory
#define MEMSIZE 8388608						//	total memory size - 8Mb
#define NUMPAGES (MEMSIZE / PAGESIZE)		//	total pages in memory
#define DATPATH "F:/nust/sem 5/Operating Systems/OS_Proj/solution.dat" //path for .datfile
using namespace std;

const char* start;


//	freeList holds ineger values of all sectors that are unused
stack <short int> freeList;


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
char* getSector(short int x) {

	return (char*) start + (PAGESIZE * x);

}



/*
	File node (leaf) for tree which stores name of file and its head pointer to page table
*/
class FileNode {
public:
	string name;
	short int* pgTblPtr;

	FileNode(string fileName){
		name = fileName;
		pgTblPtr = NULL;
	}
};



/*
	Folder node for tree which stores name, child pointers for files and subdirectories, and a pointer to parent directory
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


//	the default starting folder 
Folder* rootFolder = new Folder("root");


//	keeps track of current position in directory
Folder  *current, *tempFolder;
FileNode *tempFile;
int filePosDir;
bool fileFound, found;

// stream and string for writing in .dat file
ofstream dat;
string out = "";

/*
	returns the file number of given name in current directory 
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
		short int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;

		if (pageTable == NULL) {
			return 0;
		}

		char* page = NULL;
		int size = 0, i = 0;


		//	loop runs until the limit field found to be non-zero (last sector)
	    while (*(pageTable + i + 1) == -1) {
	        size += PAGESIZE;
	        i += 2;

	        if (i == (PAGESIZE/ 2 - 2) && *(pageTable + i + 1) == PAGESIZE) {
	        	pageTable = (short int* ) getSector(*(pageTable + i));
	        	i = 0;
	        }
	    }

		//	i is now at the last entry of the files' page table
		//	limit value obtained and added to size
		size += *(pageTable + i + 1);


		return size;
	}

}



/*
	function fileExists takes in filename as argument 
	accesses the temporary folder's file directory and iterates it fully
	returns the true if file exists
*/
bool fileExists(string filename) {

    for (int j = 0; j < tempFolder->files.size(); j++) {
        if (tempFolder->files[j]->name == filename) {
        	filePosDir = j;
            tempFile = tempFolder->files[j];
            return true;
        }
    }

    return false;
}



/*
	function folderExists takes in dirName as argument 
	accesses the temporary folder's subdirectory and iterates it fully
	returns the true if folder exists
*/
bool folderExists(string dirName) {

    for (int j = 0; j < tempFolder->subdir.size(); j++) {
        if (tempFolder->subdir[j]->dirName == dirName) {
            tempFolder = tempFolder->subdir[j];
            return true;
        }
    }

    return false;
}



/*
	File class object is created once the file is opened.

	A file can only be altered (read/ write/ truncate/ moveWithIn) if its object is created.

	The functions for alteration are all methods of this class to limit accessibility.

	Modes for opening file are read and write:

	Read allows user to view entire content of file, or can specify which byte to view from.

	Write allows user to fill up a new file, append to existing content, start overwriting from specified byte, truncate size, or move data within file.

*/
class File {
private:
	string filename, mode;
	short int* pageTable;
	char* page;
	int filesize;

public:
	File(string name, string md, bool info) {
		fileFound = fileExists(name);

		if(fileFound) {
			filename = name;
            filesize = getFileSize(filename);
            mode = md;
            pageTable = current->files[getFileNo(filename)]->pgTblPtr;
            page = NULL;

            if (info)
            	cout << "File opened: " << filename << ", mode: " << mode << ", size: " << filesize << " bytes." << endl;
		} 
        else 
            cout << "The file does not exist." << endl;
        
    }



	/*
		changeMode takes in argument md
		changes mode to user's specified mode
	*/
    void changeMode (string md) {
    	if (md == "read" || md == "write") {
    		mode = md;
    		cout << "File opened: " << filename << ", mode: " << mode << ", size: " << filesize << " bytes." << endl;
    	}
    	else 
    		cout << "Please enter a valid mode (read|write)." << endl;
    }



    void resetPageTablePtr () {
    	pageTable = current->files[getFileNo(filename)]->pgTblPtr;
    }


	/* 
		Function read prints out entire content of file
	*/
	void read() {

		if (mode != "read") {
			cout << "Please open file in \"read\" mode for this function." << endl;
			return;
		}

		int i;

		if (pageTable == NULL) {
			cout << "The file has no content to display." << endl;
			return;
		}

		//	loop runs until the limit field found to be non-zero (last sector)
		for (i = 0; *(pageTable + i + 1) == -1; i += 2) {

			page = getSector(*(pageTable + i));


			//	loops inside the sector pointed to by char pointer 'page'
			for (int j = 0; j < PAGESIZE; j++)
				cout << *(page + j);

			if (i == (PAGESIZE / 2 - 4) && *(pageTable + i + 3) == PAGESIZE) {
				pageTable = (short int *) getSector(*(pageTable + i + 2));
				i = -2;
			}


		}

		//	i is now at the last entry of the files' page table so its limit value is obtained
		int limit = *(pageTable + i + 1);

		//	pointer to the last page is obtained 
		page = getSector(*(pageTable + i));

		//	loops over the last page until the limit that was previously obtained
		for (int j = 0; j < limit; j++)
			cout << *(page + j);

		cout << endl;
		resetPageTablePtr();
	
	}



	/* 
		function readupto takes two arguments starting bite and size of bytes to be read, and
		prints out the content within that limit.
	*/
	string readUpto(int startFrom, int readUpTo) {

		if (mode != "read") {
			cout << "Please open file in \"read\" mode for this function." << endl;
			return "";
		}

		if (pageTable == NULL) {
			cout << "The file has no content to display." << endl;
			return "";
		}

		if (readUpTo - startFrom > filesize) {
			cout << "Out of bound exception. Given limit exceeds total file limit at " << filesize << " bytes." << endl;
			return "";
		}

		string readUptoText = "";

		//	which page number in the page table the byte will belong to
		int startPage = (startFrom / PAGESIZE);
		int startByte = startFrom % PAGESIZE;
		int endPage = (startFrom + readUpTo) / PAGESIZE;
		int limit = (startFrom + readUpTo) % PAGESIZE;
		int i;

		if (limit == 0)
			limit = PAGESIZE - 1;

		short int k = startPage * 2;
		//	iterates all pages before this page and outputs their content
		for (i = k; i < (endPage * 2); i += 2) {

			if (*(pageTable + k + 1) == PAGESIZE) {
				pageTable = (short int *) getSector(*(pageTable + k + 2));
				k = 0;
				i -= 2;
				continue;
			}

			page = getSector(*(pageTable + k));

			for (int j = startByte; j < PAGESIZE; j++)
				readUptoText += *(page + j);

			k += 2;
			startByte = 0;
		}

		//	gets last page and reads upto the specified byte
		page = getSector(*(pageTable + i));

		if (startPage == endPage) {

			for (int j = startByte; j < limit; j++)
				readUptoText += *(page + j);

		}
		else {

			for (int j = 0; j < limit; j++)
				readUptoText += *(page + j);
		}

		resetPageTablePtr();
		return readUptoText;

	}


	/*
		promts the user to write data which will be written to opened file
	*/
	string getInput() {
		
		string input, line;

		while (getline(cin, line)) {
			if (line == "-1")
				break;
			input += line;
			input += "\n";
		}

		return input;
	}

	/* 
		function write checks if file is empty or not. If it is a new file then it starts to write from the start,
		else it starts to append from the last byte of file.
	*/
	void write(string input) {
		
		if (mode != "write") {
			cout << "Please open file in \"write\" mode for this function." << endl;
			return;
		}
		

		int numberOfSectors = 0, appendSector, remainder, countSectors;
		short int limit, appendPage, appendPoint;


		if (pageTable == NULL) {

			if (input.size() > (freeList.size() * PAGESIZE)){
				cout << "Out of memory. Please reduce input size or delete other files." << endl;
				return;
			}

			//	If it is a new file 
			short int freeSect = freeList.top();
			freeList.pop();

			char* sector = getSector(freeSect);
			current->files[getFileNo(filename)]->pgTblPtr = (short int *) sector;
			resetPageTablePtr();

			
			limit = input.length() % PAGESIZE;

			if (limit == 0){
				numberOfSectors = input.length() / PAGESIZE;
				limit = PAGESIZE - 1;
			} else 
				numberOfSectors = input.length() / PAGESIZE + 1;

			/*	
				pageTable will have 2 bytes per entry (pagenum, limit) so total 4
				total entries in pgtable will be 256 / 2 but jumps twice so 128
				if the number of entries are greater than 126
				then the last two entries will be new linked pageTable extension
			*/

			short int k = 0;
			//	fill in page table with page number and limit and write to locations alongside as well
			for (short int i = 0; i < numberOfSectors * 2; i += 2) {

				if (k == ((PAGESIZE / 2) - 2)) {

					short int newTable = freeList.top();
					freeList.pop();

					*(pageTable + k) = newTable;
					*(pageTable + k + 1) = (short int) PAGESIZE;
					
					pageTable = (short int *) getSector(newTable);
					k = 0;
					i -= 2;

					continue;
				}

				*(pageTable + k) = freeList.top();
				freeList.pop();

				page = getSector(*(pageTable + k));

				if (i != numberOfSectors * 2 - 2) {
					* (pageTable + k + 1) = (short int) -1;

					for (int j = 0; j < PAGESIZE; j++) {
						*(page + j) = input[((i / 2) * PAGESIZE) + j];
						cout << j <<": "<<*(page+j) << endl;
					}

				} else {

					* (pageTable + k + 1) = limit;

					for (int j = 0; j < limit; j++) {
						*(page + j) = input[((i / 2) * PAGESIZE) + j];
						cout << j <<": "<<*(page+j) << endl;
					}

				}

				k += 2;
			}
			

		} else {
			
			int i;
			countSectors = 0;
			//	finds total number of sectors in page table
			for (i = 0; *(pageTable + i + 1) == -1; i += 2) {

				if (*(pageTable + i + 3) == PAGESIZE) {
					countSectors++;
					pageTable = (short int *) getSector(*(pageTable + i + 2));
					i = -2;
					continue;
				}

				countSectors++;
			}


			countSectors++;
			appendPoint = *(pageTable + i + 1);


			//	if limit on last page is not fully filled
			if (appendPoint != PAGESIZE - 1) // is some random limit
				appendPage = *(pageTable + i);

			else {

				*(pageTable + i + 1) = (short int) -1;
				i += 2;
				countSectors++;

				//	if this is not the last entry in this page table
				if (i != ((PAGESIZE / 2) - 2)) {

					if (freeList.size() == 0) {
						cout << "Out of memory. Please reduce input size or delete other files." << endl;
						return;
					}
					*(pageTable + i) = freeList.top();
					*(pageTable + i + 1) = 0;
					freeList.pop();

					appendPage = *(pageTable + i);
					appendPoint = *(pageTable + i + 1);

				//	if this is the last entry on the page table, create a new page table 
				} else {

					if (freeList.size() <= 1) {
						cout << "Out of memory. Please reduce input size or delete other files." << endl;
						return;
					}

					short int newTable = freeList.top();
					freeList.pop();
					*(pageTable + i) = newTable;
					*(pageTable + i + 1) = PAGESIZE;
						

					pageTable = (short int *) getSector(newTable);
					i = 0;

					*(pageTable + i) = freeList.top();
					*(pageTable + i + 1) = 0;
					freeList.pop();
					
					appendPage = *(pageTable + i);
					appendPoint = *(pageTable + i + 1);
					
				}

			}
			

			if (input.size() > (freeList.size() * PAGESIZE) + (PAGESIZE - appendPoint)) {
				cout << "Out of memory. Please reduce input size or delete other files." << endl;
				return;
			}


			page = getSector(appendPage);

			//	if appending data does not increase page number
			if (input.length() < (PAGESIZE - appendPoint)) {

				for (int j = 0; j < input.length(); j++) 
					*(page + appendPoint + j) = input[j];

				appendPoint += input.length();
				*(pageTable + i + 1) = appendPoint;		

				filesize = getFileSize(filename);
				resetPageTablePtr();

				cout << "Updated file size: " << filesize << endl;	
				return;

			} else {

				for (int j = appendPoint; j < PAGESIZE; j++)
					*(page + j) = input[j - appendPoint];
			

				*(pageTable + i + 1) = (short int) -1;
				remainder = PAGESIZE - appendPoint; // how much data is appended


				//	find remaing pages required to append the file and add it to page table
				limit = (input.length() - remainder) % PAGESIZE;
				appendSector = i;


				if (limit == 0) {
                    numberOfSectors = (input.length() - remainder) / PAGESIZE;
                    limit = PAGESIZE - 1;
                } else
                    numberOfSectors = (input.length() - remainder) / PAGESIZE + 1;


				numberOfSectors += countSectors;


				short int k = appendSector + 2;
				for (i = k; i < numberOfSectors * 2; i += 2) {

					if (k == ((PAGESIZE / 2) - 2)) {

						short int newTable = freeList.top();
						*(pageTable + k) = newTable;
						*(pageTable + k + 1) = PAGESIZE;
						
						freeList.pop();
						pageTable = (short int *) getSector(newTable);

						k = 0;
						i -= 2;
						continue;

					}

					*(pageTable + k) = freeList.top();
					page = getSector(*(pageTable + k));

					if (i == numberOfSectors * 2 - 2) {
						* (pageTable + k + 1) = limit;

						for (int j = 0; j < limit; j++)
							*(page + j) = input[(((i - appendSector - 2) / 2) * PAGESIZE) + j + (remainder)];
					}
					else {

						*(pageTable + k + 1) = (short int) -1;
					
						for (int j = 0; j < PAGESIZE; j++)
							*(page + j) = input[(((i - appendSector - 2) / 2) * PAGESIZE) + j + (remainder)];
						
					}

					

					freeList.pop();
					k += 2;
				}

			}
			
		}	

		resetPageTablePtr();
		filesize = getFileSize(filename);
		cout << "Updated file size: " << filesize << endl;

	}



	/* 
		function writeAt takes in positional byte as argumnent where the over writing should begin from, 
		truncates the file to that size and starts writing from that position.
	*/
	void writeAt(string data,int writeAt) {

		if (mode != "write") {
			cout << "Please open file in \"write\" mode for this function" << endl;
			return;
		}

		if (pageTable == NULL) {
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;
			return;
		}

		if (writeAt > filesize) {
			cout << "Out of bound exception. Given byte is greater than file size of" << filesize << " bytes." << endl;
			return;
		}


		truncate(writeAt-1);
		write(data);
	}



	/* 
		function truncate takes in size as integer to which the file size must be reduced to,
		and updates the file to be that size and deletes the rest of it.
	*/
	void truncate(int size) {

		if (mode != "write") {
			cout << "Please open file in \"write\" mode for this function" << endl;
			return;
		}

		if (size > filesize) {
			cout << "Out of bound exception. Given byte is greater than file size of " << filesize << " bytes." << endl;
			return;
		}
		
		int truncSectors, i;
		short int truncLimit;
		truncSectors = size / PAGESIZE;
		truncLimit = size % PAGESIZE;
		bool isLastSect = false;

		if (*(pageTable + truncSectors * 2 + 1) != -1 && *(pageTable + truncSectors * 2 + 1) != PAGESIZE)
			isLastSect = true;

		//	update limit of the final page after truncate
		*(pageTable + truncSectors * 2 + 1) = truncLimit;	

		//	push all the pages after truncate page to free list
		for (i = truncSectors*2 + 2; *(pageTable + i + 1) == -1; i += 2) {			

			freeList.push(*(pageTable + i));

			if (i == (PAGESIZE / 2 - 4) && *(pageTable + i + 3) == PAGESIZE) {
				pageTable = (short int *) getSector(*(pageTable + i + 1));
				i = -2;
			}

		}
		
		if (!isLastSect) {
			freeList.push(*(pageTable + i));
		}

		//	update file size
		filesize = getFileSize(filename);
		resetPageTablePtr();
	}



	/* 
		Move within is a cut paste function within the file. It takes 3 arguments from (starting byte), 
		size (how many bytes to be cut starting from), and to (where the chunk should be pasted).
	*/
	void moveWithin(int from, int size, int to) {

		filesize = getFileSize(filename);

		if((to > from && to < from + size ) || to < 0 || from < 0 || size < 0){
			cout << "Invalid byte arguments"<<endl;
			return;
		}
		if (mode != "write") {
			cout << "Please open file in \"write\" mode for this function" << endl;
			return;
		}

		
		if (pageTable == NULL) {
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;
			return;
		}

		if (from + size > filesize) {
			cout << "Out of bound exception. Specified chunk exceeds out of file size of" << filesize << " bytes." << endl;
			return;
		}

		mode = "read";
		//	if data is further ahead
		if (to > from) {

				string cutText = readUpto(from - 1, size);
				string middleText = readUpto(from + size - 1, to - (from + size));
				string endText = readUpto(to - 1, filesize - to);
				string writeData = middleText + cutText + endText;
				mode = "write";
				writeAt(writeData, from);

			}
		//	if data is moved prior to its original position
		else {
			string cutText = readUpto(from-1, size);
			string topText = readUpto(to - 1, from - to);
			string endText = readUpto(from + size - 1, filesize - from - size);
			string writeData = cutText + topText + endText;
			mode = "write";
			writeAt(writeData, to);
		}
		return;


	}

};





/*
	fucntion createFolder accepts argument path 
	treverses the tree from current path and if the path exists
	creates a node for subdirectory 
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

	if (createable && !folderExists(tokens.back()) && (tokens.back() != "." || tokens.back() != "..")) {

		tempFolder->subdir.push_back(new Folder(tokens.back()));
		tempFolder->subdir.back()->parent = tempFolder;

	} else
		cout << "Error: cannot create directory in specified path." << endl;

}



/*
	fucntion listDir lists all nodes of files and folders
	in current working directory
*/
void listDir() {

	if (current->subdir.size() == 0 && current->files.size() == 0) {
		cout << "Directory is empty." << endl;
		return;
	}
	
	for (int i = 0; i < current->subdir.size(); i++)
		cout << current->subdir[i]->dirName << "\t";
	
	for (int i = 0; i < current->files.size(); i++)
		cout << current->files[i]->name << "\t";

	cout << endl;	
}



/*
	function changeDir accepts argument path 
	traverses the tree related to current directory and if path exists
	updates the current working directory to specified path
*/
void changeDir(string path) {

	bool changable = true;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) {
		cout << "Invalid path entered." << endl;
		return;
	}

	tempFolder = current;
	int i = 0;

	if (tokens[0] == ".")
		i = 1;
	
	for (i; i < tokens.size(); i++) {

		if (tokens[i] == "..") {

			if (tempFolder->parent == NULL) {
				cout << "Parent of root does not exist." << endl;
				return;
			}

			tempFolder = tempFolder->parent;
		}

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
	checked if given string is numbers or not
*/
bool isNumber(string s) {

	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;

}



/*
	creates node for file and pushes it into its parent directory.
	this does not assign any pages to the file
*/
void create(string filename) {
	filename += ".txt";

	if (!fileExists(filename))
		current->files.push_back(new FileNode(filename));
	else
		cout << "A file of same name already exists." << endl;

}



/*
	the function locateFile takes in tokenized path and a check if working is for destination file,
	traverses the path to find specified file in tokens. If the path exists, the fucntion updates pointers to tempFile.
	If the path exists and file does not exists and its a destination file, the function creates the file. 
*/
void locateFile(vector<string> tokens, bool destFile) {

	found = true;
	tempFolder = current;
	int i = 0;

	if (tokens[0] == ".")
		i = 1;

    for (i; i < tokens.size(); i++) {

        if (tokens[i] == "..") {

        	if (tempFolder->parent == NULL) {
				cout << "Parent of root does not exist." << endl;
				found = false;
				return;
			}

            tempFolder = tempFolder->parent;

        } else {

            if (i != tokens.size() - 1) {
                bool checkFolder = folderExists(tokens[i]);

                if (!checkFolder) {
                    cout << "Invalid path. A folder in the specified path does not exist." << endl;
                    found = false;
                    return;
                }

            } else {

                //	in case it is found
                if (fileExists(tokens[i])) {
                	found = true;
                	return;
                }

                //	in case it is a destination file and it does not exist yet
                else if (destFile) {
                    cout << "Creating destination file..." << endl;
                    current = tempFolder;
                    create(tokens.back());
                    filePosDir = current->files.size() - 1;
                    tempFile = tempFolder->files[filePosDir];
                    cout << "New file created." << endl;
                    found = true;
                    return;
                }

                //	in case it is a source file and is not found
                else if (!destFile) {
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
            File openFile(tokenDestFile[tokenDestFile.size() - 1], "write", false);
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
	function deleteFile accepts argument filename
	goes to its page table and pushes all the pages to free list
	and removes the file node from parent node.
*/
void deleteFile(string filename) {

	if (!fileExists(filename)) {
		cout << "Error: file does not exist" << endl;
		
	} else {
		short int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;
		int pageNumber;
		int i;
		stack <int> temp;

		if (pageTable != NULL) {
			for (i = 0; *(pageTable + i + 1) == -1; i += 2) {
				pageNumber = *(pageTable + i);
				temp.push(pageNumber);
			}


			pageNumber = *(pageTable + i);
			temp.push(pageNumber);


			while (!temp.empty()) {

				freeList.push(temp.top());
				temp.pop();
			}


			freeList.push((pageTable - (short int*) start) / PAGESIZE);
			cout << freeList.top() << endl;

		}

		current->files.erase(current->files.begin() + getFileNo(filename));
	}
}



//	Prints the path of given node from root node
string pathFromRoot(Folder* dir){
	string path = dir->dirName;
	
	while(dir->parent!=NULL){
		path = dir->parent->dirName + "/" + path;
		dir = dir->parent;
	}
	return path;
}



//	list details of all the files in the given directory
void listFiles(Folder* dir) {

	if(dir->files.size() == 0)
		return;

	short int* pgTbl;
	int totalPages = 0;
	string name, pgnums = "", limit,disp = "{\n\t";

	for (int i = 0; i < dir->files.size(); i++) {
		name = dir->files[i]->name;
		
		if (dir->files[i]->pgTblPtr != NULL) {
			pgTbl = dir->files[i]->pgTblPtr;
			int j;
			
			for(j = 0; *(pgTbl + j + 1) == -1;j+=2){
				totalPages++;
				pgnums += to_string(*(pgTbl + j)) + ", ";
			}

			pgnums += to_string(*(pgTbl + j)) + " ";
			limit = to_string(*(pgTbl + j + 1));
		
		} else{
		
			pgnums = "empty";
			limit = "-";
		}
	disp += "Name: " + name + "\n\tPage numers: " + pgnums + "\n\tLimit on last page: " + limit + "\n\tTotal file size: " + to_string((totalPages*PAGESIZE) + stoi(limit)) + "\n\tPath: " + pathFromRoot(dir) + "\n}";
	}
	cout << disp;
}


//	Traverses to every folder node and calls listFiles for every folder node 
void memMap(Folder* dir) {
	listFiles(dir);
	
	if(dir->subdir.size() == 0)
		return;

	for(int i = 0;i<dir->subdir.size();i++){
		memMap(dir->subdir[i]);
	}
	
}



/*
	function getChildren accepts argument of folder node
	concatenate info about all child nodes of given folder node
	into string "out" which will be written in .dat file
*/
void getChildren(Folder* dir) {
	current = dir;
	tempFolder = dir;

	for (int i = 0; i < dir->subdir.size(); i++) {
		out += "D\t" + pathFromRoot(dir->subdir[i]) + "\n";
	}
	for (int i = 0; i < dir->files.size(); i++) {
		out += "F\t" + pathFromRoot(dir) + "/" + dir->files[i]->name + "\n";

		if (dir->files[i]->pgTblPtr != NULL) {
			File openFile(dir->files[i]->name, "read",false);
			out += "-1\n";
			out += openFile.readUpto(0, getFileSize(dir->files[i]->name));
			out += "-1\n";
		}
	}

}



/*
	function end writes the tree into .dat file
*/
void end(Folder* dir) {

	getChildren(dir);
	for (int i = 0; i < dir->subdir.size(); i++)
		end(dir->subdir[i]);

}



/*
	function readDat reads .dat file and makes the tree structure for files and directories
*/
void readDat() {

	ifstream datIn;
	vector<string> lineTokens,fileFolder,tokenizedFileName, folderPath;
	string line, fileName, content;

	datIn.open(DATPATH);


	while (!freeList.empty())
		freeList.pop();


	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);


	cout << "Memory available: " << freeList.size() * PAGESIZE << "/" << MEMSIZE << " bytes" << endl;


	while (getline(datIn, line)) {

		current = rootFolder;

		if (line[0] == 'D') {

			lineTokens = tokenize(line, '\t');
			folderPath = tokenize(lineTokens[1], '/');

			//Remove "root" from path since current directory is already root
			folderPath.erase(folderPath.begin());
			line = "";

			//serializing tokens of path to string
			for (int i = 0; i < folderPath.size(); i++) {
				line += folderPath[i] + "/";
			}

			createFolder(line);
			line = "";
		
		} else if(line[0] == 'F') {

			lineTokens = tokenize(line, '\t');
			fileFolder = tokenize(lineTokens[1], '/');
			
			fileName = fileFolder.back();

			//remove .txt from file name
			tokenizedFileName = tokenize(fileName, '.');
			fileName = tokenizedFileName[0];

			//remove root and filename from path to get file directory
			fileFolder.pop_back();
			fileFolder.erase(fileFolder.begin());
			line = "";

			//serializing tokens of path to string
			if (fileFolder.size() != 0) {
				for (int i = 0; i < fileFolder.size(); i++) {
					line += fileFolder[i] + "/";
				}
				changeDir(line);
			}
			tempFolder = current;
			//create file in durectory
			create(fileName);
			line = "";
		
		}// condition if data is to be written in file
		else if (line[0] == '-') {
			//concatenate content until -1 is encountered again
			while (getline(datIn, line)) {
				if (line[0] == '-')
					break;
				content += line + "\n";
			}
			current = tempFolder;

			//open file and write content
			File openedFile(fileName+".txt", "write", false);
			openedFile.write(content);
		}
	}
	current = rootFolder;
	datIn.close();
}



/*
	list down commands to help user
*/
void help() {

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
	cout << "mv\t\tMove file from one location to another\t\t\t\tmv ./subf/filename ../sf/" << endl;
	cout << "map\t\tDisplay memory map\t\t\t\t\t\tmap" << endl;
	cout << "end\t\tTerminate program\t\t\t\t\t\ttend" << endl;
	cout << "rdat\t\tRead and existing .dat file generated using this program\trdat" << endl;

}




/* 
	function getCommand takes no arguments in
	asks for user input (command to be run)
	takes the entered string and breaks it up by space char
	returns vector of strings containing command words
*/
vector<string> getCommand() {

	string command;

	cout << pathFromRoot(current)<<"> ";
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

	if(tokens[0] == "open"){
		if (tokens.size() == 2)
			cout << "Please input mode (read|write)" << endl;

		else if (tokens.size() == 3 && tokens[0] == "open" && (tokens[2] == "write" || tokens[2] == "read")) {

			filePosDir = getFileNo(tokens[1]);
			File openedFile(tokens[1], tokens[2], true);

	        if (!fileFound)
	        	return loop;
			
	        bool inLoop = true;

	        while (inLoop) {
		        vector<string> tokens = getCommand();

		        if (tokens.size() == 1 && tokens[0] == "wr")
		        	openedFile.write(openedFile.getInput());

		        else if (tokens.size() == 2 && tokens[0] == "wrat" && isNumber(tokens[1]))
					openedFile.writeAt(openedFile.getInput(), stoi(tokens[1]));

				else if (tokens.size() == 2 && tokens[0] == "chmod")
					openedFile.changeMode(tokens[1]);

		        else if (tokens.size() == 1 && tokens[0] == "rd")
		            openedFile.read();

		        else if (tokens.size() == 3 && tokens[0] == "rf" && isNumber(tokens[1]) && isNumber(tokens[2]))
		            cout << openedFile.readUpto(stoi(tokens[1]), stoi(tokens[2])) << endl;

		        else if (tokens.size() == 2 && tokens[0] == "trun" && isNumber(tokens[1]))
		            openedFile.truncate(stoi(tokens[1]));
				
				else if (tokens.size() == 4 && tokens[0] == "mvin" && isNumber(tokens[1]) && isNumber(tokens[2]) && isNumber(tokens[3]))
					openedFile.moveWithin(stoi(tokens[1]), stoi(tokens[2]), stoi(tokens[3]));

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
		
		filePosDir = -1;

		}

    } else if (tokens.size() == 1 && tokens[0] == "ls")
        listDir();

    else if (tokens.size() == 2 && tokens[0] == "cd")
        changeDir(tokens[1]);

    else if (tokens.size() == 2 && tokens[0] == "cr")
        create(tokens[1]);

    else if (tokens.size() == 3 && tokens[0] == "mv")
        move(tokens[1], tokens[2]);

    else if (tokens.size() == 2 && tokens[0] == "del") 
        deleteFile(tokens[1]);

    else if (tokens.size() == 2 && tokens[0] == "mkdir")
        createFolder(tokens[1]);

    else if (tokens.size() == 1 && tokens[0] == "map") 
        memMap(rootFolder);

	else if (tokens.size() == 1 && tokens[0] == "rdat")  
		readDat();	

    else if (tokens.size() == 1 && tokens[0] == "help")
        help();

    else if (tokens.size() == 1 && tokens[0] == "end"){
		dat.open(DATPATH);
		out = "";
		end(rootFolder);
		dat << out;
		dat.close();
      	loop = false;
	}

    else
        cout << "Invalid command. Type help for user guide." << endl;
    
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


	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);


	bool loop = true;
	cout << "Memory available: " << freeList.size() * PAGESIZE << "/" << MEMSIZE << " bytes" << endl;
	
	while (loop) {

		// take user input and store each word in vector
		vector<string> tokens = getCommand();

 
		// process command and call the respective function
		loop = processCommand(tokens);

	}

	free((char*) start);

	return 0;

}