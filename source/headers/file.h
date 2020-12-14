#ifndef FILE_H
#define FILE_H

#include "config.h"

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
	File (string name, string md, bool printInf);

	void printFileInfo ();
	char* getPagePtr (short int x);
	short int getByteLimit ();
	short int getPageCount ();
	short int getNextPageTableNum ();
	void setByteLimit (int limit);
	void setPageCount (int count);
	void setNextPageTableNum (int nextPageNum);	
	void setPageTablePtr (short int * pageTbl);
	void resetPageTblPtr ();
	int getFileSize ();
	void changeMode (string md);
	string pageTableData (string input, int startPage, int lastPage, int startByte,
		 int limit, int &byteCount, bool read);
    string chunkManipulation (int startFrom, int chunkSize, bool read, string data);
	string read (int startFrom, int readUpTo);
	void updateDat(string path);
	void callUpdateDat ();
	string getInput (ifstream& in);
	void assignPages (int start, int last);
	void calcLimit (int &neededPages, short int &limit, int inputSize, int macro);
	void createPageTableAndWriteData (string input, int neededPages, int limit, 
		 int &byteCount, int pageTablePageNum);
	void write (string input , bool updatedat);
	void writeAt (string data, int writeAt);
	void truncate (int size);
	void moveWithin (int from, int size, int to);

};


#endif