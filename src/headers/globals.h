#ifndef GLOBALS_H
#define GLOBALS_H

#include "mem_struct.h"
#include "file.h"

extern mutex mtx;
extern const char* start;
extern stack <short int> freeList;
extern Folder* rootFolder;
extern fstream datStream;
extern vector<int> sockets;
extern vector <thread> threads;
extern vector<Folder*>current;
extern vector<Folder*>tempFolder;
extern vector<FileNode*>tempFile;
extern File* openedFiles;
extern int filePosDir[10];
extern bool fileFound[10], found[10];
extern thread th[10];
extern ifstream threadIn[10];
extern ofstream threadOut[10];
extern string serverResponse;


#endif