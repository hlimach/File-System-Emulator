#ifndef SERVER_H
#define SERVER_H

#include <unistd.h> 
#include <stdio.h> 
#include <iostream>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <vector>
#include <errno.h>
#define PORT 95

int establishConn(struct sockaddr_in &);
int getSocket(struct sockaddr_in &, int);

#endif