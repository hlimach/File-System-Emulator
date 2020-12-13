#ifndef GLOBALS_H
#define GLOBALS_H

#include "memory_structure.h"

extern const char* start;

extern stack <short int> freeList;

extern Folder* rootFolder;

extern ofstream dat;

extern ifstream datIn;

extern string out;

extern Folder* current, * tempFolder;

extern FileNode* tempFile;

extern int filePosDir;

extern bool fileFound, found;

extern thread th[10];

extern ifstream threadIn[10];

#endif