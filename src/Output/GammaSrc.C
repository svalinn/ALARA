/* $Id: GammaSrc.C,v 1.22 2007-10-19 15:41:47 phruksar Exp $ */
#include "GammaSrc.h"

#include "DataLib/DataLib.h"
#include "Input/Mixture.h"
#include "Input/Volume.h"
#include <math.h>
/***************************
 ********* Service *********
 **************************/

GammaSrc::GammaSrc(istream& input, int inGammaType)
{
  
  char libType[9];

  gammaType = inGammaType;

  strcpy(libType,"gammalib");
  nGroups = 0;
  contactDose = 0;
  adjDose = 0; 
  exposureDose = 0;
  detvolume = 1; 
  grpBnds = NULL;
  dataLib = NULL;
  gammaAttenCoef = NULL;
  intervalptr = NULL; 

  /* get gamma library filename */
  dataLib = DataLib::newLib(libType,input);

  switch(gammaType)
    {
    case GAMMASRC_RAW_SRC:
      initRawSrc(input);
      break;
    case GAMMASRC_CONTACT:
      initContactDose(input);
      break;
    case GAMMASRC_ADJOINT:
      initAdjointDose(input); 
      break;
    case GAMMASRC_EXPOSURE:
      initExposureDose(input); 
      break;
    }

}

void GammaSrc::initRawSrc(istream& input)
{

  char token[64];
  int gNum;

  /* get gamma source output filename */
  clearComment(input);
  input >> token;
  fileName = new char[strlen(token)+1];
  strcpy(fileName,token);
  gSrcFile.open(fileName);
  if (!gSrcFile)
    error(250,"Unable to open file for gamma source output: %s\n",
	  fileName);
  
  /* get number of gamma groups to use */
  clearComment(input);
  input >> nGroups;
  /* error checking */

  grpBnds = new double[nGroups+1]; 
  /* memcheck */

  grpBnds[0] = 0;
  /* read gamma group bounds */
  clearComment(input);
  for (gNum=1;gNum<nGroups+1;gNum++)
    input >> grpBnds[gNum];

}

void GammaSrc::initExposureDose(istream& input)
{
  char token[64];
  int gNum;
  
  /* get gamma attenuation data file */
  clearComment(input);
  input >> token;
  fileName = new char[strlen(token)+1];
  strcpy(fileName,token);
  gDoseData.open(searchNonXSPath(fileName));
  if (!gDoseData)
    error(250,
	  "Unable to open file for gamma attenuation data input (contact dose): %s\n",
	  fileName);

  /* get number of data points used for this data */
  clearComment(gDoseData);
  gDoseData >> nGroups;

  grpBnds = new double[nGroups+1];
  grpBnds[0] = 0.0;

  /* read gamma energy points */
  for (gNum=1;gNum<nGroups+1;gNum++)
    {
      gDoseData >> grpBnds[gNum];
      grpBnds[gNum] *= 1.0E6;
    }

  /* read gamma energy absorption coefficient for air */
  gammaAbsAir.resize(nGroups,0);
  for (gNum=0;gNum<nGroups;gNum++)
     gDoseData >> gammaAbsAir[gNum];

  //Radius and distance are needed for infinite line approximation
  input >> radius;
  input >> distance;

}

void GammaSrc::initContactDose(istream& input)
{

  char token[64];
  int gNum;

  /* get gamma attenuation data file */
  clearComment(input);
  input >> token;
  fileName = new char[strlen(token)+1];
  strcpy(fileName,token);
  gDoseData.open(searchNonXSPath(fileName));
  if (!gDoseData)
    error(250,
	  "Unable to open file for gamma attenuation data input (contact dose): %s\n",
	  fileName);

  /* get number of data points used for this data */
  clearComment(gDoseData);
  gDoseData >> nGroups;

  grpBnds = new double[nGroups+1];
  grpBnds[0] = 0.0;

  /* read gamma energy points */
  for (gNum=1;gNum<nGroups+1;gNum++)
    {
      gDoseData >> grpBnds[gNum];
      grpBnds[gNum] *= 1.0E6;
    }


}

