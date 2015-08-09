/* $Id: GammaSrc.C,v 1.28 2009-02-23 15:17:44 wilsonp Exp $ */
#include "GammaSrc.h"

#include "DataLib/DataLib.h"
#include "Input/Mixture.h"
#include "Input/Volume.h"
#include <math.h>
using namespace std;
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
    case GAMMASRC_EXPOSURE_CYLINDRICAL_VOLUME:
      initExposureCylVolDose(input);
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
 clearComment(gDoseData);
 for (gNum=0;gNum<nGroups;gNum++)
    gDoseData >> gammaAbsAir[gNum];
      
 
  //Radius and distance are needed for infinite line approximation
  input >> radius;
  input >> distance;

}

void GammaSrc::initExposureCylVolDose(istream& input)
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
 clearComment(gDoseData);
 for (gNum=0;gNum<nGroups;gNum++)
    gDoseData >> gammaAbsAir[gNum];
      
 
  //Radius and distance are needed for infinite line approximation
  input >> token;
  media = tolower(token[0]);

  input >> radius;
  input >> height;
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

  /* read gamma energy absorption coefficient for air */
 gammaAbsAir.resize(nGroups,0);
 clearComment(gDoseData);
 for (gNum=0;gNum<nGroups;gNum++)
    gDoseData >> gammaAbsAir[gNum];

}

