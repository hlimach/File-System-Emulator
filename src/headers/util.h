#ifndef UTIL_H
#define UTIL_H

#include "config.h"
#include "mem_struct.h"

void help ();
vector<string> tokenize (string, char);
void printSpace(int);
int getPageNum (char*);
string pathFromRoot (Folder*);
int getFileNo (string, int);
bool fileExists (string, int);
bool folderExists (string, int);
void listDir (int);
bool isNumber (string);
void locateFile (vector<string>, bool, int);
void listFiles (Folder*);
void memMap (Folder*, int);
vector<string> getCommand (ifstream&, int);
bool processCommand (vector<string> tokens, ifstream& input, int);

#endif