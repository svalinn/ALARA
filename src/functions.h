/* $Id: functions.h,v 1.8 2003-10-28 22:11:39 wilsonp Exp $ */
/* input_file_utils.C */
istream* openFile(const char*);
void clearComment(istream& input);
int tokenType(char*);
double convertTime(double,char);

char* searchNonXSPath(const char* filename);
char* searchXSPath(const char* filename);
char* searchPath(const char* filename, const char* envPathVar, const char* builtinPathVar);


/* math.C */
double fact(int);

double bateman(int,int,double*,double, int&);

double dGn(int, double*, int*, int, int);
double laplaceInverse(int, int, double*, double, int&);

int smallExpansion(int,int,double*,double);
double laplaceExpansion(int, int, double*, double, int&);


double fillTElement(int, int, double*, double*, double, int*,int);


/* output.C */
char* symbol(int,char*);
char* isoName(int,char*);

