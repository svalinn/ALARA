/* $Id: GammaSrc.h,v 1.13 2008-08-06 16:44:20 phruksar Exp $ */
#include "alara.h"
#include <map>
#include <vector>
#include <set>
#include <string>
// NEED COMMENT there are no comments for this class

/* ******* Class Description ************

 */

#ifndef GAMMASRC_H
#define GAMMASRC_H


#define GAMMASRC_RAW_SRC  1
#define GAMMASRC_CONTACT  2
#define GAMMASRC_ADJOINT  3
#define GAMMASRC_EXPOSURE 4
#define GAMMASRC_EXPOSURE_CYLINDRICAL_VOLUME 5

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
  double height;
  char media;
  float detvolume; // detector volume
  VectorCache gammaMultCache;

  int findGroup(float);
  double subIntegral(int,int,float*,float*,double,double);

  void initRawSrc(istream&);
  void initContactDose(istream&);
  void initAdjointDose(istream&);
  void initExposureDose(istream&);
  void initExposureCylVolDose(istream&);
  std::map<int, double> exposureDoseCache;
  std::vector< double > gammaAbsAir; //Mass absorption attenuation in air
  float G_factor(float,float,float,float);
  void calcBuildupParameters(double, char, double&, double&, double&, double&);

public:

  GammaSrc(istream&, int);
  ~GammaSrc();

  double* getGammaMult(int);
  char* getFileName()
    { return fileName; };
  void setData(int,int,int*,int*,int*,int**,int**,float**,float**,float**,float**);
  void writeIsoName(char *isoName, std::string coolTime)
    { gSrcFile << isoName << "\t" << coolTime;};
  void writeIsotope(double*,double);
  void writeTotal(double*,int,std::vector<std::string>);

  double calcDoseConv(int,double*);
  double calcAdjDose(int,double*,double); 
  double calcExposureDoseConv(int,double*);
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








