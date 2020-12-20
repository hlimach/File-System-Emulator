#ifndef GLOBALS_H
#define GLOBALS_H

#include "mem_struct.h"

extern const char* start;
extern stack <short int> freeList;
extern Folder* rootFolder;
extern fstream datStream;
extern Folder* current[10], *tempFolder[10];
extern FileNode* tempFile[10];
extern int filePosDir[10];
extern bool fileFound[10], found[10];
extern thread th[10];
extern ifstream threadIn[10];
extern ofstream threadOut[10];


#endif