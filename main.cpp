#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <stack>
#include <string>

#define PAGESIZE 256		// size of each page in memory
#define MEMSIZE 16777216	// total memory size - 16Mb
#define NUMPAGES 65536	// total pages in memory
using namespace std;

const char* start;

// freeList holds ineger values of all sectors that are unused
stack <int> freeList;

// holds values in pairs of filename against pointer to its page table
map <string, int*> root;


/* 
	function getSector takes a sector's integer value 
	and returns a pointer to that sector 
*/
char* getSector(int x) {

	return (char*) start + (PAGESIZE * x);

}



/* 
	function getEntry takes a pointer as argument
	it finds the page number where this pointer will be found
	returns the page number as integer
*/
int getEntry(int* x) {

	return (x - (int*) start) / PAGESIZE;

}



/* 
	explain
	function
*/
string setFilename() {

	string filename;

	cout << "Enter File name: ";
	cin >> filename;

	return filename;

}



/* 
	explain
	function
*/
bool isNumber(string s) {

	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;

}



/* 
	explain
	function
*/
void create() {

	string filename = setFilename();

	int freeSect = freeList.top();
	freeList.pop();

	char* sector = getSector(freeSect);
	root[filename] = (int*) sector;

}



/* 
	function getFileSize takes in filename as argument 
	accesses the files' page table and iterates it
	returns the file size in bytes
*/
int getFileSize(string filename) {

	int* pageTable = root[filename];
	char* page = NULL;
	int size = 0, i = 0;


	// loop runs until the limit field found to be non-zero (last sector)
    while (*(pageTable + i + 1) == 0) {
        size += PAGESIZE;
        i += 2;
    }


	// i is now at the last entry of the files' page table
	// limit value obtained and added to size
	size += *(pageTable + i + 1);


	return size;

}



/* 
	explain
	function
*/
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



/* 
	explain
	function
*/
void open() {
	setFilename();
}



/* 
	explain
	function
*/
void close() {
	setFilename();
}



/* 
	explain
	function
*/
void list() {
	setFilename();
}



/* 
	explain
	function
*/
void write() {
	char mode;
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
	while (getline(cin, line)) {
		if (line == "-1")
			break;
		input += line;
		if (count != 0)
			input += "\n";
		count++;
	}

	cout << input.length() << endl;
	int numberOfSectors, limit, appendPoint, appendPage, appendSector, remainder, countSectors;

	switch (mode) {
	case('w'):

		numberOfSectors = input.length() / PAGESIZE + 1;
		limit = input.length() % PAGESIZE;
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
				for (int j = 0; j < PAGESIZE; j++) {
					*(page + j) = input[((i / 2) * PAGESIZE) + j];
				}
			}
			else {
				for (int j = 0; j < limit; j++) {
					*(page + j) = input[((i / 2) * PAGESIZE) + j];
				}
			}
		}

		break;
	
	case('a'):

		int i;
		countSectors = 0;
		
		for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
			countSectors++;
		
		}
		countSectors++;
		appendPoint = *(pageTable + i + 1);
		appendPage = *(pageTable + i);

		page = getSector(appendPage);
		
		for (int j = appendPoint; j < PAGESIZE; j++) {
			*(page + j) = input[j - appendPoint];
		}
		
		*(pageTable + i + 1) = NULL;
		remainder = PAGESIZE - appendPoint;

		numberOfSectors = (input.length() - remainder) / PAGESIZE + 1;
		limit = (input.length() - remainder) % PAGESIZE;
		numberOfSectors = numberOfSectors + countSectors;
		appendSector = i;
		for (i = appendSector + 2; i < numberOfSectors * 2; i += 2) {

			*(pageTable + i) = freeList.top();

			if (i == numberOfSectors * 2 - 2)
				* (pageTable + i + 1) = limit;
			else
				*(pageTable + i + 1) = 0;

			freeList.pop();
		}
		
		//writing
		
		for (i = appendSector+2; i < numberOfSectors * 2; i += 2) {

			page = getSector(*(pageTable + i));

			if (i != numberOfSectors * 2 - 2) {
				for (int j = 0; j < PAGESIZE; j++) {
					cout << j << endl;
					*(page + j) = input[(((i - appendSector - 2) / 2) * PAGESIZE) + j + (remainder)];
				}
			}
			else {
				for (int j = 0; j < limit; j++) {
					*(page + j) = input[(((i-appendSector-2) / 2) * PAGESIZE) + j + (remainder)];
				}
			}
		}
		break;

	default:
		break;

	}

}



