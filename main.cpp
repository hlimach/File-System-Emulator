#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <assert.h>

#define PAGESIZE 16						 /* Size of each page in memory. */
#define MEMSIZE 16384					 /* Total memory reserved for file data. */
#define NUMPAGES (MEMSIZE / PAGESIZE)	 /* Total pages in memory. */
#define LASTENTRY ((PAGESIZE / 2) - 1)	 /* Last entry point for a page table. */
#define MAXENTRIES ((PAGESIZE / 2) - 3)	 /* Maximum entries possible in a page table */
#define DATPATH "solution.dat"

using namespace std;


/* Points to the starting location of the allocated memory. */
const char* start;


/* A stack of free/ unused page numbers in ascending order maintained by the system. 
   Whenever a new page is to be assigned, this stack is popped. */
stack <short int> freeList;


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


/* Class for file nodes in the tree structure to be maintained for memory management. 
   It is a leaf node. */
class 
FileNode 
{
public:
	string name;
	short int* pgTblPtr;

	/* Pointer pgTblPtr points to this files' page table, but is assigned NULL upon 
	   Creation, and will only point to one once data is written into this file. */
	FileNode (string fileName) 
	{
		name = fileName;
		pgTblPtr = NULL;
	}
};


/* Class for folder nodes in the tree structure to be maintained for memory management.
   It maintains vectors for both subfolders and files within this folder. Stores a 
   Pointer to its parent as folder nodes are used in tree traversal. */
class 
Folder 
{
public:
	string dirName;
	vector<Folder*> subdir;
	vector<FileNode*> files;
	Folder* parent;

	Folder (string name) 
	{
		dirName = name;
		parent = NULL;
	}
};


/* The starting folder of this system : "root". */
Folder* rootFolder = new Folder("root");


/* To write into .dat file at end of program to save file info. */
ofstream dat;

/* To read from .dat file whenever required. */
ifstream datIn;

/* String is used to gather all text to be written into .dat file. */
string out;


/* Current folder pointer stores pointer to current working directory, whereas 
   tempFolder pointer is used for temporary actions/ tracking. */
Folder* current, * tempFolder;


/* Pointer tempFile points to File node to store its value if found. */
FileNode* tempFile;


/* Int filePosDir tells us which index in the vector of the current directory the 
   Needed file will be found. */
int filePosDir;


/* To let functions know if the needed file is found. */
bool fileFound, found;


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


void
enterDat(string path,bool file,string name)
{
	string prevText = "",line = "";
	datIn.open(DATPATH);
	while(getline(datIn,line))
	{
		prevText += line + "\n";
	}
	datIn.close();

	if(file)
		prevText += "F\t" + path + "/" + name; 
	else
		prevText += "D\t" + path + "/" + name; 
	
	dat.open(DATPATH);
	dat << prevText;
	dat.close();
}


void
moveDat(string oldPath,string newPath)
{
	string line = "",prevText = "",data = "";
	datIn.open(DATPATH);
	while(getline(datIn,line)){
		if(line[0] == 'F')
			if(line.substr(2,line.size()-2) == oldPath)
				break;
		prevText+=line + "\n";
	}
	
	getline(datIn,line);
	
	if(line[0] == '\a'){
		data += line + "\n";
		do{
			getline(datIn,line);
			data += line + "\n";
		}while(line[0]!='\a');
		getline(datIn,line);
	}

	
	prevText+=line+"\n";
	while(getline(datIn,line))
		prevText+=line+"\n";

	datIn.close();
	dat.open(DATPATH);
	dat << prevText + "F\t" + newPath + "\n" + data.substr(0,data.length()-1);
	dat.close();
}

