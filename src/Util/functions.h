/* $Id: functions.h,v 1.5 1999-08-24 22:06:26 wilson Exp $ */
/* input.C */
istream* openFile(char*);
void clearComment(istream& input);
int tokenType(char*);
double convertTime(double,char);


/* math.C */
double fact(int);

double bateman(int,int,double*,double, int&);

double dGn(int, double*, int*, int, int);
double laplaceInverse(int, int, double*, double, int&);

double laplaceExpansion(int, int, double*, double, int&);


double fillTElement(int, int, double*, double*, double, int*,int);


/* output.C */
char* symbol(int,char*);
char* isoName(int,char*);

