/* $Id: alara.h,v 1.18 2003-10-22 06:07:02 wilsonp Exp $ */
#ifndef ALARA_H
#define ALARA_H 1

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>


#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

/* *** STL typedef ***

 DataCache : map<int,double,less<int> >
   This STL map provides a cache for scalar data mapped with the kza
   number to which that data corresponds.  
 */ 

#include <map>

/* TO DO: Come see me next week  Why won't it compile?!?!?*/

typedef std::map<int,double,std::less<int> > DataCache;
typedef std::map<int,double*,std::less<int> > VectorCache;
typedef std::map<int, double*, std::less<int> > TempLibType;

/* import std::istream, std::ofstream since alternative implmentations
 * are unlikely */
using std::istream;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::ios;

using std::fstream;
using std::ifstream;
using std::ofstream;

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

// Avagadros number
#define AVAGADRO 6.02e23

#endif
