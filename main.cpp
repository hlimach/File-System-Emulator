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

class FileHead {
    string filename;
    int size;
    char* directory;
};

char* getSector(int x) {
    char* sectorNo;
    sectorNo = (char*) start + (SECTORSIZE * x);
    return sectorNo;
}

void Create(string filename);
void Delete(string filename);
void Open(string filename);
void Close(string filename);
void List();
void Write(string filename, char mode);
void Read(string filename);
void Read(string filename, int startFrom);


int main(int argc, const char * argv[]) {
    
    memory mem;

    start = (char*) malloc(16777216);
    
    
    while(true) {
        int command, readByte;
        string filename;
        char writeMode;

        cout << "Please enter one of the following command integer:" << endl;
        cout << "1: Create(filename) \t\tCreate a text file of name <filename>" << endl;
        cout << "2: Delete(filename) \t\tDelete a text file of name <filename>" << endl;
        cout << "3: Open(filename) \t\tOpen a text file of name <filename>" << endl;
        cout << "4: Close(filename) \t\tClose an opened text file of name <filename>" << endl;
        cout << "5: List files \t\t\tList all the files in the current directory" << endl;
        cout << "6: Read(filename) \t\tRead a text file of name <filename> from start" << endl;
        cout << "7: Read(filename, startFrom) \tRead a text file of name <filename> from the byte number given by <startFrom>" << endl;
        cout << "8: Write(filename,mode) \t\tWrite to text file of name <filename> with the given mode\n\t\t\t\t\tMode: a (append), w (write)" << endl;
        cout << "9: End Program" << endl;
        
        cin >> command;
        
        
        switch (command) {
        case(1):
            cout << "\nEnter File name: ";
            cin >> filename;
            break;
        case(2):
            cout << "\nEnter File name: ";
            cin >> filename;
            break;
        case(3):
            cout << "\nEnter File name: ";
            cin >> filename;
            break;
        case(4):
            cout << "\nEnter File name: ";
            cin >> filename;
            break;
        case(5):
            cout << "These are the files: \n";
            break;
        case(6):
            cout << "\nEnter File name: ";
            cin >> filename;
            cout << "\nEnter write mode: ";
            cin >> writeMode;
            break;
        case(7):
            cout << "\nEnter File name: ";
            cin >> filename;
            break;
        case(8):
            cout << "\nEnter File name: ";
            cin >> filename;
            cout << "\nEnter start byte: ";
            cin >> readByte;
            break;
        case(9):
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



    
