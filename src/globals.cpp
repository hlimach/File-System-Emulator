#include "headers/config.h"
#include "headers/globals.h"

/* Points to the starting location of the allocated memory. */
const char* start;

/* A stack of free/ unused page numbers in ascending order maintained by the system. 
   Whenever a new page is to be assigned, this stack is popped. */
stack <short int> freeList;

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

/* 10 threads for processing */
thread th[10];

/* 10  input streams for 10 threads of input File*/
ifstream threadIn[10];