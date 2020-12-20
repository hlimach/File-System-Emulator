#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stack>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <assert.h>
#include <thread>

#define PAGESIZE 16						 /* Size of each page in memory. */
#define MEMSIZE 16384					 /* Total memory reserved for file data. */
#define NUMPAGES (MEMSIZE / PAGESIZE)	 /* Total pages in memory. */
#define LASTENTRY ((PAGESIZE / 2) - 1)	 /* Last entry point for a page table. */
#define MAXENTRIES ((PAGESIZE / 2) - 3)	 /* Maximum entries possible in a page table */
#define DATPATH "../disk_data.dat"

using namespace std;

#endif