void GammaSrc::initAdjointDose(istream& input)
{

  char token[64];
  int gNum;
  int suminterval;

  /* get adjoint dose field information from file */
  clearComment(input);
  input >> token;
  fileName = new char[strlen(token)+1];
  strcpy(fileName,token);
  gDoseData.open(fileName);
  if (!gDoseData)
    error(260,
	  "Unable to open file for adjoint field data input (adjoint dose): %s\n",
	  fileName);

  /* get number of data points used for this data */
  clearComment(input);
  input >> nGroups;  

  grpBnds = new double[nGroups+1];
  grpBnds[0] = 0.0;

  /* read gamma energy points */
  clearComment(input); 
  for (gNum=1;gNum<nGroups+1;gNum++)
    {
      input >> grpBnds[gNum];
    }
  /* read detector volume */
  input >> detvolume;


  /* read number of interval to be summed */
//input >> suminterval;
//  if (suminterval>0)
// 	intervalptr=new int[suminterval*2+1];
//  else
//	intervalptr=new int(0);  // in case of no req sum interval set to 0
    
//  intervalptr[0]=suminterval;    
//  for (int counter=1;counter<=suminterval*2;counter++)
// 	input >> intervalptr[counter]; 

}

GammaSrc::~GammaSrc()
{

  /* empty VectorCache */
  /* foreach entry in cache... */
  /* delete vector pointed to by entry */

  delete dataLib;
  delete grpBnds;
  delete intervalptr;

}

void GammaSrc::setGammaAttenCoef(Mixture* mixList)
{

  mixList->setGammaAttenCoef(nGroups,gDoseData);

}

void GammaSrc::setAdjDoseData(Volume* volList)
{

 volList->readAdjDoseData(nGroups,gDoseData);

}

/* routine to determine which energy group a particular gamma ray is in */
int GammaSrc::findGroup(float E)
{
  int gNum, gMin = 0, gMax = nGroups-1;

  if (E < grpBnds[gMin])
    {
      /* ERROR - energy < 0 */
    }
  if (E > grpBnds[gMax])
    return gMax;

  while (1)
    {
      gNum = (gMin+gMax)/2;
      if (E < grpBnds[gNum])
	gMax = gNum;
      else
	if (E > grpBnds[gNum+1])
	  gMin = gNum+1;
	else
	  return gNum;
    }
  
}


double GammaSrc::subIntegral(int pntNum, int intTyp, float* x, float* y, 
			     double xlo, double xhi)
{
  double m,I=0;
  double x1 = x[pntNum];
  double x2 = x[pntNum+1];
  double y1 = y[pntNum];
  double y2 = y[pntNum+1];

  /* based on interpolation type */
  switch (intTyp)
    {
    case 1: /* type 1: histogram */
      /* y = y1 */
      /* I = y1*x @ dx */
      I = (xhi-xlo)*y1;
      break;
    case 2: /* type 2: linear */
      /* y = m*x + (y1-m*x1) */
      /* I = 0.5*m*x^2 + (y1-m*x1)*x @ dx */
      /* find m */
      m =(y2-y1)/(x2-x1); 
      /* area of trapezoid */
      //I = (xhi-xlo)*(0.5*(xhi + xlo)*m + y1);
      I = (xhi-xlo)*(0.5*(xhi + xlo)*m + y1-m*x1);
      break;
    case 3: /* type 3: linear-log */
      /* y = m*ln(x) + y1 -m*ln(x1) */
      /* I = m*(x*ln(x)-x) + (y1 - m*ln(x1))*x @ dx */
      m = (y2-y1)/(log(x2)-log(x1));
      I = m*( (xhi*log(xhi)-xhi) - (xlo*log(xlo)-xlo) ) 
	+ ( y1-m*log(x1) ) * (xhi-xlo);
      break;
    case 4: /* type 4: log-linear */
      /* y = y1*exp( m*(x-x1)) */
      /* I = y1*exp( m*(x-x1))/m @ dx*/
      m = (log(y2)-log(y1))/(x2-x1);
      I = expm1( m*(xhi-x1) ) - expm1( m*(xlo-x1) );
      I *= y1/m;
      break;
    case 5:/* type 5: log-log */
      /* y = y1 * (x/x1)^m */
      /* I = y1 * (x/x1)^m*x/(m+1) @ dx for m != -1 */
      /*   = y1 * x1*ln(x)         @ dx for m == -1 */
      m = ( log(y2) - log(y1) )/( log(x2) - log(x1) );
      if (m != -1)
	I = y1*( pow(xhi/x1,m)*xhi - pow(xlo/x1,m)*xlo )/(m+1);
      else
	I = y1*x1*log(xhi/xlo);
      break;
    default:
      I = 0;
    }

  return I;
}




