/* File sections:
 * Service: constructors, destructors
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 * List: maintenance of lists or arrays of objects
 */



#include "ResultList.h"

#include "Input/Mixture.h"

#include "Chains/Chain.h"

#include "Calc/topScheduleT.h"


ResultList& ResultList::operator=(const ResultList& r)
{ 
  if (this == &r) 
    return *this; 

  /*** BEGIN copied from base class operator ***/

  int resNum;

  kza = r.kza;
  delete N;
  N = new double[nResults];
  for (resNum=0;resNum<nResults;resNum++)
    N[resNum] = r.N[resNum];

  /*** END copied from base class operator ***/

  return *this;
}

      


/****************************
 ********** Tally ***********
 ***************************/

ResultList* ResultList::tally(Chain *chain, topScheduleT *schedT)
{
  ResultList* ptr = this;
  
  /* get the KZA of the root */
  int rootKza = chain->getRoot();

  /* check whether this result list goes with this root */
  if (ptr->kza != rootKza)
    {
      ptr->nextList = new ResultList(rootKza);
      ptr = ptr->nextList;
    }

  /* get the rank of the first node to be tallied */
  int rank = chain->getSetRank();

  /* get the first kza to be set */
  int setKza = chain->getKza(rank);

  /* if we are not done (finish condition: kza == 0) */
  while (setKza > 0)
    {      
      /* tally result */
      double *Nlist = schedT->results(rank);

      ptr->find(setKza)->tally(Nlist);

      /* this is allocated in topScheduleT::results() */
      delete Nlist;

      /* get next isotope */
      setKza = chain->getKza(++rank);
    }

  return ptr;

}


/*****************************
 ********* PostProc **********
 ****************************/

void ResultList::postProcList(Result* outputList, Mixture *mixPtr)
{
  ResultList *ptr = this;
  Component *compPtr=NULL;
  double density;
  int compNum;
  
  /* for each root isotope */
  while (ptr->nextList != NULL)
    {
      ptr = ptr->nextList;
      
      /* get the first component number and density for this root
       * isotope */
      compPtr = mixPtr->getComp(ptr->kza,density,NULL);

      /* if we found the component */
      while (compPtr)
	{
	  compNum = mixPtr->getCompNum(compPtr);

	  /* update this component */
	  ptr->postProc(outputList[compNum],density);

	  /* get the next component */
	  compPtr = mixPtr->getComp(ptr->kza,density,compPtr);
	}

    }
}
