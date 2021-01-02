#include "headers/config.h"
#include "headers/mem_struct.h"
#include "headers/globals.h"
#include "headers/util.h"
#include "headers/dat.h"
#include "headers/file.h"

FileNode :: FileNode(string fileName)
: name(fileName), pgTblPtr(NULL)
{}


Folder :: Folder(string name)
: dirName(name), parent(NULL)
{}


/* Traverses tree according to given path. If a folder within the path is not found, 
   The loop is exited, and the boolean found is set to false. */
bool
traverseTree (int i, vector<string> tokens, bool change, int threadNo)
{
	int lim = tokens.size() - 1;
	if (change)
		lim++;

	for (i; i < lim; i++)
    {
        if (tokens[i] == "..")
        {
            if (tempFolder[threadNo]->parent == NULL)
            {
                serverResponse += "Parent of root does not exist.\n";
                return false;
            }
            tempFolder[threadNo] = tempFolder[threadNo]->parent;
        }
        else
        {
            bool folderFound = folderExists(tokens[i],threadNo);
            if (!folderFound)
            {
                return false;
            }
        }
    }
    return true;
}


/* Fucntion createFolder accepts argument path for folder creation traverses the tree
   From current path, and if the path exists it creates a node for subdirectory. */
void 
createFolder (string path,bool updatedat, int threadNo) 
{
	bool createable = true;
	tempFolder[threadNo] = current[threadNo];
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		serverResponse += "Invalid path entered.\n";
		return;
	}
	else {
		createable = traverseTree(0, tokens, false,threadNo);

		/* If the path exists, a folder of the same name does not already exist, and
		   The name of the folder is neither '.' nor '..', then a folder at the 
		   Specified path is successfully created. */
		if (createable && !folderExists(tokens.back(),threadNo) && (tokens.back() != "." ||
			 tokens.back() != "..")) 
		{
			tempFolder[threadNo]->subdir.push_back(new Folder(tokens.back()));
			tempFolder[threadNo]->subdir.back()->parent = tempFolder[threadNo];
		}
		else
			serverResponse += "Error: cannot create directory in specified path.\n";
	}
	tempFolder[threadNo] = current[threadNo];

	if (updatedat)
		enterDat(pathFromRoot(current[threadNo]), false, path);

}


/* Function changeDir accepts argument path traverses the tree related to current 
   Directory and if path exists updates the current working directory to specified 
   Path. */
void 
changeDir (string path, int threadNo) 
{
	bool changable = true;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		serverResponse += "Invalid path entered.\n";
		return;
	}
	else 
	{
		tempFolder[threadNo] = current[threadNo];
		int i = 0;

		/* If the first token is '.', then it works in current directory. */
		if (tokens[0] == ".")
			i = 0;

		changable = traverseTree(i, tokens, true,threadNo);

		if (changable)
			current[threadNo] = tempFolder[threadNo];
		else
			serverResponse += "Error: cannot change directory to specified path.\n";
	}
}


/* Creates a file node at the current working directory and pushes it into its' files
   List. This does not assign any pages. */
void 
create (string filename,bool updatedat, int threadNo) 
{
	filename += ".txt";

	if (!fileExists(filename,threadNo)) 
	{
		current[threadNo]->files.push_back(new FileNode(filename));
		
		if (updatedat)
			enterDat(pathFromRoot(current[threadNo]), true, filename);
	}
	else
		serverResponse += "A file of same name already exists.\n";
}


/* Takes in file name as an argument. Works on current working directory. If a file
   Of the same name exists, it is deleted. */
