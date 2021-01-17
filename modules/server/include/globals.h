#ifndef GLOBALS_H
#define GLOBALS_H

#include "mem_struct.h"
#include "file.h"

extern mutex datMtx;
extern mutex stackMtx;
extern const char* start;
extern stack <short int> freeList;
extern Folder* rootFolder;
extern fstream datStream;
extern vector <int> sockets;
extern vector <thread> threads;
extern vector <Folder*> current;
extern vector <string> users;
extern vector <Folder*> tempFolder;
extern vector <FileNode*> tempFile;
extern File openedFiles;
extern vector <int> filePosDir;
extern vector <bool> fileFound, found;
extern string serverResponse;


#endif