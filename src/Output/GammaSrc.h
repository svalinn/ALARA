/* $Id: GammaSrc.h,v 1.11 2007-10-18 20:30:36 phruksar Exp $ */
#include "alara.h"
#include <map>
#include <vector>
#include <set>
// NEED COMMENT there are no comments for this class

/* ******* Class Description ************

 */

#ifndef _GAMMASRC_H
#define _GAMMASRC_H


#define GAMMASRC_RAW_SRC  1
#define GAMMASRC_CONTACT  2
#define GAMMASRC_ADJOINT  3
#define GAMMASRC_EXPOSURE 4

class GammaSrc
{
protected:
  
  DataLib *dataLib;

  int gammaType, nGroups;
  double *grpBnds;
  int *intervalptr; 
  char *fileName;
  ofstream gSrcFile;
  ifstream gDoseData;
  double contactDose, *gammaAttenCoef, adjDose, exposureDose;
  //These variables are for calculating exposure dose
  double radius, distance;
  float detvolume; // detector volume
  VectorCache gammaMultCache;

  int findGroup(float);
  double subIntegral(int,int,float*,float*,double,double);

  void initRawSrc(istream&);
  void initContactDose(istream&);
  void initAdjointDose(istream&);
  void initExposureDose(istream&);
  std::map<int, double> exposureDoseCache;
  std::vector< double > gammaAbsAir; //Mass absorption attenuation in air

public:

  GammaSrc(istream&, int);
  ~GammaSrc();

  double* getGammaMult(int);
  char* getFileName()
    { return fileName; };
  void setData(int,int,int*,int*,int*,int**,int**,float**,float**,float**,float**);
  void writeIsoName(char *isoName)
    { gSrcFile << "\t" << isoName << endl;};
  void writeIsotope(double*,double);
  void writeTotal(double*,int);

  double calcDoseConv(int,double*);
  double calcAdjDose(int,double*,double); 
  double calcExposureDoseConv(int);
  void setGammaAttenCoef(Mixture*);
  void setAdjDoseData(Volume*);

  int getNumGrps()
    { return nGroups; };
  int getType()
    { return gammaType; } ;
  int* getintervalptr()
    { return intervalptr; }; 

};

#endif