void
removeDat (string path,bool file)
{
	string line = "", prevText = "", endText = "";
	datIn.open(DATPATH);
	while(getline(datIn,line)){
		if(line[0] == 'F' || line[0] == 'D')
			if(line.substr(2,line.size()-2) == path)
				break;
		prevText+=line + "\n";
	}
	
	if (datIn.eof()) {

		datIn.close();
		dat.open(DATPATH);
		dat << prevText;
		dat.close();
		return;
	}
	getline(datIn,line);
	
	if(file){
		if(line[0] == '\a'){
			do{
				getline(datIn,line);
			}while(line[0]!='\a');
			getline(datIn,line);
		}
	}
	endText+=line+"\n";
	while(getline(datIn,line))
		endText+=line+"\n";

	datIn.close();
	dat.open(DATPATH);
	dat << prevText + endText.substr(0,endText.length()-1);
	dat.close();
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


/* File object is created whenever the user must interact with any file. It has all 
   The functions for read, write, truncate, and move within. The user must open a file 
   By providing the mode in which they wish to interact with it. The mode can be 
   Changed later using 'chmod' command. */
class 
File 
{
private:
	string filename, mode;
	short int* pageTable;
	char* page;
	int fileSize;
	bool printInfo;

public:

	/* Constructor takes in the files' name, the desried mode, and a boolean value
	   For if the file must be opened and changed in silent mode (only for interal calls, 
	   Not in user control. User opened files are never opened in silent mode.). */
	File (string name, string md, bool printInf) 
	{
		fileFound = fileExists(name);

		if (fileFound) 
		{
			filename = name;
			pageTable = current->files[getFileNo(filename)]->pgTblPtr;
			fileSize = getFileSize();
			mode = md;
			page = NULL;
			printInfo = printInf;

			if (printInfo)
				cout << "File opened: " << filename << ", mode: " << mode << ", size: "
					 << fileSize << " bytes." << endl;
		}

		else
			cout << "The file does not exist." << endl;

	}


	/* For any given page number within memory, its pointer is returned. */
	char* 
	getPagePtr (short int x) 
	{
		return (char*)start + (PAGESIZE * x);
	}


	/* The first entry in any page table is its' limit i.e where the last pages' last 
	   Byte is (where it is filled upto). */
	short int 
	getByteLimit () 
	{
		return *(pageTable);
	}


	/* The second entry in any page table is the number of pages it has i.e how many
	   Pages are kept in the page table to assist iteration. */
	short int 
	getPageCount () 
	{
		return *(pageTable + 1);
	}


	/* The last entry in a page table is the page number of next linked page table. */
	short int 
	getNextPageTableNum () 
	{
		return *(pageTable + LASTENTRY);
	}


	/* Sets/ Updates the current page tables' limit to be the given value. */
	void 
	setByteLimit (int limit) 
	{
		*(pageTable) = limit;
	}


	/* Sets/ Updates the current page tables' page count to be the given value. */
	void 
	setPageCount (int count) 
	{
		*(pageTable + 1) = count;
	}


	/* Sets/ Updates the current page tables' last entry to be the given value
	   Which indicates which page number of page table it is next linked to. */
	void 
	setNextPageTableNum (int nextPageNum) 
	{
		*(pageTable + LASTENTRY) = nextPageNum;
	}	


	/* Sets/ Updates the page tables' pointer to point to a new location in memory. */
	void 
	setPageTablePtr (short int * pageTbl) 
	{
		pageTable = pageTbl;
	}


	/* Resets page table pointer to point to the starting page table. */
	void 
	resetPageTblPtr () 
	{
		pageTable = current->files[getFileNo(filename)]->pgTblPtr;
	}


	/* Iterates all the page tables of the given file and returns the total size in 
	   Bytes of the current file. */
	int 
	getFileSize () 
	{
		resetPageTblPtr();
		if (pageTable == NULL)
			return 0;
		else 
		{
			/* Obtain the page number of the starting page table, and send into
			   The do...while, which extracts page table using its number, adds
			   Its limit to 'size' variable, gets the next page table number, and
			   Does the same process until the next page table number is '-1', 
			   Which indicates that there are no further page tables linked. */
			int nextPageTableNum = getPageNum((char *) pageTable), size = 0;

			do 
			{
				pageTable = (short int*) getPagePtr(nextPageTableNum);
				short int pageNums = getPageCount(), limit = getByteLimit();
				size += (((pageNums - 1) * PAGESIZE) + limit);
				nextPageTableNum = getNextPageTableNum();

			} while (nextPageTableNum != -1);

			resetPageTblPtr();
			return size;
		}
	}



	

	/* This function is invoked when user wants to change mode from which file is
	   Currently opened in. If the argument is valid (read or write), then it is
	   Successfully changed, allowing user access to functions that are available
	   Within that mode. Otherwise, an error message is printed. */
	void 
	changeMode (string md) 
	{
		if (md == "read" || md == "write") 
		{
			mode = md;
			if(printInfo){
				cout << "File opened: " << filename << ", mode: " << mode << ", size: " <<
				 fileSize << " bytes." << endl;
			}
		}
		else
			cout << "Please enter a valid mode (read|write)." << endl;
	}


	/* For the current page table, this function takes in argument to firstly specify 
	   A read or write operation. If it is a write opeartion, the input string is 
	   Written into the specified location, otherwise the specified locations' data is
	   Simply returned to calling function. */
	string 
	pageTableData (string input, int startPage, int lastPage, int startByte, int limit,
		 int &byteCount, bool read) 
	{
		/* stores data that is being read. */
		string data = "";

		/* Value startPage tells the function which page to start iteration from, and
		   Value lastPage tells upto which page it should go. */
        for (int i = startPage + 1; i <= lastPage + 1; i++) 
        {
            page = getPagePtr(*(pageTable + i));

            /* If the iteration is not on the last page, the the data is read from 
               Start to end of this page. */
            if (i != (lastPage + 1)) 
            {
            	/* If a starting byte is specified, then for the first page the data
            	   Is read from this byte onwards. Afterwards it is reset to 0 and
            	   For every page ahead the data is read from the starting. */
                for (int j = startByte; j < PAGESIZE; j++)
                {
                    if (read)
                        data += *(page + j);
                    else
                        *(page + j) = input[byteCount];

                    byteCount++;
                }
                startByte = 0;
            }

            /* If the iteration is on the last page, the the data is read from here upto
               The defined limit. */
            else 
            {
                for (int j = startByte; j < limit; j++) 
                {
                    if (read)
                        data += *(page + j);
                    else
                        *(page + j) = input[byteCount];

                    byteCount++;
                }
                startByte = 0;
            }
        }
		return data;
    }


    /* Function takes in starting byte, and chunk size as an argument, and manipulates
       It according to read/ write bool. */
    string
    chunkManipulation (int startFrom, int chunkSize, bool read, string data) 
    {
    	string text = "";
    	/* Which page number in the page table the byte will belong to. */
		int startPage, endPage, neededPages, temp = 0;
		int startByte = startFrom % PAGESIZE;
		int limit = (startFrom + chunkSize) % PAGESIZE;

		startPage = startFrom / PAGESIZE + 1;

		if (limit == 0)
		{
			limit = PAGESIZE;
			endPage = (startFrom + chunkSize) / PAGESIZE;
		}
		else 
			endPage = (startFrom + chunkSize) / PAGESIZE + 1;

		neededPages = endPage - startPage + 1;

		/* Loop ahead to go to page table with the needed starting page. */
		while (startPage > MAXENTRIES) 
		{
			pageTable = (short int*) getPagePtr(getNextPageTableNum());
			startPage -= MAXENTRIES;
		}

		/* If the reading is within a page table then send all arguments 
		   At once. */
		if ((PAGESIZE - startByte) + ((MAXENTRIES - startPage) * PAGESIZE) > 
			 startFrom + chunkSize) 
			text += pageTableData(data, startPage, endPage, startByte, limit, temp,
				 read);
		

		/* If the reading is not within a page table then send starting byte 
		   And starting page in the beginning, loop until needed pages are in
		   The last page table limits, and then send it the limit. */
		else 
		{
			int pageTablePageNum = getPageNum((char *) pageTable);
			do 
			{
				pageTable = (short int*) getPagePtr(pageTablePageNum);
				text += pageTableData(data, startPage, MAXENTRIES, startByte, PAGESIZE,
					 temp, read);

				pageTablePageNum = getNextPageTableNum();
				neededPages -= (MAXENTRIES - startPage + 1);
				startPage = 1;
				startByte = 0;
			} while (neededPages > MAXENTRIES);
			
			pageTable = (short int*) getPagePtr(pageTablePageNum);
			text += pageTableData(data, 1, neededPages, 0, limit, temp, read);
		}
		return text;
    }


	/* Function read takes two arguments starting bite and size of bytes to be read, 
	   And prints out the content within that limit. */
	string 
	read (int startFrom, int readUpTo) 
	{
		string text = "";
		resetPageTblPtr();
        if (mode != "read") 
        {
            cout << "Please open file in \"read\" mode for this function." << endl;
            return "";
        }
        else if (pageTable == NULL) 
        {
            cout << "The file has no content to display." << endl;
            return "";
        }
        else if ((readUpTo - startFrom > fileSize) || (startFrom + readUpTo > fileSize) || (startFrom < 0)) 
		{
			cout << "Out of bound exception. Given limit exceeds total file limit at "
				 << fileSize << " bytes." << endl;
			return "";
		}
        else if (startFrom == 0 && readUpTo == fileSize)
        {
        	/* The current page tables' number is taken and sent into the
        	   Do...while loop, which runs upto the last page table. It prints
        	   Data within each page table according to its page count and limit,
        	   And once this iteration is done, the page table pointer is reset
        	   To point to the starting table. */
            int nextPageTableNum = getPageNum((char *) pageTable), temp = 0;

            do 
            {
                pageTable = (short int*) getPagePtr(nextPageTableNum);
                text += pageTableData("", 1, getPageCount(), 0, getByteLimit(), temp,
                	 true);
                nextPageTableNum = getNextPageTableNum();

            } while (nextPageTableNum != -1);
            resetPageTblPtr();
	    	return text;
        }
        else 
        {
        	text = chunkManipulation(startFrom, readUpTo, true, "");
        	resetPageTblPtr();
			return text;
        }
        
    }



	void
	updateDat(string path)
	{

		datIn.open(DATPATH);
		string line = "",topText = "",data = "",endText = "";
		
		//read top text untill file is found
		while(getline(datIn,line)){
			
			if(line[0] == 'F')
				if(line.substr(2,line.size()-2) == path)
					break;
			
			topText += line + "\n";
		}

		topText += line +"\n";

		if(fileSize!=0)
			data = "\a\n" + read(0,fileSize) + "\n\a\n";

		if(datIn.eof()){	
			
			datIn.close();
			dat.open(DATPATH);
			dat << topText + data;
			dat.close();
			return;
		}

		//skip the data stored in that file if any
		getline(datIn,line);
		if(line[0] == '\a'){
			do{
				getline(datIn,line);
			}while(line[0]!='\a');
			getline(datIn,line);
		}
		
		//read after the file is found and data is skipped
		endText += line + "\n";

		while(getline(datIn,line))
			endText += line + "\n";

		datIn.close();
		dat.open(DATPATH);
		dat << topText + data +	endText.substr(0,endText.length()-1);
		dat.close();

	}


	/* Makes necessary changes and calls updateDat function */
	void
	callUpdateDat(){
		bool prevPrintInfo = printInfo;
		printInfo = false;
		changeMode("read");
		updateDat(pathFromRoot(current)+"/"+filename);
		changeMode("write");
		printInfo = prevPrintInfo; 
	}


	/* Prompts user to input data for the write functions. It stops taking input once 
	   The user enters '-1' in a new line and presses enter. */
	string 
	getInput (ifstream& in) 
	{
		if (mode != "write") 
		{
			cout << "Please open file in \"write\" mode for this function." << endl;
			return "";
		} 
		else 
		{
			string input, line;
			while (getline(cin, line)) 
			{
				if (line == "-1")
					break;

				input += line;
				input += "\n";
			}
			input = input.substr(0, input.size() - 1);
			return input;
		}
	}


	/* Function assigns pages to a file as it is being filled i.e. according to needed
	   Pages within a page table. It pops a value from free list and assigns it to the
	   Page table entry. */
	void 
	assignPages (int start, int last) 
	{
		for (int i = start + 1; i <= last + 1; i++) 
		{
			*(pageTable + i) = freeList.top();
			freeList.pop();
		}
	}


	/* Function calculates the limit and needed pages according to the input size. */
	void 
	calcLimit (int &neededPages, short int &limit, int inputSize, int macro) 
	{
		limit = inputSize % macro;
		if (limit == 0) 
		{
            limit = macro;
            neededPages = inputSize / macro;
        }
        else
            neededPages = inputSize / macro + 1;
	}


	/* For a write operation, once the limit, and needed pages are calculated, this
	   Function creates page tables up until when there are no more pages needed.
	   It fills them us as it creates them. */
	void 
	createPageTableAndWriteData (string input, int neededPages, int limit, 
		 int &byteCount, int pageTablePageNum) 
	{
		do 
		{
            pageTable = (short int*) getPagePtr(pageTablePageNum);

            /* If the needed pages are more than the maximum entries allowed in
               A page table, then this control branch is taken. It assigns the
               Pages, writes into them, assigns the page count to be maximum 
               Entries, and assigns it a new page table it is further linked to. */
            if(neededPages >= MAXENTRIES) 
            {
            	/* It sets the limit according to exact amount of needed pages (if 
            	   The needed pages are still more, it means the max limit will be 
            	   Assigned to show that page table is completely filled. Otherwise, 
            	   The limit of the input is assigned and this page table is not 
            	   Further linked to another page table). */
                if (neededPages != MAXENTRIES) 
                {
                    setByteLimit((short int) PAGESIZE);
                    setNextPageTableNum(freeList.top());
                    freeList.pop();
                }
                else 
                {
                    setByteLimit((short int) limit);
                    setNextPageTableNum((short int) -1);
                }

                setPageCount((short int) MAXENTRIES);
                assignPages(1, MAXENTRIES);
                pageTableData(input, 1, MAXENTRIES, 0, getByteLimit(), byteCount, false);

                neededPages -= MAXENTRIES;
                pageTablePageNum = getNextPageTableNum();
            }
            /* If the needed pages are less than the maximum entries allowed in a
               Page table, then this control branch is taken. It assigns the pages
               Needed, writes into them, assigns the page count to be the amount of
               Pages that were needed in completing the data writing and does not
               Further link this page table to another one. */
            else 
            {
                setByteLimit((short int) limit);
                setPageCount((short int) neededPages);
                assignPages(1, neededPages);
                pageTableData(input, 1, neededPages, 0, limit, byteCount, false);
                setNextPageTableNum((short int) -1);
                neededPages = 0;
            }

        } while (neededPages != 0);

        assert (byteCount == input.length());
	}


	/* Write function is invoked whenever user wishes to write into a new file or to
	   Append to an existing file. */
	void 
	write(string input , bool updatedat) 
	{

		int neededPages = 0, byteCount = 0;
        short int limit;

		if (mode != "write") 
			return;
		
		else if (input.size() > (freeList.size() * PAGESIZE)) 
        {
            cout << "Not enough memory available. " << 
            	 "Please reduce input size or delete other files." << endl;
            return;
        }

		/* If it is a new file i.e. empty, then this control branch is taken. */
		else if (pageTable == NULL) 
		{
			/* Assigns this file a new page table. */
            short int pageTablePageNum = freeList.top();
            freeList.pop();
            char* page = getPagePtr(pageTablePageNum);
            current->files[getFileNo(filename)]->pgTblPtr = (short int*) page;
            pageTable = (short int*) page;

            /* Calculates needed pages and limit, then it calls onto function which
               Handles creating page tables and writing process according to amount
               Of needed pages. */
            calcLimit(neededPages, limit, input.length(), PAGESIZE);
            createPageTableAndWriteData(input, neededPages, limit, byteCount, 
            	 pageTablePageNum);
        }

        /* If it is a not new file, then this control branch is taken (append). */
        else 
        {
        	/* Iterates to the last page table that is assigned to this file. */
            resetPageTblPtr();
            while (getNextPageTableNum() != -1) 
                pageTable = (short int*) getPagePtr(getNextPageTableNum());

            /* Checks for available space in this page table, and calculates
               Needed pages and limit for this input. */
            int freeSpace = (MAXENTRIES * PAGESIZE) - (((getPageCount() - 1) * PAGESIZE)
            	 + getByteLimit());
            calcLimit(neededPages, limit, (int) (input.length() - (PAGESIZE - 
            	 getByteLimit())), PAGESIZE);

            /* If the available space is more than the input size, then only the 
               Current page table must be filled upto the needed amount. */
            if (freeSpace >= input.length()) 
            {
				/* If the input size is less than the space available within
				   A single page, then only this page must be written in, so the
				   needed pages are assgined 0 so no more pages are assigned. */
				if (input.length() < PAGESIZE - getByteLimit()) 
				{
					neededPages = 0;
                    limit = getByteLimit() + input.length();
				} 
				/* If the input size is more than the space available within a single
				   Page, then however many pages needed ahead are assigned, and the
				   Writing process is handled by calling pageTableData function. */
                assignPages(getPageCount() + 1, getPageCount() + neededPages);
                pageTableData(input, getPageCount(), getPageCount() + neededPages, 
                	 getByteLimit(), limit + 1, byteCount, false);
                setByteLimit(limit);
                setPageCount(getPageCount() + neededPages);
            }

            /* If the available space is less than the input size, then more page
               Tables will be need to assigned according to needed pages, so this
               Control branch is taken. */
            else 
            {
                assignPages(getPageCount() + 1, MAXENTRIES);
                pageTableData(input, getPageCount(), MAXENTRIES, getByteLimit(), 
                	 PAGESIZE, byteCount, false);
                neededPages -= (MAXENTRIES - getPageCount());
                setByteLimit(PAGESIZE);
                setPageCount(MAXENTRIES);
                setNextPageTableNum(freeList.top());
                freeList.pop();
                int pageTablePageNum = getNextPageTableNum();
                createPageTableAndWriteData(input, neededPages, limit, byteCount, 
                	 pageTablePageNum);
            }
        }

		fileSize = getFileSize();
		if(updatedat)
			callUpdateDat();
		
		if (printInfo)
			cout << "Updated file size: " << fileSize << endl;
	}


	/* WriteAt function is invoked whenever user wishes to write into file at a
	   Specific (over writing). */
	void 
	writeAt (string data, int writeAt) 
	{
		if (mode != "write") 
			return;

		else  if (pageTable == NULL) 
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;

		else if (writeAt > fileSize || writeAt < 0) 
			cout << "Out of bound exception. Given byte is greater than file size of " 
				<< fileSize << " bytes." << endl;

		else if (data.length() + writeAt > fileSize) 
		{
			truncate(writeAt - 1);
			write(data,true);
		}
		else 
			chunkManipulation(writeAt, data.length(), false, data);
		
		callUpdateDat();
		resetPageTblPtr();
	}


	/* Truncate takes the size that the current file must be reduced to, and
	   Essentially 'deletes' from that point onwards. */
	void 
	truncate (int size) 
	{
		if (mode != "write") 
		{
			cout << "Please open file in \"write\" mode for this function" << endl;
			return;
		}
		else if (size > fileSize) 
		{
			cout << "Out of bound exception. Given byte is greater than file size of "
				 << fileSize << " bytes." << endl;
			return;
		}
		else
		{
			short int reduceTo = size;
            resetPageTblPtr();

            /* First calculate the page number in all the page tables where this given
               Size ends. */
            int pageNumOfGivenByte;
            calcLimit(pageNumOfGivenByte, reduceTo, size, PAGESIZE);
            short int pageNumOfGivenB = (short int) pageNumOfGivenByte;
            
            /* Calculate the page table number in all the page tables where this page
               Will be located. */
            int whichPageTable;
            calcLimit(whichPageTable, pageNumOfGivenB, pageNumOfGivenByte, MAXENTRIES);

            /* Gets current page table number, and iterates until the page table with
               The desired page is reached. */
            int nextPageTableNum = getPageNum((char *) pageTable);
            for (int i = 1; i < whichPageTable; i++)
            {
                pageTable = (short int*) getPagePtr(nextPageTableNum);
                nextPageTableNum = getNextPageTableNum();
            }
            
            /* Pointer to this page table is obtained, and all remaining pages in this
               Page table are added back to free list. */
            pageTable = (short int*) getPagePtr(nextPageTableNum);
            for (int i = pageNumOfGivenB + 2; i <= getPageCount() + 1; i++)
                freeList.push(*(pageTable + i));

            /* This page tables' limit, page count, and next page table number are
               Updated according to its reduced size. Next page table number is set
               To -1 to indicate no further linking. */
            setByteLimit(reduceTo);
            setPageCount(pageNumOfGivenB);
            nextPageTableNum = getNextPageTableNum();
            setNextPageTableNum((short int) -1);

	        /* For as long as there are linked page tables ahead, this loop goes into
	           Each one and adds all their pages into free list, and at the end adds
	           The page tables' page number to the free list as well. */
	        while (nextPageTableNum != -1) 
	        {
	            pageTable = (short int*) getPagePtr(nextPageTableNum);

	            for (int i = 2; i <= getPageCount() + 1; i++)
	                freeList.push(*(pageTable + i));

	            freeList.push(nextPageTableNum);
	            nextPageTableNum = getNextPageTableNum();
	        }

	        fileSize = getFileSize();
	        if (printInfo)
	            cout << "File size reduced to " << fileSize << " bytes." << endl;
	    }
		callUpdateDat();
    }


	/* Move within is invoked for a cut paste action where user specifies where to cut
	   From, for how many bytes (size of text chunk that must be cut), and where it
	   Should be pasted. */
	void 
	moveWithin (int from, int size, int to) 
	{
		fileSize = getFileSize();

		if ((to > from && to < from + size) || to < 0 || from < 0 || size < 0) 
		{
			cout << "Invalid byte arguments." << endl;
			return;
		}
		else if (mode != "write") 
		{
			cout << "Please open file in \"write\" mode." << endl;
			return;
		}
		else if (pageTable == NULL) 
		{
			cout << "Invalid command. Cannot call 'Write At' on an empty file." << endl;
			return;
		}
		else if (from + size > fileSize) 
		{
			cout << "Out of bound exception. Specified chunk exceeds out of file size of "
				  << fileSize << " bytes." << endl;
			return;
		}
		else
		{
			mode = "read";
			/* If the paste position is ahead of the cut position. */
			if (to > from) 
			{
				string cutText = read(from - 1, size);
				string middleText = read(from + size - 1, to - (from + size));
				string writeData = middleText + cutText;
				mode = "write";
				printInfo = false;
				writeAt(writeData, from - 1);
				printInfo = true;
			}
			/* If the paste position is before the cut position. */
			else 
			{
				string cutText = read(from - 1, size);
				string topText = read(to - 1, from - to);
				string writeData = cutText + topText;
				mode = "write";
				printInfo = false;
				writeAt(writeData, to - 1);
				printInfo = true;
			}
			resetPageTblPtr();
		}
	}


};


/* Traverses tree according to given path. If a folder within the path is not found, 
   The loop is exited, and the boolean found is set to false. */
bool
traverseTree (int i, vector<string> tokens, bool change)
{
	int lim = tokens.size() - 1;
	if (change)
		lim++;

	for (i; i < lim; i++)
    {
        if (tokens[i] == "..")
        {
            if (tempFolder->parent == NULL)
            {
                cout << "Parent of root does not exist." << endl;
                return false;
            }
            tempFolder = tempFolder->parent;
        }
        else
        {
            bool folderFound = folderExists(tokens[i]);
            if (!folderFound)
            {
                return false;
            }
        }
    }
    return true;
}


/* Fucntion createFolder accepts argument path for folder creation traverses the tree
   From current path, and if the path exists it creates a node for subdirectory. */
void 
createFolder (string path,bool updatedat) 
{
	bool createable = true;
	tempFolder = current;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		cout << "Invalid path entered." << endl;
		return;
	}
	else {
		createable = traverseTree(0, tokens, false);

		/* If the path exists, a folder of the same name does not already exist, and
		   The name of the folder is neither '.' nor '..', then a folder at the 
		   Specified path is successfully created. */
		if (createable && !folderExists(tokens.back()) && (tokens.back() != "." ||
			 tokens.back() != "..")) 
		{
			tempFolder->subdir.push_back(new Folder(tokens.back()));
			tempFolder->subdir.back()->parent = tempFolder;
		}
		else
			cout << "Error: cannot create directory in specified path." << endl;
	}
	tempFolder = current;
	if(updatedat)
		enterDat(pathFromRoot(current),false,path);

}


