/* $Id: GammaSrc.C,v 1.6 2000-11-29 21:29:47 wilson Exp $ */
#include "GammaSrc.h"

#include "DataLib/DataLib.h"

/***************************
 ********* Service *********
 **************************/

GammaSrc::GammaSrc(istream& input)
{
  
  char token[64];
  char libType[9];
  int gNum;

  strcpy(libType,"gammalib");
  nGroups = 0;
  grpBnds = NULL;
  dataLib = NULL;

  /* get gamma library filename */
  dataLib = DataLib::newLib(libType,input);

  /* get gamma source output filename */
  clearComment(input);
  input >> token;
  fileName = new char[strlen(token)+1];
  strcpy(fileName,token);
  gSrcFile.open(fileName);
  if (!gSrcFile)
    {
      /* error */
    }

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

GammaSrc::~GammaSrc()
{

  /* empty VectorCache */
  /* foreach entry in cache... */
  /* delete vector pointed to by entry */

  delete dataLib;
  delete grpBnds;

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
      I = (xhi-xlo)*(0.5*(xhi + xlo)*m + y1);
      break;
    case 3: /* type 3: linear-log */
      /* y = m*ln(x) + y1 -m*ln(x1) */
      /* I = m*(x*ln(x)-x) + (y1 - m*ln(x1))*x @ dx */
      m = (y2-y1)/(log(x2)-log(x1));
      I = m*( (xhi*log(xhi)-xhi) - (xlo*log(xlo)-xlo) ) 
	+ ( y1-m*log(x1) ) * (xhi-xlo);
      break;
    case 4: /* type 4: log-linesr */
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
  double Elo, Ehi;

  if (numSpec > 0)
    {
      gammaMult = new double[nGroups];
      
      for (gNum=0;gNum<nGroups;gNum++)
	gammaMult[gNum] = 0.0;
      
      for (specNum=0;specNum<numSpec;specNum++)
	{
	  /* foreach discrete gamma */
	  for (gammaNum=0;gammaNum<numDisc[specNum];gammaNum++)
	    {
	      /* determine energy bin */
	      gNum = findGroup(discGammaE[specNum][gammaNum]);
	      /* increment bin */
	      gammaMult[gNum] += discGammaI[specNum][gammaNum];
	    }
	  
	  /* CONTINUOUS ENERGY SPECTRUM */
	  regNum = 0;
	  pntNum = 0;
	  gNum = 0;
	  if (numIntReg[specNum] > 0 && nPts[specNum] > 0)
	    {
	      Elo = max(double(contX[specNum][pntNum]),grpBnds[gNum]);
	      Ehi = min(double(contX[specNum][pntNum+1]),grpBnds[gNum+1]);
	      gammaMult[gNum] += subIntegral(pntNum,intRegT[specNum][regNum],
					     contX[specNum],contY[specNum],
					     Elo,Ehi);
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
