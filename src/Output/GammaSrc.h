/* $Id: GammaSrc.h,v 1.7 2002-08-05 20:23:19 fateneja Exp $ */
#include "alara.h"
#include <set>

// NEED COMMENT there are no comments for this class


/* ******* Class Description ************

 */

#ifndef _GAMMASRC_H
#define _GAMMASRC_H


#define GAMMASRC_RAW_SRC  1
#define GAMMASRC_CONTACT  2
#define GAMMASRC_ADJOINT  3

class GammaSrc
{
protected:
  
  DataLib *dataLib;

  int gammaType, nGroups;
  double *grpBnds;
  char *fileName;
  ofstream gSrcFile;
  ifstream gAttenData;
  double contactDose, *gammaAttenCoef;

  VectorCache gammaMultCache;

  int findGroup(float);
  double subIntegral(int,int,float*,float*,double,double);

public:

  GammaSrc(istream&, int);
  ~GammaSrc();

  void initRawSrc(istream&);
  void initContactDose(istream&);

  double* getGammaMult(int);
  char* getFileName()
    { return fileName; };
  void setData(int,int,int*,int*,int*,int**,int**,float**,float**,float**,float**);
  void writeIsoName(char *isoName)
    { gSrcFile << "\t" << isoName << endl;};
  void writeIsotope(double*,double);
  void writeTotal(double*,int);

  double calcDoseConv(int,double*);
  void setGammaAttenCoef(Mixture*);

  int getNumGrps()
    { return nGroups; };
  int getType()
    { return gammaType; } ;


};

#endif








