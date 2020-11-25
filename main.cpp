//
//  main.cpp
//  OS
//
//  Created by Haleema Ramzan on 11/25/20.
//
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stack>
#include <string>

#define SECTORSIZE 256
#define MEMSIZE 16777216
#define NUMSECTORS 65536
using namespace std;

const char* start;
stack<int> freeList;
map<string, int*> root;
struct memory {
	unsigned address : 24; //24 bits unsigned value - 16Mb total memory
	unsigned sectorAdd : 8; //8 bits unsigned value - 256 bytes per sector
};

char* getSector(int x) {
	char* sectorNo;
	sectorNo = (char*)start + (SECTORSIZE * x);
	return sectorNo;
}

int getEntry(int* x) {
	int entry = (x - (int*)start) / SECTORSIZE;
	return entry;
}

string setFilename() {
	string filename;

	cout << "Enter File name: ";
	cin >> filename;

	return filename;
}

void create() {

	string filename = setFilename();
	int freeSect = freeList.top();
	freeList.pop();
	char* sector = getSector(freeSect);
	root[filename] = (int*)sector;
}


void deleteFile(string filename) {
	int* pageTable = root[filename];
	int pageNumber;
	int i;
	stack <int> temp;
	for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
		pageNumber = *(pageTable + i);
		temp.push(pageNumber);
	}
	pageNumber = *(pageTable + i);
	temp.push(pageNumber);

	while (!temp.empty()) {

		freeList.push(temp.top());
		temp.pop();
	}
	freeList.push(getEntry(pageTable));
	cout << freeList.top() << endl;
}

void open() {
	setFilename();
}

void close() {
	setFilename();
}

void list() {
	setFilename();
}

void write() {
	char mode;
	int writePtr = 0;
	cout << "Enter mode: ";
	cin >> mode;

	while (mode != 'a' && mode != 'w') {
		cout << "Enter a valid mode!" << endl;
		cout << "Enter mode: ";
		cin >> mode;
	}

	string filename = setFilename();
	int* pageTable = root[filename];
	char* page = NULL;

	string input, line;

	int count = 0;
	while (getline(cin, line))
	{
		if (line == "-1")
			break;
		input += line;
		if (count != 0)
			input += "\n";
		count++;
	}
	cout << input.length() << endl;
	int numberOfSectors = input.length() / SECTORSIZE + 1;
	int limit = input.length() % SECTORSIZE;
	cout << limit << endl;

	switch (mode)
	{
	case('w'):
		//deleteFile(filename);
		//create(filename);



		for (int i = 0; i < numberOfSectors * 2; i += 2) {

			*(pageTable + i) = freeList.top();

			if (i == numberOfSectors * 2 - 2)
				* (pageTable + i + 1) = limit;
			else
				*(pageTable + i + 1) = NULL;

			freeList.pop();
		}
		for (int i = 0; i < numberOfSectors * 2; i += 2) {

			page = getSector(*(pageTable + i));

			if (i != numberOfSectors * 2 - 2) {
				for (int j = 0; j < SECTORSIZE; j++) {
					cout << input[((i / 2) * SECTORSIZE) + j] << endl;
					*(page + j) = input[((i / 2) * SECTORSIZE) + j];
				}
			}
			else {
				for (int j = 0; j < limit; j++) {
					cout << input[((i / 2) * SECTORSIZE) + j] << endl;
					*(page + j) = input[((i / 2) * SECTORSIZE) + j];
				}
			}
		}
		for (int k = 0; k < 10; k++) {
			cout << *(pageTable + k) << endl;
		}

	default:
		break;
	}

}
void read() {
	cout << "in read" << endl;
	string filename = setFilename();
	int* pageTable = root[filename];
	char* page = NULL;
	int i;
	for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
		page = getSector(*(pageTable + i));
		for (int j = 0; j < SECTORSIZE; j++)
			cout << *(page + j);
	}
	int limit = *(pageTable + i + 1);
	page = getSector(*(pageTable + i));
	for (int j = 0; j < limit; j++)
		cout << *(page + j);
	cout << "printed" << endl;
}

void read(int startFrom) {
	setFilename();
}


bool isNumber(string s)
{
	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;
}

int main(int argc, const char* argv[]) {

	start = (char*)malloc(MEMSIZE);

	for (int i = NUMSECTORS; i >= 0; i--)
		freeList.push(i);
	string filename;
	bool loop = true;
	while (loop) {
		int command, readByte;
		string startFrom;

		cout << "Please enter one of the following command integer:" << endl;
		cout << "1: Create(filename) \t\t\tCreate a text file of name <filename>" << endl;
		cout << "2: Delete(filename) \t\t\tDelete a text file of name <filename>" << endl;
		cout << "3: Open(filename) \t\t\t\tOpen a text file of name <filename>" << endl;
		cout << "4: Close(filename) \t\t\t\tClose an opened text file of name <filename>" << endl;
		cout << "5: List files \t\t\t\t\tList all the files in the current directory" << endl;
		cout << "6: Read(filename) \t\t\t\tRead a text file of name <filename> from start" << endl;
		cout << "7: Read(filename, startFrom) \tRead a text file of name <filename> from the byte number given by <startFrom>" << endl;
		cout << "8: Write(filename,mode) \t\tWrite to text file of name <filename> with the given mode\n\t\t\t\t\t\t\t\tMode: a (append), w (write)" << endl;
		cout << "9: End Program" << endl;
		cout << "\nEnter command: ";
		cin >> command;


		switch (command) {
		case(1):
			create();
			break;
		case(2):
			filename = setFilename();
			deleteFile(filename);
			break;
		case(3):
			open();
			break;
		case(4):
			close();
			break;
		case(5):
			list();
			break;
		case(6):
			read();
			break;
		case(7):
			while (true) {
				cout << "Enter starting byte number: ";
				cin >> startFrom;

				if (isNumber(startFrom)) {
					read(stoi(startFrom));
					break;
				}
				else
					cout << "Enter a valid byte number!" << endl;
			}

			break;
		case(8):
			write();
			break;
		case(9):
			loop = false;
			break;
		default:
			cout << "Please enter a valid integer command!" << endl;
			break;
		}
	}




	free((char*)start);

	return 0;
}




