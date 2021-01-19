#ifndef MEM_STRUCT_H
#define MEM_STRUCT_H

/* Class for file nodes in the tree structure to be maintained for memory management. 
   It is a leaf node. */
class 
FileNode 
{
public:
	string name;
	short int* pgTblPtr;
	int numReaders;
	vector<vector <int>> fileQue;
	//sem_t *writer_sema;
	sem_t writer_sema;
	mutex queMtx;
	bool active;

	/* Pointer pgTblPtr points to this files' page table, but is assigned NULL upon 
	   Creation, and will only point to one once data is written into this file. */
	FileNode (string fileName);
	FileNode ();
};


/* Class for folder nodes in the tree structure to be maintained for memory management.
   It maintains vectors for both subfolders and files within this folder. Stores a 
   Pointer to its parent as folder nodes are used in tree traversal. */
class 
Folder 
{
public:
	string dirName;
	vector<Folder*> subdir;
	vector<FileNode*> files;
	Folder* parent;
	int NumUsers;

	Folder (string name);
};


bool traverseTree (int, vector<string>, bool, int, bool);
void createFolder (string, bool, int);
void changeDir (string, int, bool);
void create (string, bool, int);
void deleteFile (string, int);
void move (string, string, int);
void removeChildren (Folder*, int);
void deleteFolder(string, int);
void toRoot(int);

#endif