void GammaSrc::setData(int kza, int numSpec,
		       int *numDisc, int *numIntReg, int *nPts, 
		       int **intRegB, int **intRegT, 
		       float **discGammaE, float **discGammaI,
		       float **contX, float **contY)
{

  int specNum, gammaNum, gNum, pntNum, regNum;
  double *gammaMult = NULL;
  double Elo, Ehi, interpFrac;

  if (numSpec > 0)
    {
        gammaMult = new double[nGroups];
        for (gNum=0;gNum<nGroups;gNum++)
	  gammaMult[gNum] = 0.0;
      

      contactDose = 0.0;
      adjDose = 0.0;
      exposureDose = 0.0;
    
      for (specNum=0;specNum<numSpec;specNum++)
	{
	  /* foreach discrete gamma */
	  for (gammaNum=0;gammaNum<numDisc[specNum];gammaNum++)
	    {
	      /* determine energy bin */
   	        gNum = findGroup(discGammaE[specNum][gammaNum]);

	      switch (gammaType)
		{
		case GAMMASRC_RAW_SRC:
		case GAMMASRC_ADJOINT:
		  /* increment bin */
		  gammaMult[gNum] += discGammaI[specNum][gammaNum];
		  break;
		case GAMMASRC_CONTACT:
		  /* increment total contact dose */
		  /* if this is the first group, we are interpolating between the first data point
		     and (0,0) */
		  /* if this is the "last group" we may be extrapolating */
		  interpFrac = (discGammaE[specNum][gammaNum] - grpBnds[gNum])/
		    (grpBnds[gNum+1] - grpBnds[gNum]);
		  /* Note: FISPACT Contact dose formula calls for gamma source in units of MeV/kg.s */
		  /* Note: gammaAttenCoef is really point data for the upper bound of a given group */
		  /* Note: if this is the first group, we are interpolating between the first data point
		     and (0,0) */
		  if (gNum == 0)
		    contactDose += discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum]*1e-6 *
		      gammaAttenCoef[gNum] * interpFrac;
		  else
		    contactDose += discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum]*1e-6 *
		      (gammaAttenCoef[gNum-1]*(1.0 - interpFrac) + 
		       gammaAttenCoef[gNum] * interpFrac);
		  break;
                 case GAMMASRC_EXPOSURE:
		  interpFrac = (discGammaE[specNum][gammaNum] - grpBnds[gNum])/
		    (grpBnds[gNum+1] - grpBnds[gNum]);		   
		   //infinite cylinder
		  if (gNum == 0) 
		   exposureDose += discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		     M_PI*radius*radius/(4*distance)*gammaAbsAir[gNum] * interpFrac;  
		  else 
		    exposureDose += discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		     M_PI*radius*radius/(4*distance)*(gammaAbsAir[gNum-1]*(1.0 - interpFrac) + 
		      gammaAbsAir[gNum] * interpFrac); 
		  break;
                }
	    }
	  
	  /* CONTINUOUS ENERGY SPECTRUM */
	  regNum = 0;
	  pntNum = 0;
	  gNum = 0; 
          // nPts[specNum] indicates size of contX[specNum].
          // (pntNum + 1) must never exceed nPts[specNum] so that contX[specNum][pntNum+1] is valid.
         
	  //Only perform the following while loop when dealing with adjoint dose and photon source
          if (gammaType == GAMMASRC_ADJOINT || gammaType == GAMMASRC_RAW_SRC )
	  while ( regNum < numIntReg[specNum] && pntNum < (nPts[specNum]-1))
	    {
 	      Elo = std::max(double(contX[specNum][pntNum]),grpBnds[gNum]);		
	      Ehi = std::min(double(contX[specNum][pntNum+1]),grpBnds[gNum+1]);
	       
              switch (gammaType)
		{
                case GAMMASRC_RAW_SRC:
	        case GAMMASRC_ADJOINT:
                  	
		  gammaMult[gNum] += subIntegral(pntNum,intRegT[specNum][regNum],
						 contX[specNum],contY[specNum],
						 Elo,Ehi);
		  break;
		case GAMMASRC_CONTACT:
		  /* ignore continuous spectrum for now */
		  break;
		
		}

             if ( gammaType != GAMMASRC_EXPOSURE)
	      if (Ehi == grpBnds[gNum+1]) 
		gNum++;
	      else
		{
		  pntNum++;
		  if (pntNum > intRegB[specNum][regNum])
		    regNum++;
		}
	    }
	}

        gammaMultCache[kza] = gammaMult;
    }

}

