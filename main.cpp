//
//  main.cpp
//  OS
//
//  Created by Haleema Ramzan on 11/25/20.
//
#include <iostream>
#include <cstdlib>
#include <cstring>

#define SECTORSIZE 256

using namespace std;

const char* start;

struct memory {
    unsigned address: 24; //24 bits unsigned value - 16Mb total memory
    unsigned sectorAdd: 8; //8 bits unsigned value - 256 bytes per sector
};

char* getSector(int x) {
    char* sectorNo;
    sectorNo = (char*) start + (SECTORSIZE * x);
    return sectorNo;
}

string setFilename() {
    string filename;
    
    cout << "Enter File name: ";
    cin >> filename;
    
    return filename;
}

void create() {
    setFilename();
}

void deleteFile() {
    setFilename();
}

void open() {
    setFilename();
}

void close() {
    setFilename();
}

void list () {
    setFilename();
}

void write() {
    char mode;
    
    cout << "Enter mode: ";
    cin >> mode;
    
    while(mode != 'a' && mode != 'w') {
        cout << "Enter a valid mode!" << endl;
        cout << "Enter mode: ";
        cin >> mode;
    }
    
    setFilename();
    
}
void read () {
    setFilename();
}

void read (int startFrom) {
    setFilename();
}


bool isNumber(string s)
{
    for (int i = 0; i < s.length(); i++)
        if (isdigit(s[i]) == false)
            return false;
 
    return true;
}

int main(int argc, const char * argv[]) {
    
    memory mem;

    start = (char*) malloc(16777216);
    
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
                deleteFile();
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
                while(true) {
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

    
    for (int i = 0; i < 10; i++)
        cout << *getSector(i) << endl;
    
    free((char*) start);
    
    return 0;
}



    
