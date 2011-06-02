/* $Id: debug.h,v 1.2 1999-08-24 22:06:26 wilson Exp $ */
void verbose(int, const char*, ...);
void debug(int, const char*, ...);
void error(int, const char*, ...);
void warning(int, const char*, ...);

void memCheck(void*, const char*);