/* 
	explain
	function
*/
void read() {
	string filename = setFilename();
	int* pageTable = root[filename];
	char* page = NULL;
	int i;


	// loop runs until the limit field found to be non-zero (last sector)
	for (i = 0; *(pageTable + i + 1) == 0; i += 2) {
		page = getSector(*(pageTable + i));


		// loops inside the sector pointed to by char pointer 'page'
		for (int j = 0; j < PAGESIZE; j++)
			cout << *(page + j);

	}


	// i is now at the last entry of the files' page table so its limit value is obtained
	int limit = *(pageTable + i + 1);

	// pointer to the last page is obtained 
	page = getSector(*(pageTable + i));

	// loops over the last page until the limit that was previously obtained
	for (int j = 0; j < limit; j++)
		cout << *(page + j);

}



/* 
	explain
	function
*/
void read(string filename, int startFrom) {
	int* pageTable = root[filename];
	char* page = NULL;

	// calculate which page number in the page table the entry will belong to
	int pageNum = (startFrom / PAGESIZE) + 1; // 2


	// calculate this pages' index in the page table
	int i = (pageNum * 2) - 2; // 2


	/*
		calculate the total number of pages of the file
		loop runs until last page found
		2 entire of last page added, and divided to get count
	*/
	int totalPages = 0;

	while (*(pageTable + totalPages + 1) == 0)
		totalPages += 2;

	totalPages = (totalPages + 2) / 2; //4


	// obtain this page to start reading from it
	page = getSector(*(pageTable + i));


	if (pageNum != totalPages) {

		for (int j = startFrom - ((pageNum - 1) * PAGESIZE); j < PAGESIZE; j++)
		cout << *(page + j);


		// go to next page and loop until the limit field found to be non-zero (last page)
		for (i += 2; *(pageTable + i + 1) == 0; i += 2) {
			page = getSector(*(pageTable + i));


			// loops inside the sector pointed to by char pointer 'page'
			for (int j = 0; j < PAGESIZE; j++)
				cout << *(page + j);

		}


		// i is now at the last entry of the files' page table. 
		// It's limit value and pointer to the last page is obtained
		int limit = *(pageTable + i + 1);
		page = getSector(*(pageTable + i));

		// loops over the last page until the limit that was previously obtained
		for (int j = 0; j < limit; j++)
			cout << *(page + j);

	}
	else {

		// obtain this pages' limit
		int limit = *(pageTable + i + 1);


		// start reading from given byte upto the page limit
		for (int j = startFrom - ((totalPages - 1) * PAGESIZE); j < limit; j++)
			cout << *(page + j);

	}

	
}



/* 
	explain
	function
*/
int main(int argc, const char* argv[]) {

	start = (char*)malloc(MEMSIZE);

	for (int i = NUMPAGES; i >= 0; i--)
		freeList.push(i);
	string filename;
	bool loop = true;
	while (loop) {
		int command, size;
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

				filename = setFilename();
				size = getFileSize(filename);

				while (true) {
					cout << "Enter starting byte number (within file size:" << size <<  " bytes) : ";
					cin >> startFrom;

					if (isNumber(startFrom)) {

						if (stoi(startFrom) <= size) {
							read(filename, stoi(startFrom));
							break;
						} 
						else 
							cout << "Enter a valid byte number!" << endl;
						
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




	free((char*) start);

	return 0;
}




