/* input.C */
istream* openFile(char*);
void clearComment(istream& input);
int tokenType(char*);
double convertTime(double,char);


/* math.C */
double fact(int);

double bateman(int,int,double*,double*,double);

double loopSoln(int,int,double*,double*,double);

double dGn(int, double*, int*, int, int);
double laplaceInverse(int, int, double*, double);

int small(double*,double,int);
double laplaceExpansion(int, int, double*, double);


/* output.C */
char* symbol(int,char*);
char* isoName(int,char*);

