#ifndef UTIL_H
#define UTIL_H

#include "config.h"
#include "mem_struct.h"

char* convertMessage (string, int);
void help ();
vector<string> tokenize (string, char);
void printSpace(int);
int getPageNum (char*);
string pathFromRoot (Folder*);
int getFileNo (string, int);
void pushStack(short int);
short int popStack();
bool fileExists (string, int);
bool folderExists (string, int,bool);
void listDir (int);
bool isNumber (string);
void locateFile (vector<string>, bool, int);
void listFiles (Folder*);
void memMap (Folder*, int);
void sendResponse (int);
vector<string> getCommand (int);
void invalidCmdMsg();
int getCmdIndex (vector<string>, string);
void fileCmds1Call(int index, vector<string>, int, bool&);
void fileCmds2Call(int index, vector<string>, int);
void fileCmdProcessing(vector<string>, int);
void cmds1Call (int index, vector<string>, int, bool&);
void cmds2Call(int index, vector<string>, int);
void cmds3Call(int index, vector<string>, int);
bool processCommand (vector<string>, int);

#endif