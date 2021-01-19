#include "../include/config.h"
#include "../include/file.h"
#include "../include/globals.h"
#include "../include/util.h"
#include "../include/dat.h"
#include "../include/sema.h"

File :: File ()
{}

File :: File (string name, string md, bool printInf, int threadNo) 
: threadNum(threadNo), filename(name), mode(md), page(NULL), printInfo(printInf),
	pageTable(current[threadNo]->files[getFileNo(name, threadNo)]->pgTblPtr)
{
	fileSize = getFileSize();
	printFileInfo();
}


/* Prints file information by checking if the file is opened silently or not. */
void File ::
printFileInfo()
{
	if (printInfo)
		serverResponse[threadNum] += "File opened: " + filename + ", mode: " + 
			mode + ", size: " + to_string(fileSize) + "\n";
}


/* For any given page number within memory, its pointer is returned. */
char* File :: 
getPagePtr (short int x) 
{
	return (char*)start + (PAGESIZE * x);
}


/* The first entry in any page table is its' limit i.e where the last pages' last 
   Byte is (where it is filled upto). */
short int File ::
getByteLimit () 
{
	return *(pageTable);
}


/* The second entry in any page table is the number of pages it has i.e how many
   Pages are kept in the page table to assist iteration. */
short int File ::
getPageCount () 
{
	return *(pageTable + 1);
}


/* The last entry in a page table is the page number of next linked page table. */
short int File ::
getNextPageTableNum () 
{
	return *(pageTable + LASTENTRY);
}


/* Sets/ Updates the current page tables' limit to be the given value. */
void File ::
setByteLimit (int limit) 
{
	*(pageTable) = limit;
}


/* Sets/ Updates the current page tables' page count to be the given value. */
void File ::
setPageCount (int count) 
{
	*(pageTable + 1) = count;
}


/* Sets/ Updates the current page tables' last entry to be the given value
   Which indicates which page number of page table it is next linked to. */
void File ::
setNextPageTableNum (int nextPageNum) 
{
	*(pageTable + LASTENTRY) = nextPageNum;
}	


/* Sets/ Updates the page tables' pointer to point to a new location in memory. */
void File ::
setPageTablePtr (short int * pageTbl) 
{
	pageTable = pageTbl;
}


/* Resets page table pointer to point to the starting page table. */
void File ::
resetPageTblPtr () 
{
	pageTable = current[threadNum]->files[getFileNo(filename,threadNum)]->pgTblPtr;
}


/* Iterates all the page tables of the given file and returns the total size in 
   Bytes of the current file. */
int File ::
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
void File ::
changeMode (string md) 
{
	if (mode != md)
	{
		if ((md == "read" || md == "write")) 
		{
			leaveFile(tempFile[threadNum], mode);
			enterFile(tempFile[threadNum], threadNum, md);
			mode = md;
			printFileInfo();
		}
		else
			serverResponse[threadNum] += "Please enter a valid mode (read|write).\n";
	}
	else
		serverResponse[threadNum] += "File already opened in " + md + " mode.\n";
}


/* For the current page table, this function takes in argument to firstly specify 
   A read or write operation. If it is a write opeartion, the input string is 
   Written into the specified location, otherwise the specified locations' data is
   Simply returned to calling function. */
