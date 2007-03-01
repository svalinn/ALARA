/* $Id: math.C,v 1.17 2007-03-01 15:53:22 phruksar Exp $ */
#include "alara.h"
#include "Matrix.h"

/* this is used to determine how many terms are needed in the expansion */
#define MAXEXPTOL 1e-15

/* This is the maximum number of terms allowed for the expansion method
 * If more are needed, the inversion method is used */
#define MAXNUMEXPTERMS 15

/* This is used to determine if poles are degenerate.  Loops should
 * generate poles which are exactly the same, but poles with small
 * relative differences may act as degeneracies anyway */
#define SMALL_REL_DIFF 1e-8

/* routine for to calculate factorial */
double fact(int i)
{
  int idx,idx2;
  /* create a look-up table for factorials */
  const int maxFactorial = 50;
  static double *factorials = NULL;

  if (factorials == NULL)
    {
      factorials = new double[maxFactorial];
      for (idx=0;idx<maxFactorial;idx++)
	{
	  idx2 = idx;
	  factorials[idx] = 1;
	  while (idx2>1) factorials[idx] *= idx2--;
	}
    }

  if (i < maxFactorial)
    return factorials[i];
  else
    {
      double result=1;
      
      while (i>1) result *= i--;
      
      return result;
    }
}


double bateman(int row, int col, double* d, double t, int& finitePositive)
{
  //A short-term solution to a loop problem is to bypass a Bateman method.
  ////////
//   double sum_la = laplaceExpansion(row, col, d, t, finitePositive);
//   if (finitePositive)
//     return sum_la;
//   else
    return laplaceInverse(row, col, d, t, finitePositive);
  ////////

  double sum, sumInc, den;
  int term, denTerm;

  finitePositive = TRUE;

  sum = 0;
  sumInc = 0;

  for (term=col;term<row;term++)
    {
      /* set denominator element based on Laplace root: d[term]
       * (traditional analytic inverse Laplace transform) */
      den = 1;

      for (denTerm=col;denTerm<term;denTerm++)
	den *= (d[denTerm]-d[term]);

      for (denTerm++;denTerm<=row;denTerm++)
	den *= (d[denTerm]-d[term]);

      /* set numerator element based on Laplace root: d[term] */
      sumInc = expm1(-d[term]*t)-expm1(-d[row]*t);

      /* add element based on Laplace root: d[term] */
      sum += sumInc/den;
    }

  /* negative results are due to round-off error and 
   * imply very small results */
  if (sum < 0 || isnan(sum))
    {
      finitePositive = FALSE;
      return 0;
    }
  else
   return sum;

};


double dGn(int idx, double *pole, int *mult, int numPoles, int termNum)
{
  int pNum, pwr;
  double invPwrSum,result = 0;


  if (termNum==0)   /* End Condition - 0th derivative */
    /* return inverse product of pole-otherPoles */
    {
      result = 1;

      /* all poles before the current pole */
      for (pNum=0; pNum<idx; pNum++)
	result /= pow( pole[pNum] - pole[idx] , mult[pNum] );

      /* all poles after the current pole */
      for (pNum++; pNum<numPoles; pNum++)
	result /= pow( pole[pNum] - pole[idx] , mult[pNum] );
    }
  else
    for (pwr=termNum;pwr>0;pwr--)
      {
	invPwrSum = 0;

	/* all poles before the current pole */
	for (pNum=0; pNum<idx; pNum++)
	  invPwrSum += mult[pNum] * pow( pole[pNum]-pole[idx] , -pwr );

	/* all poles after the current pole */
	for (pNum++; pNum<numPoles; pNum++)
	  invPwrSum += mult[pNum] * pow( pole[pNum]-pole[idx] , -pwr );

	/* recursively add terms */
	result += -2*(pwr%2 -.5) * (fact(termNum-1)/fact(termNum-pwr)) 
	  * invPwrSum * dGn(idx,pole,mult,numPoles,termNum-pwr);
      }
  
  return result;

}

double laplaceInverse(int row, int col, double *d, double t, 
		      int& finitePositive)
{
  int idx, checkIdx, multCnt;
  int numPoles = 0;
  int *mult = new int[row-col+1];
  double *pole = new double[row-col+1];
  double poleResult, result  = 0;

  finitePositive = TRUE;

  /* index all the poles with the multiplicities */
  for (idx = col;idx<=row;idx++)
    {
      for (checkIdx=0;checkIdx<numPoles;checkIdx++)
	if ( fabs((d[idx]-pole[checkIdx]))<SMALL_REL_DIFF*d[idx] )
	  {
	    mult[checkIdx]++;
	    break;
	  }
      if (checkIdx == numPoles)
	{
	  pole[checkIdx] = d[idx];
	  mult[checkIdx] = 1;
	  numPoles++;
	}
    }

  /* perform recursive analytic Laplace inversion */
  for (idx=0;idx<numPoles;idx++)
    {
      poleResult = 0;

      for (multCnt=mult[idx];multCnt>0;multCnt--)
	poleResult += dGn(idx, pole, mult, numPoles, mult[idx] - multCnt)
	  * pow(t,multCnt-1)
	  / fact(multCnt-1) 
	  / fact(mult[idx]-multCnt) ;

      result += poleResult * exp(-pole[idx]*t);
    }

  delete mult;
  delete pole;

  if (result < 0 || isnan(result))
    {
      finitePositive = FALSE;
      return 0;
    } 
  else
    return result;

}


