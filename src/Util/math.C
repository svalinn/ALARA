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
  double result=1;

  while (i>1) result *= i--;

  return result;
}


double bateman(int row, int col, double* d, double t)
{
  double sum, sumInc, den;
  int term, denTerm;

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

  /* multiply normalization by sum of Laplace inverse terms */
  /* absolute value here takes care of negative results */
  /******* THIS IS AN INCORRECT STOP-GAP **********/
  return fabs(sum);

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

double laplaceInverse(int row, int col, double *d, double t)
{
  int idx, checkIdx, multCnt;
  int numPoles = 0;
  int *mult = new int[row-col+1];
  double *pole = new double[row-col+1];
  double poleResult, result;

  if (row == col)
    /* if this is on the diagonal, 
     * simple exponential decay */
    return exp(-d[col]*t);
  
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

  delete pole;

  /* absolute value here takes care of negative results */
  /******* THIS IS AN INCORRECT STOP-GAP **********/
  return fabs(result);

}

double fillTElement(int row, int col, double *P, double *d, double t, 
		    int* loopRank)
{

  int idx, termNum;
  int sz = row-col;
  double result, correction;

  /* do this product up front to eliminate costly
   * computation which may end in 0 anyway */
  double productionProduct = 1;
  for (idx=col;idx<row;idx++)
    productionProduct *= P[idx+1];

  if (productionProduct == 0)
    return productionProduct;

  /******* DEFAULT TO LAPLACE EXPANSION TO AVOID ROUND-OFF *********/
  /* initialize matrix with destruction rates */
  Matrix poleMat(d,sz+1);
  Matrix powPoleMat(sz+1);

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

  /******* IF WE NEED TOO MANY TERMS *****
   ******* GO TO ALTERNATIVE METHOD  *****
   ******* BASED ON LOOP CONDITION   *****/
  if (termNum == MAXNUMEXPTERMS)
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
    if (col<=loopRank[row] && (d[row] > 0 || loopRank[row-1] >-1))
      result = laplaceInverse(row,col,d,t);
    else
      result = bateman(row,col,d,t);
    
  return result*productionProduct;
}

