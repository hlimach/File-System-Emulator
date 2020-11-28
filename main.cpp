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

//	returns PageTable pointr for file with name in current directory
int* getPtr(string name);


//tokenizes a given string with respect to delimeter
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

class FileNode {
public:
	string name;
	int* pgTblPtr;

	FileNode(string fileName){
		name = fileName;
		pgTblPtr = NULL;
	}
};

class File {
private:
	string filename;
	int* pageTable;
	char* page;

public:
	File(string name) {
		filename = name;

		pageTable = getPtr(filename);
		page = NULL;
	}


	/* 
		explain
		function
	*/
	void read() {
		int i;


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

	}



	/* 
		function readupto takes two arguments starrting bite and size of bytes to be read.
	*/
	void readUpto(int startFrom, int readUpTo) {
		//	which page number in the page table the byte will belong to
		int startPage = (startFrom / PAGESIZE);
		int startByte = startFrom % PAGESIZE;
		int endPage = (startFrom + readUpTo) / PAGESIZE;
		int limit = (startFrom + readUpTo) % PAGESIZE;
		int i;;

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
		explain
		function
	*/
	void write() {
		
		char mode;
		cout << "enter mode:" << endl;
		cin >> mode;
		string input, line;

		int count = 0;
		while (getline(cin, line)) {
			if (line == "-1")
				break;
			input += line;
			if (count != 0)
				input += "\n";
			count++;
		}

		cout << input.length() << endl;
		int numberOfSectors = 0, limit, appendPoint, appendPage, appendSector, remainder, countSectors;

		switch (mode) {
		case('w'):

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
			break;

		case('a'):
			
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
			break;
		}	
	}

	/* 
		explain
		function
	*/
	void writeAt(int writeAt) {

			truncate(writeAt);
			write();
	}



	/* 
		explain
		function
	*/
	void truncate(int size) {
		int truncSectors, truncLimit,i,temp,count=0;
		truncSectors = size / PAGESIZE + 1;
		truncLimit = size % PAGESIZE;
		bool check;

		for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
			count++;
			if ( count == truncSectors) {
				*(pageTable + i + 1) = truncLimit;
				check = 1;
			}
			else if( count > truncSectors){
				freeList.push(*(pageTable + i));
			}
			
		}
		
		if (check) {
			freeList.push(*(pageTable + i));
		} else {	
			*(pageTable + i) = truncLimit;
		}
	}

};

class Folder {
public:
	string dirName;
	vector<Folder* > subdir;
	vector<FileNode*> files;
	Folder* parent;

	Folder(string name) {
	
		dirName = name;
		parent = NULL;
	
	}
};

Folder* rootFolder = new Folder("root");
Folder* current, * tempFolder;

void createFolder(string path) {

	tempFolder = current;
	vector<string> tokens = tokenize(path, '/');
	int j;
	
	for (int i = 0; i < tokens.size() - 1; i++) {
		if (tokens[i] == "..")
			tempFolder = tempFolder->parent;
		else {

			for (j = 0; j < tempFolder->subdir.size(); j++) {
				if (tempFolder->subdir[j]->dirName == tokens[i])
					break;
			}

			tempFolder = tempFolder->subdir[j];

		}
	
	}
	tempFolder->subdir.push_back(new Folder(tokens.back()));
	tempFolder->subdir.back()->parent = tempFolder;
	

}

void listDir() {

	cout << "Here is the list of files and folders in this directory" << endl;
	
	for (int i = 0; i < current->subdir.size(); i++)
		cout << current->subdir[i]->dirName << "\t\t";
	
	for (int i = 0; i < current->files.size(); i++)
		cout << current->files[i]->name << "\t";
}

void changeDir(string path) {

	vector<string> tokens = tokenize(path, '/');
	int j;
	
	for (int i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "..")
			current = current->parent;
		else {

			for (j = 0; j < current->subdir.size(); j++) {
				if (current->subdir[j]->dirName == tokens[i])
					break;
			}

			current = current->subdir[j];

		}
	}

}

/* 
	explain
	function
*/
bool isNumber(string s) {

	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;

}



/* 
	explain
	function
*/
void create(string filename) {

	int freeSect = freeList.top();
	freeList.pop();

	char* sector = getSector(freeSect);
	current->files.push_back(new FileNode(filename));
	current->files.back()->pgTblPtr = (int*)sector;

}

int* getPtr(string name) {
	
	int i;
	for (i = 0; i < current->files.size(); i++) {
		if (current->files[i]->name == name)
			break;
	}
	return current->files[i]->pgTblPtr;
}