/* function to return a rough estimate of whether or not the expansion technique
   will converge quickly enough */
int smallExpansion(int row, int col, double *d, double t)
{
  int poleNum, n=MAXNUMEXPTERMS;
  int rank=row-col+1;
  double max=0;

  /* for each pole in the problem */
  for (poleNum=col;poleNum<=row;poleNum++)
    /* determine the largest pole */
    max = d[poleNum]>max?d[poleNum]:max;

  /* using a defined maximum number of terms, if the last term results
   * in a correction which is too large */
  if (rank*pow(max*t,n)*fact(rank-1)/(n*fact(n+rank-1)) > MAXEXPTOL)
    /* if too large, return false */
    return FALSE;

  return TRUE;
}




double laplaceExpansion(int row, int col, double *d, double t, int &converged)
{

  int idx, termNum;
  int sz = row-col;
  double result, correction;

  /* initialize matrix with destruction rates */
  Matrix poleMat(d,sz+1,col);
  Matrix powPoleMat(sz+1);

  /* innocent until proven guilty */
  converged = TRUE;

  /* zeroth term is simply the correct power of t/n!  */
  result = pow(t,sz)/fact(sz);

  /* for each successive term */
  for (termNum=1;termNum<MAXNUMEXPTERMS;termNum++)
    {
      /* multiply the power matrix by the non-power matrix */
      powPoleMat *= poleMat;

      /* power of t/n! times coefficient, with alternating sign!! */
      correction = powPoleMat.rowSum(sz) * ( 1-2*(termNum%2) )
	* pow(t,termNum+sz)/fact(termNum+sz);
      
      if (fabs(correction/result) > MAXEXPTOL)
	/* use this term if significant */
	result += correction;
      else
	/* otherwise break */
	break;
    }

  /* for monstrous decay rates (e.g. Be-8) the correction or
   * result may become infinite */
  if (termNum == MAXNUMEXPTERMS || isnan(result))
    converged = FALSE;
  
  return result;
}

double fillTElement(int row, int col, double *P, double *d, double t, 
		    int* loopRank, int rank)
{

  int idx,loopIdx,parLoopIdx;
  int defSuccess, altSuccess;
  double result;

  /* process loop information in reverse problem */
  if (rank != row)
    {
      loopIdx = loopRank[rank];
      parLoopIdx = loopRank[rank+1];
    }
  else
    {
      loopIdx = loopRank[row];
      parLoopIdx = loopRank[row-1];
    }

  /* if there is no loop, at this level, row - loopIdx = -1 */
  if (loopIdx == -1)
    loopIdx = row+1;

  /* do this product up front to eliminate costly
   * computation which may end in 0 anyway */
  double productionProduct = 1;
  for (idx=col;idx<row;idx++)
    productionProduct *= P[idx+1];

  if (productionProduct == 0)
    return productionProduct;

  /* implement default method */

  /* This seemingly complicated condition saves using the loop
   * solution during a reference calculation when only the last
   * isotope introduces the loop.  In this case, there is no real
   * degeneracy, since the destruction rate of the last isotope is
   * zero'ed for a reference calculation.  The condition can be
   * understood as follows: 
   *  - when checking for loop solution, if we are calculating the
   *    production from an isotope inside the loop, generally use
   *    the loop sol'n, but only iff
   *  - only the last isotope can ever have a destruction rate of
   *    zero
   *      - if d[row] > 0, use loop sol'n
   *  - even if last isotope does have a 0 destruction rate, if
   *    the previous isotope was already in a loop, we need to use
   *    the loop sol'n
   *      - if loopRank[row-1] > -1, check for loop sol'n \
   *        (This check must be done after the first one because
   *        it ensures that we are not checking loopRank[-1])*/
  if (col<=row-loopIdx && (d[row] > 0 || parLoopIdx >-1))
    {
      /* get rough estimate of success of expansion method */
      defSuccess = smallExpansion(row,col,d,t);
      
      /* if we think the expansion method is good, use it */
      if (defSuccess)
	result = laplaceExpansion(row,col,d,t,defSuccess);
      
      /* if either we think the expansion method is bad,
         or we prove that it is bad, use the inversion method */
      if (!defSuccess)
	result = laplaceInverse(row,col,d,t,altSuccess);
    }
  else
    result = bateman(row,col,d,t,altSuccess);

  /* used during debugging 
  if (isinf(result) || isnan(result) )
    error(2001,"No mathematical method was successful!");  */
     
  return result*productionProduct;
}

