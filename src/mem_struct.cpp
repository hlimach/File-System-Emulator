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
traverseTree (int i, vector<string> tokens, bool change)
{
	int lim = tokens.size() - 1;
	if (change)
		lim++;

	for (i; i < lim; i++)
    {
        if (tokens[i] == "..")
        {
            if (tempFolder->parent == NULL)
            {
                cout << "Parent of root does not exist." << endl;
                return false;
            }
            tempFolder = tempFolder->parent;
        }
        else
        {
            bool folderFound = folderExists(tokens[i]);
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
createFolder (string path,bool updatedat) 
{
	bool createable = true;
	tempFolder = current;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		cout << "Invalid path entered." << endl;
		return;
	}
	else {
		createable = traverseTree(0, tokens, false);

		/* If the path exists, a folder of the same name does not already exist, and
		   The name of the folder is neither '.' nor '..', then a folder at the 
		   Specified path is successfully created. */
		if (createable && !folderExists(tokens.back()) && (tokens.back() != "." ||
			 tokens.back() != "..")) 
		{
			tempFolder->subdir.push_back(new Folder(tokens.back()));
			tempFolder->subdir.back()->parent = tempFolder;
		}
		else
			cout << "Error: cannot create directory in specified path." << endl;
	}
	tempFolder = current;

	if (updatedat)
		enterDat(pathFromRoot(current), false, path);

}


/* Function changeDir accepts argument path traverses the tree related to current 
   Directory and if path exists updates the current working directory to specified 
   Path. */
void 
changeDir (string path) 
{
	bool changable = true;
	vector<string> tokens = tokenize(path, '/');

	if (tokens.size() == 0) 
	{
		cout << "Invalid path entered." << endl;
		return;
	}
	else 
	{
		tempFolder = current;
		int i = 0;

		/* If the first token is '.', then it works in current directory. */
		if (tokens[0] == ".")
			i = 0;

		changable = traverseTree(i, tokens, true);

		if (changable)
			current = tempFolder;
		else
			cout << "Error: cannot change directory to specified path." << endl;
	}
}


/* Creates a file node at the current working directory and pushes it into its' files
   List. This does not assign any pages. */
void 
create (string filename,bool updatedat) 
{
	filename += ".txt";

	if (!fileExists(filename)) 
	{
		current->files.push_back(new FileNode(filename));
		
		if (updatedat)
			enterDat(pathFromRoot(current), true, filename);
	}
	else
		cout << "A file of same name already exists." << endl;
}


/* Takes in file name as an argument. Works on current working directory. If a file
   Of the same name exists, it is deleted. */
void 
deleteFile (string filename) 
{
	/* If a file of the given name does not exist in current working directory. */
	if (!fileExists(filename)) 
	{
		cout << "Error: file does not exist" << endl;
		return;
	}
	else 
	{
		/* Get page table pointer of this file, and open the file. */
		short int* pageTable = current->files[getFileNo(filename)]->pgTblPtr;
		File delFile(filename, "read", false);

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

			freeList.push(getPageNum((char *) current->files[getFileNo(filename)]->
					 pgTblPtr));
		}

		current->files.erase(current->files.begin() + getFileNo(filename));
		removeDat(pathFromRoot(current) + "/" + filename,true);	
	}
}



/* Takes in two paths for source and destination file, and moves the source file to
   The Destination file. */
void 
move (string srcPath, string destPath) 
{
	vector<string> tokenSrcFile = tokenize(srcPath, '/');
	vector<string> tokenDestFile = tokenize(destPath, '/');

	int srcPos, destPos;
	FileNode* srcFile, * destFile;
	Folder* srcFolder, * destFolder;

	/* Checks if source file exists, in which case it records its variables. */
	locateFile(tokenSrcFile, false);
	bool srcFileFound = found;
	if (srcFileFound) 
	{
		srcFile = tempFile;
		srcFolder = tempFolder;
		srcPos = filePosDir;
	}
	else
		return;

	/* Checks if destination file exists, in which case it records its variables. */
	locateFile(tokenDestFile, true);
	bool destFileFound = found;
	if (destFileFound) 
	{
		destFile = tempFile;
		destFolder = tempFolder;
		destPos = filePosDir;
	}
	else
		return;

	/* Checks if both specified files are not the same, because for a cut paste within
	   The same file, the move within function should be invoked. */
	if (srcFile != destFile) 
	{
		if (destFile != NULL) 
		{
			Folder *temp = current;
			current = destFolder;
			deleteFile(tokenDestFile.back());
			current = temp;
		}

		moveDat(pathFromRoot(srcFolder) +"/"+ srcFile->name,pathFromRoot(destFolder) +"/"+ tokenDestFile.back());
		srcFile->name = tokenDestFile.back();
		destFolder->files.push_back(srcFile);
		srcFolder->files.erase(srcFolder->files.begin() + srcPos);
	}
	else 
	{
		cout << "Invalid Arguments. To move within a file, plese open the file first."
			 << endl;
		return;
	}
}


void
removeChildren (Folder* dir)
{
	tempFolder = dir;

	if(dir->subdir.size() == 0 && dir->files.size() == 0)
		return;
	
	for (int i = 0; i < dir->files.size(); i++)
		deleteFile(dir->files[i]->name);

	for (int i = 0; i < dir->subdir.size(); i++)
	{
		current = dir->subdir[i];
		removeDat(pathFromRoot(current), false);
		removeChildren(current);
	}

	removeDat(pathFromRoot(dir), false);
	dir->subdir.clear();

}


/* Recursive function to delete all files and folders in the given folder */
void 
deleteFolder(string folderName)
{
	tempFolder = current;

	if (!folderExists)
	{
		cout << "The folder does not exist in current directory" << endl;
		return;
	}

	removeDat(pathFromRoot(current) + "/" + folderName,false);
	Folder * temp = current;

	for (int i = 0; i < current->subdir.size();i++)
	{
		if (current->subdir[i]->dirName == folderName)
		{
			current = current->subdir[i];
			current->parent->subdir.erase(current->parent->subdir.begin()+i);
			break;
		}
	}

	removeChildren(current);
	current = temp;
	tempFolder = current;

}