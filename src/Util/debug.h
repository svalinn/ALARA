/* $Id: debug.h,v 1.2 1999-08-24 22:06:26 wilson Exp $ */
void verbose(int, char*, ...);
void debug(int, char*, ...);
void error(int, char*, ...);
void warning(int, char*, ...);

void memCheck(void*, const char*);