void 
deleteFile (string filename, int threadNo) 
{
	/* If a file of the given name does not exist in current working directory. */
	tempFolder[threadNo] = current[threadNo];
	if (!fileExists(filename,threadNo)) 
	{
		serverResponse += "Error: file does not exist\n";
		return;
	}
	else 
	{
		/* Get page table pointer of this file, and open the file. */
		short int* pageTable = current[threadNo]->files[getFileNo(filename,threadNo)]->pgTblPtr;
		File delFile(filename, "read", false,threadNo);

		/* If the file is not empty, then start deletion process. */
		if (pageTable != NULL) 
		{
			/* Get page number of current page table and send into the do...while which
			   Gets a pointer to the given page table, iterates over each page in the 
			   Page table according to its page count, and pushes them into temp stack,
			   Pushes the page tables' page into temp stack, and does this until no
			   More page tables are left. */
			int nextPageTableNum = getPageNum((char *) pageTable);
			do 
			{
				pageTable = (short int*) delFile.getPagePtr(nextPageTableNum);
				delFile.setPageTablePtr(pageTable);
				short int pageNums = delFile.getPageCount();

				for (int i = 2; i <= pageNums + 1; i++)
					freeList.push(*(pageTable + i));

				nextPageTableNum = delFile.getNextPageTableNum();

			} while (nextPageTableNum != -1);

			freeList.push(getPageNum((char *) current[threadNo]->files[getFileNo(filename,threadNo)]->
					 pgTblPtr));
		}

		current[threadNo]->files.erase(current[threadNo]->files.begin() + getFileNo(filename,threadNo));
		removeDat(pathFromRoot(current[threadNo]) + "/" + filename,true);	
	}
}


/* Takes in two paths for source and destination file, and moves the source file to
   The Destination file. */
void 
move (string srcPath, string destPath, int threadNo) 
{
	vector<string> tokenSrcFile = tokenize(srcPath, '/');
	vector<string> tokenDestFile = tokenize(destPath, '/');

	int srcPos, destPos;
	FileNode* srcFile, * destFile;
	Folder* srcFolder, * destFolder;

	/* Checks if source file exists, in which case it records its variables. */
	locateFile(tokenSrcFile, false,threadNo);
	bool srcFileFound = found[threadNo];
	if (srcFileFound) 
	{
		srcFile = tempFile[threadNo];
		srcFolder = tempFolder[threadNo];
		srcPos = filePosDir[threadNo];
	}
	else
		return;

	/* Checks if destination file exists, in which case it records its variables. */
	locateFile(tokenDestFile, true,threadNo);
	bool destFileFound = found[threadNo];
	if (destFileFound) 
	{
		destFile = tempFile[threadNo];
		destFolder = tempFolder[threadNo];
		destPos = filePosDir[threadNo];
	}
	else
		return;

	/* Checks if both specified files are not the same, because for a cut paste within
	   The same file, the move within function should be invoked. */
	if (srcFile != destFile) 
	{
		if (destFile != NULL) 
		{
			Folder *temp = current[threadNo];
			current[threadNo] = destFolder;
			deleteFile(tokenDestFile.back(),threadNo);
			current[threadNo] = temp;
		}

		moveDat(pathFromRoot(srcFolder) + "/" + srcFile->name,pathFromRoot(destFolder) 
			+ "/" + tokenDestFile.back());

		srcFile->name = tokenDestFile.back();
		destFolder->files.push_back(srcFile);
		srcFolder->files.erase(srcFolder->files.begin() + srcPos);
	}
	else 
	{
		serverResponse += "To move within a file, please open the file first.\n";
		return;
	}
}


void
removeChildren (Folder* dir, int threadNo)
{
	tempFolder[threadNo] = dir;

	if(dir->subdir.size() == 0 && dir->files.size() == 0)
		return;
	
	for (int i = 0; i < dir->files.size(); i++)
		deleteFile(dir->files[i]->name,threadNo);

	for (int i = 0; i < dir->subdir.size(); i++)
	{
		current[threadNo] = dir->subdir[i];
		removeDat(pathFromRoot(current[threadNo]), false);
		removeChildren(current[threadNo], threadNo);
	}

	removeDat(pathFromRoot(dir), false);
	dir->subdir.clear();

}


/* Recursive function to delete all files and folders in the given folder */
void 
deleteFolder(string folderName, int threadNo)
{
	tempFolder[threadNo] = current[threadNo];

	if (!folderExists)
	{
		serverResponse += "The folder does not exist in current directory\n";
		return;
	}

	removeDat(pathFromRoot(current[threadNo]) + "/" + folderName,false);
	Folder * temp = current[threadNo];

	for (int i = 0; i < current[threadNo]->subdir.size();i++)
	{
		if (current[threadNo]->subdir[i]->dirName == folderName)
		{
			current[threadNo] = current[threadNo]->subdir[i];
			current[threadNo]->parent->subdir.erase(current[threadNo]->parent->subdir.begin()+i);
			break;
		}
	}

	removeChildren(current[threadNo],threadNo);
	current[threadNo] = temp;
	tempFolder[threadNo] = current[threadNo];

}