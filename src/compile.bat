@ECHO OFF
ECHO compiling . . .
sudo g++ -o main main.cpp util.cpp mem_struct.cpp globals.cpp dat.cpp file.cpp threads.cpp server.cpp -lpthread
ECHO done
PAUSE

clang++ -std=c++11 main.cpp util.cpp mem_struct.cpp globals.cpp dat.cpp file.cpp threads.cpp server.cpp