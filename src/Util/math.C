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


double bateman(int row, int col, double* P, double* d, double t)
{
  if (row == col)
    /* if this is on the diagonal, 
     * simple exponential decay */
    return exp(-d[col]*t);
  
  if (d[col] == 0) 
    /* if the isotope at this rank=col has no destruction, 
     * the whole column is empty ! (except the diagonal) 
     * Note: this is only possible when calling from a decay matrix sol'n
     */
      return 0;

  double result, sum, sumInc, den;
  int term, denTerm;

  result = 1;
  sum = 0;
  sumInc = 0;

  for (term=col;term<row;term++)
    {
      /* multiplicative increment of normalization */
      result *= P[term+1];
      
      if (result == 0)
	/* if there is no production path between the isotopes at
	 * row and col, this element is empty
	 * Note: this is only possible when calling from a decay matrix sol'n
	 */
	return 0;

      
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
  result *= sum;

  return result;

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

  return result;

}

double laplaceExpansion(int row, int col, double *d, double t, int numTerms)
{

  int sz = row-col;
  double result;

  if (row == col)
    /* if this is on the diagonal, 
     * simple exponential decay */
    return exp(-d[col]*t);
  

  /* initialize matrix with destruction rates */
  Matrix poleMat = Matrix::Triangle(d,sz+1);
  Matrix powPoleMat = Matrix::Identity(sz+1);

  /* zeroth term is simply the correct power of t/n!  */
  result = pow(t,sz)/fact(sz);

  /* for each successive term */
  for (int termNum=1;termNum<numTerms;termNum++)
    {
      /* multiply the power matrix by the non-power matrix */
      powPoleMat *= poleMat;

      /* power of t/n! times coefficient, with alternating sign!! */
      result += powPoleMat.rowSum(sz) * ( 1-2*(termNum%2) )
              * pow(t,termNum+sz)/fact(termNum+sz);
    }

  return result;
}

int small(double *d, double t, int rank)
{
  double max=0;

  /* for each pole in the problem */
  for (int poleNum=0;poleNum<rank;poleNum++)
    /* determine the larges pole */
    max = d[poleNum]>max?d[poleNum]:max;

  int n=MAXNUMEXPTERMS;
  /* using a defined maximum number of terms, if the last term results
   * in a correction which is too large */
  if (pow(rank*max*t,n)*fact(rank-1)/(n*fact(n+rank-1)) > MAXEXPTOL)
    /* return the maximum number of terms + 1 
     * to be used to indicate that a series expansion will not work */
    return MAXNUMEXPTERMS+1;

  /* arrive at a VERY rough approximation for number of terms by splitting
   * guess in half */
  /* for each guess at the number of terms */
  for (;n>0;n/=2)
    {
      /* if the correction is too large */
      if (pow(rank*max*t,n)*fact(rank-1)/(n*fact(n+rank-1))>MAXEXPTOL)
	/* return the previous guess */
	return 2*n;
    }

  /* return the last guess (must be 1 ... VERY unlikely ) */
  return n;
}
  

double loopSoln(int row, int col, double *P, double *d, double t)
{

  double result = 1;
  int idx;

  for (idx=col;idx<row;idx++)
    result *= P[idx+1];

  int numExpansionTerms = small(d,t,row-col+1);

  if (numExpansionTerms>MAXNUMEXPTERMS)
    result *= laplaceInverse(row, col, d, t);
  else
    result *= laplaceExpansion(row, col, d, t, numExpansionTerms);

  return result;



}
