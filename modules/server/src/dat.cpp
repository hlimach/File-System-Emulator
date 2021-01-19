#include "../include/config.h"
#include "../include/globals.h"
#include "../include/dat.h"
#include "../include/util.h"
#include "../include/file.h"


/* opens file stream whenever required to read or write to .dat file */
void
openStream()
{
	datMtx.lock();
	datStream.open(DATPATH, ios::out | ios::in | ios::app);

	if(!datStream.is_open())
    	cout << "Error opening " << DATPATH << ".\n";
}


/* closes file stream when read or write operation is completed */
void
closeStream()
{
	datStream.close();
	datMtx.unlock();
}

/* Whenever a new file or folder is created, this function is called so it appends
   To the end of the existing .dat file, updating it. */
void
enterDat (string path, bool file, string name)
{
	string data = "";
	if(file)
		data = "F\t" + path + "/" + name;
	else
		data = "D\t" + path + "/" + name;

	openStream();
	datStream << data << endl;
	closeStream();
}


/* Whenever a file is moved, this function is called. It gets the path of the file,
   Removes its entry from the .dat file, and adds it to the end with the updated
   Path. */
void
moveDat (string oldPath, string newPath)
{
	ofstream overWriteDat;
	string line = "", prevText = "", data = "";
	openStream();

	while (getline(datStream, line))
	{
		if (line[0] == 'F')
			if (line.substr(2, line.size() - 2) == oldPath)
				break;

		prevText += line + "\n";
	}
	getline (datStream, line);

	if (line[0] == '\a')
	{
		data += line + "\n";
		do
		{
			getline(datStream,line);
			data += line + "\n";
		} while(line[0]!='\a');

		getline(datStream,line);
	}
	if(!datStream.eof())
	{
		prevText += line + "\n";

		while (getline(datStream, line))
			prevText += line + "\n";
	}

	overWriteDat.open(DATPATH);
	overWriteDat << prevText + "F\t" + newPath + "\n" + data.substr(0, data.length() - 1);
	overWriteDat.close();
	closeStream();
}


/* Whenever a file or folder is deleted, it removes its entry from the .dat file. */
void
removeDat (string path, bool file)
{
	ofstream overWriteDat;
	string line = "", prevText = "", endText = "";
	openStream();

	while (getline(datStream, line))
	{
		if (line[0] == 'F' || line[0] == 'D')
			if (line.substr(2, line.size() - 2) == path)
				break;

		prevText += line + "\n";
	}

	if (datStream.eof())
	{
		overWriteDat.open(DATPATH);
		overWriteDat << prevText;
		overWriteDat.close();
		closeStream();
		return;
	}

	getline(datStream,line);

	if (file)
	{
		if (line[0] == '\a'){
			do
			{
				getline(datStream,line);
			} while(line[0]!='\a');

			getline(datStream,line);
		}
	}
	endText += line + "\n";

	while (getline(datStream, line))
		endText+=line+"\n";

	endText = endText.substr(0, endText.length() - 1);

	overWriteDat.open(DATPATH);

	// if (endText[endText.size() - 1] == '\a')
	// 	endText = endText.substr(0, endText.length() - 1);

	overWriteDat << prevText + endText;
	overWriteDat.close();
	closeStream();
}


/* Reads the given .dat file, reloads the entire memory structure and writes into
   Files as given in the file. */
void
readDat ()
{
	vector<string> fileFolder;
	string line, fileName, content, path;

	openStream();
	datStream.seekg(0);

	while (!freeList.empty())
		freeList.pop();

	for (short int i = NUMPAGES - 1; i >= 0; i--)
		freeList.push(i);


	while (getline(datStream, line))
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

				changeDir(line, 0, false);
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
			while (getline(datStream, line))
			{
				if (line[0] == '\a')
					break;
				content += line + "\n";
			}

			content = content.substr(0, content.size() - 1);
			current[0] = tempFolder[0];

			/* Open file and write content */
			File openedFile(fileName + ".txt", "write", false, 0);
			openedFile.write(content, false);
			content = "";
		}
	}

	current[0] = rootFolder;
	tempFolder[0] = current[0];
	closeStream();
}