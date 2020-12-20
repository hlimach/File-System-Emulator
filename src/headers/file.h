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
	int fileSize, threadNum;
	bool printInfo;

public:
	File (string name, string md, bool printInf, int threadNo);

	void printFileInfo ();
	char* getPagePtr (short int);
	short int getByteLimit ();
	short int getPageCount ();
	short int getNextPageTableNum ();
	void setByteLimit (int);
	void setPageCount (int);
	void setNextPageTableNum (int);	
	void setPageTablePtr (short int*);
	void resetPageTblPtr ();
	int getFileSize ();
	void changeMode (string);
	string pageTableData (string, int, int, int, int, int&, bool);
    string chunkManipulation (int, int, bool, string);
	string read (int, int);
	void updateDat (string);
	void callUpdateDat ();
	string getInput (ifstream&);
	void assignPages (int, int);
	void calcLimit (int&, short int&, int, int);
	void createPageTableAndWriteData (string, int, int, int&, int);
	void write (string, bool);
	void writeAt (string, int);
	void truncate (int);
	void moveWithin (int, int, int);

};


#endif