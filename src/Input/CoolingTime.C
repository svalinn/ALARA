/* File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */

#include "CoolingTime.h"
#include "Input_def.h"

/***************************
 ********* Service *********
 **************************/

CoolingTime::CoolingTime(double coolTime, char unts)
  : units(unts), coolingTime(coolTime)
{
  next = NULL;
}

CoolingTime::CoolingTime(const CoolingTime& c)
  : units(c.units), coolingTime(c.coolingTime)
{
  next = NULL;
}

/****************************
 *********** Input **********
 ***************************/

/**** get a list of cooling times *******/
/* called by Input::read(...) */
void CoolingTime::getCoolingTimes(istream& input)
{
  char token[32], inUnits;
  CoolingTime *ptr = this;

  verbose(2,"Reading after-shutdown cooling times.");
  /* read list of cooling times until keyword end */
  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      input >> inUnits;
      ptr->next = new CoolingTime(atof(token),inUnits);
      memCheck(next,"CoolingTime::getCoolingTimes(...): next");

      ptr = ptr->next;

      /* check for valid units */
      if (strchr(UNITS,inUnits) == NULL)
	error(144,"Invalid units in cooling time: %10g %c",
	      ptr->coolingTime, ptr->units);

      verbose(3,"Added cooling time at: %g %c.",
	      ptr->coolingTime, ptr->units);

      clearComment(input);
      input >> token;
    }

  if (ptr->head())
    warning(112,"No after-shutdown/cooling times were defined.");


}

/****************************
 ********* Preproc **********
 ***************************/


/* count the cooling times and assign them to an array */
int CoolingTime::makeCoolingTimes(double *& coolingTimes)
{
  
  CoolingTime *head = this;
  CoolingTime *ptr = head;
  
  int nCoolingTimes = 0, coolNum=0;
  coolingTimes = NULL;

  verbose(2,"Processing cooling times.");

  /* count the cooling times */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      nCoolingTimes++;
    }

  verbose(3,"Found %d cooling times.",nCoolingTimes);
  
  if (nCoolingTimes>0) 
    { 
      /* make the array */
      coolingTimes = new double[nCoolingTimes];
      memCheck(coolingTimes,"CoolingTime::makeCoolingTimes(...): coolingTimes");
      
      /* assign each of the cooling times */
      ptr = head;
      while (ptr->next != NULL)
	{
	  ptr = ptr->next;
	  coolingTimes[coolNum++] = convertTime(ptr->coolingTime,ptr->units);
	  verbose(3,"Added cooling time %g %c = %g = %g.",
		  ptr->coolingTime,ptr->units, 
		  convertTime(ptr->coolingTime,ptr->units),
		  coolingTimes[coolNum-1]);
	}
    }

  return nCoolingTimes;

}

/****************************
 ******** Postproc **********
 ***************************/

void CoolingTime::writeHeader()
{
  CoolingTime *ptr = this;
  char textBuf[16];

  cout << "isotope\t shutdown   ";

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      sprintf(textBuf,"%7g %c   ",ptr->coolingTime, ptr->units);
      cout << textBuf;
    }
  cout << endl;
  writeSeparator();
}

void CoolingTime::writeTotalHeader(char* type)
{
  CoolingTime *ptr = this;
  char textBuf[16];

  cout << type;
  if (strlen(type)<8)
    cout << "\t";
  cout << " shutdown   ";

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      sprintf(textBuf,"%7g %c   ",ptr->coolingTime, ptr->units);
      cout << textBuf;
    }
  cout << endl;
  writeSeparator();
}

void CoolingTime::writeSeparator()
{

  CoolingTime *ptr = this;
  cout << "========";

  /* shutdown */
  cout << "============";
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      cout << "============";
    }
  cout << endl;
}
