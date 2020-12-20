#ifndef DAT_H
#define DAT_H

#include "config.h"

void openStream (bool);
void closeStream ();
void enterDat (string, bool, string);
void moveDat (string, string);
void removeDat (string, bool);
void readDat ();

#endif