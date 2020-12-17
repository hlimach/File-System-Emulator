#ifndef UTIL_H
#define UTIL_H

#include "config.h"
#include "mem_struct.h"

void help ();
vector<string> tokenize (string, char);
void printSpace();
int getPageNum (char*);
string pathFromRoot (Folder*);
int getFileNo (string);
bool fileExists (string);
bool folderExists (string);
void listDir ();
bool isNumber (string);
void locateFile (vector<string>, bool);
void listFiles (Folder*);
void memMap (Folder*);
vector<string> getCommand (ifstream&,int);
bool processCommand (vector<string> tokens, ifstream& input,int);


#endif