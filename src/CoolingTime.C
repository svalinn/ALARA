/* $Id: CoolingTime.C,v 1.4 2003-01-13 04:34:54 fateneja Exp $ */
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
#include "Output_def.h"

/***************************
 ********* Service *********
 **************************/

/** Default constructor creates a blank list head with no arguments.
      Otherwise, it sets both the time and units and initializes the
      'next' pointer to NULL. */
CoolingTime::CoolingTime(double coolTime, char unts)
  : units(unts), coolingTime(coolTime)
{
  next = NULL;
}

/** Copy constructor copies scalar members and sets 'next' to NULL. */
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
/** It reads a list of times and units until it finds the keyword "end".
    It is called through the head of the cooling times list. */
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
	error(120,"Invalid units in cooling time: %10g %c",
	      ptr->coolingTime, ptr->units);

      verbose(3,"Added cooling time at: %g %c.",
	      ptr->coolingTime, ptr->units);

      clearComment(input);
      input >> token;
    }

  if (ptr->head())
    warning(121,"No after-shutdown/cooling times were defined.");


}

/****************************
 ********* Preproc **********
 ***************************/
/** Called through the head of the cooling times list, it
    allocates the correct number of doubles, assigning them to the pointer
    reference argument.  It returns the number of cooling times. */
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

/** There is a column for the isotope, a column for the @shutdown
    result, and then a column for each after-shutdown cooling time. */
void CoolingTime::writeHeader(int cooltime_units)
{
  CoolingTime *ptr = this;
  char textBuf[16];

  cout << "isotope  t_1/2(s)   pre-irrad   shutdown   ";

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (cooltime_units == COOLTIME_S) // print cooling time converted to seconds
      {
        double t_sec = convertTime(ptr->coolingTime, ptr->units);
        sprintf(textBuf, "%9.3e s ", t_sec);
      }
      else  // print cooling time in default units
      {
        sprintf(textBuf, "%7g %c   ", ptr->coolingTime, ptr->units);
      }
      
      cout << textBuf;
    }
  cout << endl;
  writeSeparator();
}

void CoolingTime::getCoolTimesStrings(std::vector<std::string>& coolTimesList)
{
  coolTimesList.push_back("shutdown");
  
  CoolingTime *ptr = this;
  char textBuf[16];

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      sprintf(textBuf,"%7g %c   ",ptr->coolingTime, ptr->units);
      coolTimesList.push_back(textBuf);
    }
  
  return;
}

/** There is a  column indicating the counter for the total in question,
    one column for @ shutdown results, and then one column for each of the
    after-shutdown cooling times. */
void CoolingTime::writeTotalHeader(const char* type, int cooltime_units)
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
      if (cooltime_units == COOLTIME_S) // print cooling time converted to seconds
      {
        double t_sec = convertTime(ptr->coolingTime, ptr->units);
        sprintf(textBuf, "%9.3e s ", t_sec);
      }
      else  // print cooling time in default units
      {
        sprintf(textBuf, "%7g %c   ", ptr->coolingTime, ptr->units);
      }
      
      cout << textBuf;
  }
  cout << endl;
  writeSeparator();
}

void CoolingTime::writeSeparator()
{

  CoolingTime *ptr = this;
  cout << "==========";

  /* shutdown */
  cout << "============";
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      cout << "================";
    }
  cout << endl;
}