void GammaSrc::initAdjointDose(istream& input)
{

  char token[64];
  int gNum;

  /* read detector volume */
  clearComment(input);
  input >> detvolume;
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

float GammaSrc::G_factor(float k,float p,float MsR,float /*b1 = 0*/)
{

  ifstream GFile;
  GFile.open(searchNonXSPath("GFunction"));
  if (!GFile)
    error(250,"Unable to open file for interpolating G function: %s\n",
	  "GFunction");

  //float k_min, k_max, p_min, p_max;
  int readint;
  float readflt;

  //Read k values from file
  vector<float> k_val;
  GFile >> readint;
  for (int i=0; i<readint; i++)
  {
    GFile >> readflt;
    k_val.push_back(readflt);
  }
  
  vector<float> p_val;
  GFile >> readint;
  for (int i=0; i < readint; i++)
  {
    GFile >> readflt;
    p_val.push_back(readflt);
  }


  //Make sure that all parameters are within ranges
  if ( ( k < k_val[0] ) || ( k > k_val[k_val.size()-1] ) )
    error(1104, "The ratio height/radius is out of range for cylindrical volume source calculation.\n");
  else if ( (p < p_val[0] ) || (p > p_val[p_val.size()-1] ) )
    error(1105, "The ratio distance/radius is out of range for cylindrical volume source calculation.\n");


  vector<float> MsR_val;
  GFile >> readint;
  for (int i=0; i < readint; i++)
  {
    GFile >> readflt;
    MsR_val.push_back(readflt);
  }
  
  //index is MsR
  map< float, vector< vector<float> > > MsRTable;
  //Store k and p values;


 //Create map entries for MsRTable[2];
  vector< vector<float> > kpMatrix(4, vector<float>(5,0));

  for (unsigned int numMsR=0; numMsR<MsR_val.size(); numMsR++)
  {
    for (unsigned int i=0; i<k_val.size(); i++)
      for (unsigned int j=0;j<p_val.size(); j++)
	GFile >> kpMatrix[i][j];

  
    MsRTable[MsR_val[numMsR]] = kpMatrix;

  }

  //Extrapolate if MsR is greater than limit
 if ( MsR > MsR_val[MsR_val.size()-1] )
  {
    vector< vector<float> > kpM1,kpM2;
    kpM2 = MsRTable[MsR_val.size()-1];
    kpM1 = MsRTable[MsR_val.size()-2];

    for (unsigned int i=0; i<k_val.size(); i++)
      for (unsigned int j=0; j<p_val.size(); j++)
	kpMatrix[i][j] = ( kpM2[i][j] - kpM1[i][j] )/(MsR_val[MsR_val.size()-1] -MsR_val[MsR_val.size()-2] )*(MsR-MsR_val[MsR_val.size()-1]) + kpM2[i][j];

  }

  else 
  {
    //Interpolate MsRTable based on MsR
    map< float, vector< vector<float> > >::iterator currPtr;  

    for (currPtr = MsRTable.begin(); currPtr != MsRTable.end(); currPtr++)
      if (MsR <= currPtr->first)
	break;


    if (MsR == currPtr->first)
      //no need to interpolate MsR    
      kpMatrix = currPtr->second;
    else
      {
	vector< vector<float> > kpM1,kpM2;
	float h_MsR, l_MsR;

	kpM2 = currPtr->second; h_MsR = currPtr->first;
	currPtr--;
	kpM1 = currPtr->second; l_MsR = currPtr->first;
   
	for (unsigned int i=0; i<k_val.size(); i++)
	  for (unsigned int j=0; j<p_val.size(); j++)
	    kpMatrix[i][j] = (kpM2[i][j] - kpM1[i][j])/(h_MsR - l_MsR)*(MsR-l_MsR) + kpM1[i][j];
      }
  }
  //Interpolate kpMatrix based on k (row) to determine p vectors
  vector<float> p_vec(p_val.size(),0);

  unsigned int k_idx;
  for (k_idx=0; k_idx < k_val.size(); k_idx++)
    if ( k <= k_val[k_idx] )
      break;

  if ( k == k_val[k_idx] )
    p_vec = kpMatrix[k_idx];
  else
  {
    vector<float> p1,p2;
    float lo_k,hi_k;

    p1 = kpMatrix[k_idx-1]; lo_k = k_val[k_idx-1];
    p2 = kpMatrix[k_idx];   hi_k = k_val[k_idx];

    for (unsigned int j=1; j<p1.size(); j++)
      p_vec[j] = (p2[j] - p1[j])/(hi_k - lo_k)*(k - lo_k) + p1[j];

  }

  //Interpolate p_vec based on p to finally obtain G Factor
  double G_Factor;
  unsigned int p_idx;

  for (p_idx = 0; p_idx < p_val.size(); p_idx++)
    if (p <= p_val[p_idx])
      break;

  if ( p == p_val[p_idx])
    G_Factor = p_vec[p_idx];
  else
  {
    float lo_p, hi_p, lo_G, hi_G;
    hi_p = p_val[p_idx]; hi_G = p_vec[p_idx];
    lo_p = p_val[p_idx-1]; lo_G = p_vec[p_idx-1];

    G_Factor = (hi_G - lo_G)/(hi_p - lo_p)*(p - lo_p) + lo_G;
    
  }    

  return G_Factor;
  
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
  const double doseConvConst = 5.76e-10;
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
                  if(false)
		      gammaMult[gNum] += discGammaI[specNum][gammaNum];
                  else{
		      gammaMult[gNum] += discGammaI[specNum][gammaNum]*discGammaE[specNum][gammaNum]/
                                         (0.5*(grpBnds[gNum+1] + grpBnds[gNum]));
                  }
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
		    contactDose += doseConvConst*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum]*1e-6 *
		       gammaAbsAir[gNum]/gammaAttenCoef[gNum]* interpFrac;
		  else
		    contactDose += doseConvConst*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum]*1e-6 *
		      ( gammaAbsAir[gNum-1]/gammaAttenCoef[gNum-1]*(1.0 - interpFrac) + 
		        gammaAbsAir[gNum]/gammaAttenCoef[gNum] * interpFrac);
		  break;
                 case GAMMASRC_EXPOSURE:
		  interpFrac = (discGammaE[specNum][gammaNum] - grpBnds[gNum])/
		    (grpBnds[gNum+1] - grpBnds[gNum]);		   
		   //infinite line
		  //////
		  if (gNum == 0) 
		   exposureDose += 0.0659*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		     M_PI*radius*radius/(4*distance)*gammaAbsAir[gNum] * interpFrac;
		  else 
		    exposureDose += 0.0659*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		     M_PI*radius*radius/(4*distance)*( gammaAbsAir[gNum-1]*(1.0 - interpFrac) + 
		     gammaAbsAir[gNum] * interpFrac ); 
		  break;
		case GAMMASRC_EXPOSURE_CYLINDRICAL_VOLUME:
		  //		  exposureDose = 0;
                  // Need to work on G_factor function see the literature page 399 Eq. 6.6-22b
		 
		  interpFrac = (discGammaE[specNum][gammaNum] - grpBnds[gNum])/
		    (grpBnds[gNum+1] - grpBnds[gNum]);


		  if (gNum == 0)
		  {
		    double A1,A2,alpha1,alpha2;
		    //Determine Buildup Factor using Taylor's Method
		    calcBuildupParameters(grpBnds[gNum], media, A1,A2,alpha1,alpha2);
		    
		    // buildup flux = A1*uncollided_flux_1 + A2*uncollided_flux_2;
		    double MuR = gammaAttenCoef[gNum]*interpFrac*radius;

        	    exposureDose += 0.0659*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		     radius/(2*M_PI)*gammaAbsAir[gNum] * interpFrac*
		     (A1*G_factor(height/radius,distance/radius,MuR*(1+alpha1),0) + A2*G_factor(height/radius,distance/radius,MuR*(1+alpha2),0));  
		  }
		  else
		   {
                    double A1,A2,alpha1,alpha2;
		    calcBuildupParameters((grpBnds[gNum+1]+grpBnds[gNum])/2,media,A1,A2,alpha1,alpha2); 

		    double MuR = (gammaAttenCoef[gNum]*interpFrac+gammaAttenCoef[gNum-1]*(1.0-interpFrac))*radius;
		    exposureDose += 0.0659*discGammaI[specNum][gammaNum] * discGammaE[specNum][gammaNum] *1e-6 *
		    radius/(2*M_PI)*(gammaAbsAir[gNum-1]*(1.0 - interpFrac) + gammaAbsAir[gNum] * interpFrac)*
		    (A1*G_factor(height/radius,distance/radius,MuR*(1+alpha1),0) + A2*G_factor(height/radius,distance/radius,MuR*(1+alpha2),0)); 

		   }
		  //////
                
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
	     {
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

void GammaSrc::writeTotal(double *photonSrc,int nResults,std::vector<std::string> coolTimesList)
{
  int gNum, resNum;

  for (resNum=0;resNum<nResults;resNum++)
    {
      gSrcFile << "TOTAL" << "\t" << coolTimesList[resNum];

      for (gNum=0;gNum<nGroups;gNum++)
	{
	  gSrcFile << "\t" << photonSrc[resNum*nGroups+gNum];
	}
      gSrcFile << endl;
    }
}

void GammaSrc::writeIsotope(double *photonSrc, double N)
{
  int gNum;

  for (gNum=0;gNum<nGroups;gNum++)
    {
      gSrcFile << "\t" << (NULL == photonSrc ? 0 : photonSrc[gNum]*N);
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

double GammaSrc::calcExposureDoseConv(int kza, double *mixGammaAttenCoef)
{
  if ( exposureDoseCache.find(kza) == exposureDoseCache.end() ) {

    exposureDose = 0;

    gammaAttenCoef = mixGammaAttenCoef;

    dataLib->readGammaData(kza,this);

    gammaAttenCoef = NULL;

    exposureDoseCache[kza] = exposureDose;

  }

  return exposureDoseCache[kza];

}

void GammaSrc::calcBuildupParameters(double En,  char mat, double& A1, double& A2, double& alpha1, double& alpha2)
{
  //Convert En to MeV
  En = En/1e6;

  ifstream BFile;
  BFile.open(searchNonXSPath("Buildup_Parameter"));
  if (!BFile)
    error(250,"Unable to open file for interpolating buildup factors: %s\n",
	  "Buildup_Parameter");

  vector<double> E_vec;

  int readint; double readflt;
  char token[64]; 

  BFile >> readint;
  
  for (int i=0;i < readint; i++)
  {
    BFile >> readflt;
    E_vec.push_back(readflt);
  }
  
  map<char,int> material_map;
  BFile >> readint;
  for (int i=0;i < readint; i++)
  {
    int temp;

    BFile >> token; BFile >> temp;
    material_map[tolower(token[0])] = temp;

  }
 
  //Read in a correct parameter for a given "mat".
  map<char,int>::iterator mapPtr;
  int n_skip = -1;

  for (mapPtr = material_map.begin(); mapPtr != material_map.end(); mapPtr++)
  {
    if ( mat == mapPtr->first )
      n_skip = mapPtr->second;

  }

  if ( n_skip == -1)
  {
      cout << "Exposure buildup parameters for a specified material are unavailable\n";
      exit(0);
  }
    
  //Skip entries to intended set of material
  for (unsigned int i=0;i < (n_skip*E_vec.size()*3);i++)
    BFile >> readflt;

  
  map<double, vector<double> > params;
  for (unsigned int i=0; i<E_vec.size(); i++)
  {
    vector<double> param_vec;
    
    //Read A
    BFile >> readflt;
    param_vec.push_back(readflt);

    //Read -alpha_1
    BFile >> readflt;
    param_vec.push_back(-1*readflt);

    //Read alpha_2
    BFile >> readflt;
    param_vec.push_back(readflt);

    params[E_vec[i]] = param_vec;
  }
   
  BFile.close();

  //Start doing interpolation
  vector<double> param_vec(3,0);

  //Extrapolate if En < Emin or En > Emax
 if ( En < E_vec[0] )
  {
    vector< double > p_vec1,p_vec2;
    p_vec1 = params[E_vec[0]];
    p_vec2 = params[E_vec[1]];

    for (int i=0; i<3; i++)
	param_vec[i] = (p_vec2[i] - p_vec1[i])/(E_vec[1] - E_vec[0])*(En-E_vec[0]) + p_vec1[i];

  }
 else if (En > E_vec[E_vec.size()-1])
   {
     vector< double > p_vec1,p_vec2;
     p_vec1 = params[E_vec[E_vec.size()-2]];
     p_vec2 = params[E_vec[E_vec.size()-1]];

     for (int i=0; i<3; i++)
	param_vec[i] = (p_vec2[i] - p_vec1[i])/(E_vec[E_vec.size()-1] - E_vec[E_vec.size()-2])*(En-E_vec[E_vec.size()-1]) + p_vec2[i];
   }

 else
   {
      
      map<double, vector<double> >::iterator paramsPtr;

      for (paramsPtr = params.begin(); paramsPtr != params.end(); paramsPtr++)
	if ( En <= paramsPtr->first )
	  break;

      if (En == paramsPtr->first)
	param_vec = paramsPtr->second;
      else
	{
	  vector< double > p_vec1,p_vec2;
	  double hi_E = paramsPtr->first;
	  p_vec2 = paramsPtr->second;

	  paramsPtr--;
	  double lo_E = paramsPtr->first;
	  p_vec1 = paramsPtr->second;

	  for (int i=0; i<3; i++)
	    param_vec[i] = (p_vec2[i] - p_vec1[i])/(hi_E - lo_E)*(En-lo_E) + p_vec1[i];
	  

	}
   }

 A1 = param_vec[0];
 A2 = 1 - A1;
 alpha1 = param_vec[1];
 alpha2 = param_vec[2];

}