/* 
	function getFileSize takes in filename as argument 
	accesses the files' page table and iterates it
	returns the file size in bytes
*/
int getFileSize(string filename) {

	int* pageTable = getPtr(filename);
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



/* 
	explain
	function
*/
void deleteFile(string filename) {

	int* pageTable = getPtr(filename);
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

}



/* 
	explain
	function
*/
void list() {
	cout << "in list()" << endl;
	//setFilename();
}



/* 
	explain
	function
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


	if (tokens[0] == "open") {

        File openedFile(tokens[1]);
        bool inLoop = true;


        while (inLoop) {
	        vector<string> tokens = getCommand();

	        if (tokens[0] == "wr") {
	            openedFile.write();
	        } else if (tokens[0] == "wrat") {
	            openedFile.writeAt(stoi(tokens[1]));
	        } else if (tokens[0] == "rd") {
	            openedFile.read();
	        } else if (tokens[0] == "rf") {
	            openedFile.readUpto(stoi(tokens[1]),stoi(tokens[2]));
	        } else if (tokens[0] == "trun") {
	            openedFile.truncate(stoi(tokens[1]));
	        } else if (tokens[0] == "close") {
	            cout << "File closed." << endl;
	            inLoop = false;
	        } else if (tokens[0] == "end") {
	            cout << "Close file before ending program." << endl;
	        } else if (tokens[0] == "help") {
	        	help();
	    	} else {
	            cout << "Invalid command. Type help for user guide." << endl;
	        }	
    	}

    } else if (tokens[0] == "ls") {
        listDir();
    } else if (tokens[0] == "cd") {
        changeDir(tokens[1]);
    } else if (tokens[0] == "cr") {
        create(tokens[1]);
    } else if (tokens[0] == "mv") {
        //move();
    } else if (tokens[0] == "del") {
        deleteFile(tokens[1]);
    } else if (tokens[0] == "mkdir") {
        createFolder(tokens[1]);
    } else if (tokens[0] == "map") {
        //memMap();
    } else if (tokens[0] == "help") {
        help();
    } else if (tokens[0] == "end") {
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
	

	// while (loop) {
	// 	int command, size;
	// 	string readUpto;

	// 	cout << "Please enter one of the following command integer:" << endl;
	// 	cout << "1: Create(filename) \t\t\tCreate a text file of name <filename>" << endl;
	// 	cout << "2: Delete(filename) \t\t\tDelete a text file of name <filename>" << endl;
	// 	cout << "3: Open(filename) \t\t\t\tOpen a text file of name <filename>" << endl;
	// 	cout << "4: Close(filename) \t\t\t\tClose an opened text file of name <filename>" << endl;
	// 	cout << "5: List files \t\t\t\t\tList all the files in the current directory" << endl;
	// 	cout << "6: Read(filename) \t\t\t\tRead a text file of name <filename> from start" << endl;
	// 	cout << "7: Read(filename, readUpto) \tRead a text file of name <filename> from the byte number given by <readUpto>" << endl;
	// 	cout << "8: Write(filename,mode) \t\tWrite to text file of name <filename> with the given mode\n\t\t\t\t\t\t\t\tMode: a (append), w (write)" << endl;
	// 	cout << "9: End Program" << endl;
	// 	cout << "\nEnter command: ";
	// 	cin >> command;


	// 	switch (command) {

	// 		case(1):
	// 			create();
	// 			break;

	// 		case(2):

	// 			filename = setFilename();
	// 			deleteFile(filename);

	// 			break;

	// 		case(3):
	// 			open();
	// 			break;

	// 		case(4):
	// 			close();
	// 			break;

	// 		case(5):
	// 			list();
	// 			break;

	// 		case(6):
	// 			read();
	// 			break;

	// 		case(7):

	// 			filename = setFilename();
	// 			size = getFileSize(filename);

	// 			while (true) {
	// 				cout << "Enter starting byte number (within file size:" << size <<  " bytes) : ";
	// 				cin >> readUpto;

	// 				if (isNumber(readUpto)) {

	// 					if (stoi(readUpto) <= size) {
	// 						read(filename, stoi(readUpto));
	// 						break;
	// 					} 
	// 					else 
	// 						cout << "Enter a valid byte number!" << endl;
						
	// 				}
	// 				else
	// 					cout << "Enter a valid byte number!" << endl;

	// 			}

	// 			break;

	// 		case(8):
	// 			write();
	// 			break;

	// 		case(9):
	// 			loop = false;
	// 			break;

	// 		default:
	// 			cout << "Please enter a valid integer command!" << endl;
	// 			break;

	// 	}
	// }