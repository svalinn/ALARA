/* $Id: alara.h,v 1.12 2002-05-06 18:03:54 wilsonp Exp $ */
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
#include <algorithm.h>

/* *** STL typedef ***

 DataCache : map<int,double,less<int> >
   This STL map provides a cache for scalar data mapped with the kza
   number to which that data corresponds.  
 */ 

#include <map.h>

/* TO DO: Come see me next week  Why won't it compile?!?!?*/

typedef map<int,double,less<int> > DataCache;
typedef map<int,double*,less<int> > VectorCache;

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
