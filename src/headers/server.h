#ifndef SERVER_H
#define SERVER_H

#include <unistd.h> 
#include <stdio.h> 
#include <iostream>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <thread>
#include <vector>
#include <errno.h>
#include <array>
#define PORT 95

void thFunc(int);
int establishConn(struct sockaddr_in &);
int getSocket(struct sockaddr_in &, int);

#endif