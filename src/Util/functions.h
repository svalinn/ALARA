/* input.C */
istream* openFile(char*);
void clearComment(istream& input);
int tokenType(char*);
double convertTime(double,char);


/* math.C */
double fact(int);

double bateman(int,int,double*,double);

double dGn(int, double*, int*, int, int);
double laplaceInverse(int, int, double*, double);

double fillTElement(int, int, double*, double*, double, int*);


/* output.C */
char* symbol(int,char*);
char* isoName(int,char*);

