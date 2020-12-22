@ECHO OFF
ECHO compiling . . .
g++ -o main main.cpp util.cpp mem_struct.cpp globals.cpp dat.cpp file.cpp threads.cpp
ECHO done
PAUSE