#include "headers/config.h"
#include "headers/globals.h"

/* lockable object to prevent muultiple threads accessing shared data */
mutex mtx;

/* Points to the starting location of the allocated memory. */
const char* start;

/* A stack of free/ unused page numbers in ascending order maintained by the system. 
   Whenever a new page is to be assigned, this stack is popped. */
stack <short int> freeList;

/* The starting folder of this system : "root". */
Folder* rootFolder = new Folder("root");

/* To read and write to .dat file whenever required. */
fstream datStream;

/* Current folder pointer stores pointer to current working directory, whereas 
   tempFolder pointer is used for temporary actions/ tracking. */
Folder* current[10] = {rootFolder, rootFolder, rootFolder, rootFolder, rootFolder,
		 rootFolder, rootFolder, rootFolder, rootFolder, rootFolder}; 
Folder* tempFolder[10] = {rootFolder, rootFolder, rootFolder, rootFolder, rootFolder,
		 rootFolder, rootFolder, rootFolder, rootFolder, rootFolder}; 

/* Pointer tempFile points to File node to store its value if found. */
FileNode* tempFile[10];

/* 10 instances of open files for 10 threads */
File* openedFiles = new File[10];

/* Int filePosDir tells us which index in the vector of the current directory the 
   Needed file will be found. */
int filePosDir[10];

/* To let functions know if the needed file is found. */
bool fileFound[10], found[10];

/* 10 threads for processing */
thread th[10];

/* 10  input streams for 10 threads of input File*/
ifstream threadIn[10];

/* 10 output streams for 10 threads of output file*/
ofstream threadOut[10];