string File ::
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
string File ::
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
string File ::
read (int startFrom, int readUpTo) 
{
	string text = "";
	resetPageTblPtr();
    if (mode != "read") 
    {
        serverResponse[threadNum] += "Please open file in \"read\" mode for this function.\n";
        return "";
    }
    else if (pageTable == NULL) 
    {
        serverResponse[threadNum] += "The file has no content to display.\n";
        return "";
    }
    else if ((readUpTo - startFrom > fileSize) || (startFrom + readUpTo > fileSize) || (startFrom < 0)) 
	{
		serverResponse[threadNum] += "Out of bounds. Given limit exceeds total file limit at "
			 + to_string(fileSize) + " bytes.\n";
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


/* Updates the information of this file written in the .dat file. */
void File ::
updateDat (string path)
{
	ofstream overWriteDat;
	openStream();
	string line = "", topText = "", data = "", endText = "";
	
	//read top text untill file is found
	while (getline(datStream, line))
	{	
		cout << "line: " << line << endl;
		if (line[0] == 'F')
			if (line.substr(2, line.size() - 2) == path)
				break;
		
		topText += line + "\n";
	}

	topText += line + "\n";

	cout << "topText" << topText << endl;

	if (fileSize != 0)
		data = "\a\n" + read(0, fileSize) + "\n\a\n";

	if (datStream.eof())
	{		
		overWriteDat.open(DATPATH);
		overWriteDat << topText + data;
		overWriteDat.close();
		closeStream();
		return;
	}

	//skip the data stored in that file if any
	getline(datStream, line);
	if (line[0] == '\a')
	{
		do
		{
			getline(datStream, line);
		} while(line[0] != '\a');

		getline(datStream,line);
	}
	
	//read after the file is found and data is skipped
	endText += line + "\n";

	while (getline(datStream, line))
		endText += line + "\n";

	overWriteDat.open(DATPATH);
	overWriteDat << topText + data + endText.substr(0, endText.length() - 1);
	overWriteDat.close();
	closeStream();
}


/* Makes necessary changes and calls updateDat function */
void File ::
callUpdateDat ()
{
	bool prevPrintInfo = printInfo;
	printInfo = false;
	mode = "read";
	updateDat(pathFromRoot(current[threadNum]) + "/" + filename);
	mode = "write";
	printInfo = prevPrintInfo; 
}


/* Prompts user to input data for the write functions. It stops taking input once 
   The user enters '-1' in a new line and presses enter. */
string File ::
getInput (int threadNo) 
{
	if (mode != "write") 
	{
		serverResponse[threadNum] += "Please open file in \"write\" mode for this function.\n";
		return "";
	} 
	else 
	{
		// File writing permission characters sent to client
		serverResponse[threadNum] = "%^#$";
		char* op = convertMessage(serverResponse[threadNum], serverResponse[threadNum].size());
		::send(sockets[threadNo], op, strlen(op), 0);
		serverResponse[threadNum] = "";
		
		// Buffer created for incoming text receival
		string input = "";
		char buffer[BUFFER] = {0};

		// If response is larger than buffer, it is received in chunks (packets)
	    do 
		{
			bzero(buffer, BUFFER);
	        if (::read(sockets[threadNo], buffer, BUFFER) == 0)
			{
				cout << "user " + users[threadNo] + " crashed" << endl;
	        	input = "";
				serverResponse[threadNum] = "";
			}
			else
	        {
	        	string check = buffer;
	        	if (check.size() == BUFFER + 1)
	        		check = check.substr(0, BUFFER);
	        	input += check;
	        }

        } while (input.substr(input.size() - 2, 2) != "#$");

		return input.substr(0, input.size() - 2);
	}
}


/* Function assigns pages to a file as it is being filled i.e. according to needed
   Pages within a page table. It pops a value from free list and assigns it to the
   Page table entry. */
void File ::
assignPages (int start, int last) 
{
	for (int i = start + 1; i <= last + 1; i++) 
		*(pageTable + i) = popStack();
}


/* Function calculates the limit and needed pages according to the input size. */
void File ::
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
void File ::
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
                setNextPageTableNum(popStack());
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
void File ::
write (string input, bool updatedat) 
{
	int neededPages = 0, byteCount = 0;
    short int limit;

	if (mode != "write") 
		return;
	
	else if (input.size() > (freeList.size() * PAGESIZE)) 
    {
        serverResponse[threadNum] += "Not enough memory available. ";
		serverResponse[threadNum] += "Please reduce input size or delete other files.\n";
        return;
    }

	/* If it is a new file i.e. empty, then this control branch is taken. */
	else if (pageTable == NULL) 
	{
		/* Assigns this file a new page table. */
        short int pageTablePageNum = popStack();
        char* page = getPagePtr(pageTablePageNum);
        current[threadNum]->files[getFileNo(filename,threadNum)]->pgTblPtr = (short int*) page;
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
            setNextPageTableNum(popStack());
            int pageTablePageNum = getNextPageTableNum();
            createPageTableAndWriteData(input, neededPages, limit, byteCount, 
            	 pageTablePageNum);
        }
    }

	fileSize = getFileSize();
	if(updatedat)
		callUpdateDat();
	
	if (printInfo)
		serverResponse[threadNum] += "Updated file size: " + to_string(fileSize) + "\n";
}


/* WriteAt function is invoked whenever user wishes to write into file at a
   Specific (over writing). */
void File ::
writeAt (string data, int writeAt) 
{
	if (mode != "write") 
		return;

	else  if (pageTable == NULL) 
	{
		serverResponse[threadNum] += "Invalid command. Cannot call 'Write At' on an empty file.\n";
		return;
	}

	else if (writeAt > fileSize || writeAt < 0) 
	{
		serverResponse[threadNum] += "Out of bounds. Given byte is greater than file size of " 
			+ to_string(fileSize) + " bytes.\n";
		return;
	}

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
void File ::
truncate (int size) 
{
	if (mode != "write") 
	{
		serverResponse[threadNum] += "Please open file in \"write\" mode for this function\n";
		return;
	}
	else if (size > fileSize) 
	{
		serverResponse[threadNum] +=  "Out of bounds. Given byte is greater than file size of "
			 + to_string(fileSize) + " bytes.\n";
		return;
	}
	else if (size == 0)
	{
		serverResponse[threadNum] += "Close file to delete it.\n"; 
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
            pushStack(*(pageTable + i));

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
                pushStack(*(pageTable + i));

            pushStack(nextPageTableNum);
            nextPageTableNum = getNextPageTableNum();
        }

        fileSize = getFileSize();

        if (printInfo)
            serverResponse[threadNum] += "File size reduced to " + to_string(fileSize) + " bytes.\n";
    }
	callUpdateDat();
}


/* Move within is invoked for a cut paste action where user specifies where to cut
   From, for how many bytes (size of text chunk that must be cut), and where it
   Should be pasted. */
void File ::
moveWithin (int from, int size, int to) 
{
	fileSize = getFileSize();

	if ((to > from && to < from + size) || to < 0 || from < 0 || size < 0) 
	{
		serverResponse[threadNum] += "Invalid byte arguments.\n";
		return;
	}
	else if (mode != "write") 
	{
		serverResponse[threadNum] += "Please open file in \"write\" mode.\n";
		return;
	}
	else if (pageTable == NULL) 
	{
		serverResponse[threadNum] += "Invalid command. Cannot call 'Write At' on an empty file.\n";
		return;
	}
	else if (from + size > fileSize) 
	{
		serverResponse[threadNum] += "Out of bounds. Specified chunk exceeds out of file size of "
			  + to_string(fileSize) + " bytes.\n";
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