double* GammaSrc::getGammaMult(int kza)
{
  if (!gammaMultCache.count(kza))
    {
      gammaMultCache[kza] = NULL;
      dataLib->readGammaData(kza,this);
    }

  return gammaMultCache[kza];

}

void GammaSrc::writeTotal(double *photonSrc,int nResults)
{
  int gNum, resNum;

  gSrcFile << "TOTAL" << endl;

  for (resNum=0;resNum<nResults;resNum++)
    {
      for (gNum=0;gNum<nGroups;gNum++)
	{
	  gSrcFile << photonSrc[resNum*nGroups+gNum];
	  if (gNum%6 == 5) 
	    gSrcFile << endl;
	  else
	    gSrcFile << "\t";
	}
      gSrcFile << endl;
    }
}

void GammaSrc::writeIsotope(double *photonSrc, double N)
{
  int gNum;

  if (photonSrc == NULL)
    return;

  gSrcFile << "\t"; 

  for (gNum=0;gNum<nGroups;gNum++)
    {
      gSrcFile << photonSrc[gNum]*N;
      if (gNum%6 == 5)
	gSrcFile << endl;
      gSrcFile << "\t";
    }
  
  gSrcFile << endl;
}

double GammaSrc::calcDoseConv(int kza, double *mixGammaAttenCoef)
{
  /* reset contact dose in case there is no gamma data for this isotope */
  contactDose = 0;

  gammaAttenCoef = mixGammaAttenCoef;

  dataLib->readGammaData(kza,this);

  gammaAttenCoef = NULL;

  return contactDose;


}

double GammaSrc::calcAdjDose(int kza, double *volAdjDoseConv, double vol)
{

  double adjDose = 0;
  double* mult = getGammaMult(kza);
  int gNum;

 // volAdjDoseConv group order is lo-hi and getGammaMult order is lo-hi 
  if (mult != NULL)
    {
      for (gNum=0;gNum<nGroups;gNum++)
	adjDose += mult[gNum]*volAdjDoseConv[gNum];
    }

  return adjDose*vol/detvolume;
  
}

double GammaSrc::calcExposureDoseConv(int kza)
{
  if ( exposureDoseCache.find(kza) == exposureDoseCache.end() ) {

    exposureDose = 0;

    dataLib->readGammaData(kza,this);

    exposureDoseCache[kza] = exposureDose;

  }

  return exposureDoseCache[kza];

};