/* Function changeDir accepts argument path traverses the tree related to current 
   Directory and if path exists updates the current working directory to specified 
   Path. */
void 
changeDir (string path) 
{
	bool changable = true;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		cout << "Invalid path entered." << endl;
		return;
	}
	else 
	{
		tempFolder = current;
		int i = 0;

		/* If the first token is '.', then it works in current directory. */
		if (tokens[0] == ".")
			i = 0;

		changable = traverseTree(i, tokens, true);

		if (changable)
			current = tempFolder;
		else
			cout << "Error: cannot change directory to specified path." << endl;
	}
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



/* Creates a file node at the current working directory and pushes it into its' files
   List. This does not assign any pages. */
void 
create (string filename,bool updatedat) 
{
	filename += ".txt";

	if (!fileExists(filename)) 
		current->files.push_back(new FileNode(filename));
	else
		cout << "A file of same name already exists." << endl;
	if(updatedat)
		enterDat(pathFromRoot(current),true,filename);
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



/* Takes in file name as an argument. Works on current working directory. If a file
   Of the same name exists, it is deleted. */
void 
deleteFile (string filename) 
{
	/* If a file of the given name does not exist in current working directory. */
	if (!fileExists(filename)) 
	{
		cout << "Error: file does not exist" << endl;
		return;
	}
	else 
	{
		/* Get page table pointer of this file, and open the file. */
		short int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;
		File delFile(filename, "read", false);

		/* If the file is not empty, then start deletion process. */
		if (pageTable != NULL) 
		{
			/* Get page number of current page table and send into the do...while which
			   Gets a pointer to the given page table, iterates over each page in the 
			   Page table according to its page count, and pushes them into temp stack,
			   Pushes the page tables' page into temp stack, and does this until no
			   More page tables are left. */
			int nextPageTableNum = getPageNum((char *) pageTable);
			do 
			{
				pageTable = (short int*) delFile.getPagePtr(nextPageTableNum);
				delFile.setPageTablePtr(pageTable);
				short int pageNums = delFile.getPageCount();

				for (int i = 2; i <= pageNums + 1; i++)
					freeList.push(*(pageTable + i));

				nextPageTableNum = delFile.getNextPageTableNum();

			} while (nextPageTableNum != -1);

			freeList.push(getPageNum((char *) current->files[getFileNo(filename)]->
					 pgTblPtr));
		}

		current->files.erase(current->files.begin() + getFileNo(filename));
		removeDat(pathFromRoot(current)+"/"+filename,true);	
	}
}



/* Takes in two paths for source and destination file, and moves the source file to
   The Destination file. */
void 
move (string srcPath, string destPath) 
{
	vector<string> tokenSrcFile = tokenize(srcPath, '/');
	vector<string> tokenDestFile = tokenize(destPath, '/');

	int srcPos, destPos;
	FileNode* srcFile, * destFile;
	Folder* srcFolder, * destFolder;

	/* Checks if source file exists, in which case it records its variables. */
	locateFile(tokenSrcFile, false);
	bool srcFileFound = found;
	if (srcFileFound) 
	{
		srcFile = tempFile;
		srcFolder = tempFolder;
		srcPos = filePosDir;
	}
	else
		return;

	/* Checks if destination file exists, in which case it records its variables. */
	locateFile(tokenDestFile, true);
	bool destFileFound = found;
	if (destFileFound) 
	{
		destFile = tempFile;
		destFolder = tempFolder;
		destPos = filePosDir;
	}
	else
		return;

	/* Checks if both specified files are not the same, because for a cut paste within
	   The same file, the move within function should be invoked. */
	if (srcFile != destFile) 
	{
		if (destFile != NULL) 
		{
			Folder *temp = current;
			current = destFolder;
			deleteFile(tokenDestFile.back());
			current = temp;
		}

		moveDat(pathFromRoot(srcFolder) +"/"+ srcFile->name,pathFromRoot(destFolder) +"/"+ tokenDestFile.back());
		srcFile->name = tokenDestFile.back();
		destFolder->files.push_back(srcFile);
		srcFolder->files.erase(srcFolder->files.begin() + srcPos);
	}
	else 
	{
		cout << "Invalid Arguments. To move within a file, plese open the file first."
			 << endl;
		return;
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



void
removeChildren(Folder* dir){
	tempFolder = dir;
	if(dir->subdir.size() == 0 && dir->files.size() == 0)
		return;
	
	for(int i = 0; i<dir->files.size();i++)
		deleteFile(dir->files[i]->name);

	for(int i = 0; i<dir->subdir.size();i++){
		current = dir->subdir[i];
		removeDat(pathFromRoot(current),false);
		removeChildren(current);
	}
	removeDat(pathFromRoot(dir),false);
	dir->subdir.clear();

}


/* Recursive function to delete all files and folders in th giver folder */
void 
deleteFolder(string folderName){

	tempFolder = current;
	if(!folderExists){
		cout<<"The folder does not exist in current directory"<<endl;
		return;
	}
	removeDat(pathFromRoot(current)+"/"+folderName,false);
	Folder * temp = current;
	for(int i = 0; i<current->subdir.size();i++){
		if (current->subdir[i]->dirName == folderName){
			current = current->subdir[i];
			current->parent->subdir.erase(current->parent->subdir.begin()+i);
			break;
		}
	}
	removeChildren(current);
	current = temp;
	tempFolder = current;

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



/* Reads the given .dat file, reloads the entire memory structure and writes into
   Files as given in the file. */
void 
readDat () 
{

	vector<string> lineTokens, fileFolder, tokenizedFileName, folderPath;
	string line, fileName, content,path;

	datIn.open(DATPATH);

	while (!freeList.empty())
		freeList.pop();

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);


	while (getline(datIn, line)) 
	{
		current = rootFolder;

		if (line[0] == 'D') 
		{
			path = line.substr(6,line.size()-6);
			createFolder(path,false);
			line = "";

		}
		else if (line[0] == 'F') 
		{
			line = line.substr(6, line.size() - 6);
			fileFolder = tokenize(line, '/');

			fileName = fileFolder.back();

			/* Remove .txt from file name */
			fileName = fileName.substr(0, fileName.size() - 4);

			/* Remove root and filename from path to get file directory */
			fileFolder.pop_back();
			line = "";

			/* Serializing tokens of path to string */
			if (fileFolder.size() != 0) 
			{
				for (int i = 0; i < fileFolder.size(); i++) 
					line += fileFolder[i] + "/";
				
				changeDir(line);
			}

			tempFolder = current;

			/* Create file in durectory */
			create(fileName,false);
			line = "";

		} 
		/* Condition if data is to be written in file */
		else if (line[0] == '\a') 
		{
			/* Concatenate content until -1 is encountered again */
			while (getline(datIn, line)) 
			{
				if (line[0] == '\a')
					break;
				content += line + "\n";
			}

			content = content.substr(0, content.size() - 1);
			current = tempFolder;

			/* Open file and write content */
			File openedFile(fileName + ".txt", "write", false);
			openedFile.write(content,false);
			content = "";
		}
	}
	current = rootFolder;
	tempFolder = current;
	datIn.close();
}




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




/* Prompts user to enter in their command, and once it is taken, it is tokenized based
   On spaces and the vector of resulting strings is returned. */
vector<string> 
getCommand (ifstream& input) 
{
	string command;
	cout << pathFromRoot(current) << "> ";
	getline(cin, command);
	// getline(input,command);
	return tokenize(command, ' ');
}



/* ProcessCommand takes in commands vector as arugment, forwards the commands to their
   Respective functions, and returns boolean to main to update running status of 
   Program. */
bool 
processCommand (vector<string> tokens,ifstream& input) 
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
			File openedFile(tokens[1], tokens[2], true);

			if (!fileFound)
				return loop;

			bool inLoop = true;

			while (inLoop) 
			{
				vector<string> tokens = getCommand(input);

				if (tokens.size() == 1 && tokens[0] == "wr"){
					openedFile.write(openedFile.getInput(input),true);
				}

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



int 
main (int argc, const char* argv[]) 
{
	current = rootFolder;
	tempFolder = rootFolder;
	start = (char*)malloc(MEMSIZE);

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);

	bool loop = true;
	printSpace();

	ifstream threadIn;
	threadIn.open("thread inputs/1.txt");
	while (loop) 
	{
		vector<string> tokens = getCommand(threadIn);
		loop = processCommand(tokens,threadIn);
	}

	free((char*)start);
	return 0;
}