/* $Id: alara.h,v 1.7 2000-01-20 05:06:50 wilson Exp $ */
#ifndef _ALARA_H
#define _ALARA_H 1


#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <algorithm>

/* *** STL typedef ***

 DataCache : map<int,double,less<int> >
   This STL map provides a cache for scalar data mapped with the kza
   number to which that data corresponds.  
 */ 

#include <map>

typedef map<int,double,less<int> > DataCache;

class compare {
public:
  bool operator()(const char *s,const char *t)const
    { return strcmp(s,t) < 0;
    }
};


#ifndef PI
#define PI M_PI
#endif

#define TRUE (1==1)
#define FALSE (0==1)

#define SLONG sizeof(long)
#define SINT sizeof(int)
#define SFLOAT sizeof(float)
#define SDOUBLE sizeof(double)

/* remove this definition because it is part of STL */
/* #define max(x,y) (x>y?x:y) */

#define MODE_FORWARD 1
#define MODE_REVERSE 0

#include "Util/classes.h"

#include "Util/debug.h"

#include "Util/functions.h"

#include "Util/Statistics.h"

extern int chainCode;
extern const char* SYMBOLS;

extern int verb_level;
extern int debug_level;


#endif
