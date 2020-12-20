#include "headers/config.h"
#include "headers/globals.h"
#include "headers/dat.h"
#include "headers/util.h"
#include "headers/file.h"


/* Whenever a new file or folder is created, this function is called so it appends
   To the end of the existing .dat file, updating it. */
void
enterDat (string path, bool file, string name)
{
	string prevText = "", line = "";

	datIn.open(DATPATH);

	while(getline(datIn, line))
		prevText += line + "\n";
	
	datIn.close();

	if(file)
		prevText += "F\t" + path + "/" + name; 
	else
		prevText += "D\t" + path + "/" + name; 
	
	dat.open(DATPATH);
	dat << prevText;
	dat.close();
}


/* Whenever a file is moved, this function is called. It gets the path of the file,
   Removes its entry from the .dat file, and adds it to the end with the updated
   Path. */
void
moveDat (string oldPath, string newPath)
{
	string line = "", prevText = "", data = "";
	datIn.open(DATPATH);

	while (getline(datIn, line))
	{
		if (line[0] == 'F')
			if (line.substr(2, line.size() - 2) == oldPath)
				break;

		prevText += line + "\n";
	}
	
	getline (datIn, line);
	
	if (line[0] == '\a')
	{
		data += line + "\n";
		do
		{
			getline(datIn,line);
			data += line + "\n";
		} while(line[0]!='\a');

		getline(datIn,line);
	}

	prevText += line + "\n";

	while (getline(datIn, line))
		prevText += line + "\n";

	datIn.close();
	dat.open(DATPATH);
	dat << prevText + "F\t" + newPath + "\n" + data.substr(0, data.length() - 1);
	dat.close();
}


/* Whenever a file or folder is deleted, it removes its entry from the .dat file. */
void
removeDat (string path, bool file)
{
	string line = "", prevText = "", endText = "";
	datIn.open(DATPATH);

	while (getline(datIn, line))
	{
		if (line[0] == 'F' || line[0] == 'D')
			if (line.substr(2, line.size() - 2) == path)
				break;

		prevText += line + "\n";
	}
	
	if (datIn.eof()) 
	{
		datIn.close();
		dat.open(DATPATH);
		dat << prevText;
		dat.close();
		return;
	}

	getline(datIn,line);
	
	if (file)
	{
		if (line[0] == '\a'){
			do
			{
				getline(datIn,line);
			} while(line[0]!='\a');

			getline(datIn,line);
		}
	}
	endText += line + "\n";

	while (getline(datIn, line))
		endText+=line+"\n";

	datIn.close();
	dat.open(DATPATH);
	dat << prevText + endText.substr(0, endText.length() - 1);
	dat.close();
}


/* Reads the given .dat file, reloads the entire memory structure and writes into
   Files as given in the file. */
void 
readDat () 
{
	vector<string> fileFolder;
	string line, fileName, content, path;

	datIn.open(DATPATH);

	while (!freeList.empty())
		freeList.pop();

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);


	while (getline(datIn, line)) 
	{
		current[0] = rootFolder;

		if (line[0] == 'D') 
		{
			path = line.substr(6, line.size() - 6);
			createFolder(path, false, 0);
			line = "";
		}
		else if (line[0] == 'F') 
		{
			line = line.substr(6, line.size() - 6);
			fileFolder = tokenize(line, '/');

			fileName = fileFolder.back();

			/* Remove .txt from file name */
			fileName = fileName.substr(0, fileName.size() - 4);

			/* Remove root and filename from path to get file directory */
			fileFolder.pop_back();
			line = "";

			/* Serializing tokens of path to string */
			if (fileFolder.size() != 0) 
			{
				for (int i = 0; i < fileFolder.size(); i++) 
					line += fileFolder[i] + "/";
				
				changeDir(line, 0);
			}

			tempFolder[0] = current[0];

			/* Create file in durectory */
			create(fileName, false, 0);
			line = "";
		} 

		/* Condition if data is to be written in file */
		else if (line[0] == '\a') 
		{
			/* Concatenate content until -1 is encountered again */
			while (getline(datIn, line)) 
			{
				if (line[0] == '\a')
					break;
				content += line + "\n";
			}

			content = content.substr(0, content.size() - 1);
			current[0] = tempFolder[0];

			/* Open file and write content */
			File openedFile(fileName + ".txt", "write", false, 0);
			openedFile.write(content, false, 0);
			content = "";
		}
	}

	current[0] = rootFolder;
	tempFolder[0] = current[0];
	datIn.close();
}