/* $Id: Component.C,v 1.15 2000-04-28 15:31:56 wilson Exp $ */
/* (Potential) File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */


#include "Component.h"
#include "Mixture.h"

#include "Chains/Root.h"

ifstream Component::matLib;
ifstream Component::eleLib;

/***************************
 ********* Service *********
 **************************/
Component::Component(int compType, char *name, double dens, double volFrac) :
  type(compType),density(dens), volFraction(volFrac)
{
  compName = NULL;
  if (name != NULL)
    {
      compName = new char[strlen(name)+1];
      memCheck(compName,"Component::Component(...) constructor: compName");
      strcpy(compName,name);
    }

  next = NULL;
}

Component::Component(const Component& comp) :
  type(comp.type), density(comp.density), volFraction(comp.volFraction)
{ 
  compName = NULL;
  if (comp.compName != NULL)
    {
      compName = new char[strlen(comp.compName)+1];
      memCheck(compName,"Component::Component(...) copy constructor: compName");
      strcpy(compName,comp.compName);
    }

  next = NULL;

}

Component& Component::operator=(const Component& comp)
{ 
  if (this == &comp)
    return *this;

  type = comp.type;
  density = comp.density;
  volFraction = comp.volFraction;

  delete compName;
  compName = NULL;
  if (comp.compName != NULL)
    {
      compName = new char[strlen(comp.compName)+1];
      memCheck(compName,"Component::operator(...) constructor: compName");
      strcpy(compName,comp.compName);
    }


  return *this;

}


/****************************
 *********** Input **********
 ***************************/

/* get individual components and add them to list */
/* called by Mixture::getMixture() */
Component* Component::getComponent(int setType,istream &input, Mixture *mixPtr)
{

  char name[64];
  double dens=0, volFrac=0;

  input >> name;
  if (setType < COMP_SIM)
    input >> dens >> volFrac;
  if (setType == COMP_SIM)
    input >> volFrac;

  next = new Component(setType,name,dens,volFrac);
  memCheck(next,"Component::getComponent(...) : next");

  verbose(3,"type code: %d name: %s, density %g, volume fraction: %g",
	  setType,name,dens,volFrac);

  mixPtr->incrVolFrac(volFrac);

  return next;

}

void Component::getMatLib(istream& input)
{
  char fname[256];
  input >> fname;
  matLib.open(fname,ios::in);

  if (matLib == 0)
    error(110,"Unable to open material library: %s",fname);

  verbose(2,"Openned material library %s",fname);
}

void Component::getEleLib(istream& input)
{
  char fname[256];
  input >> fname;
  eleLib.open(fname,ios::in);

  if (eleLib == 0)
    error(111,"Unable to open element library: %s",fname);

  verbose(2,"Openned element library %s",fname);
}

/****************************
 ********* Preproc **********
 ***************************/

/* replace a component of type 'l' with its expanded reference */
/* called by Mixture::copySim() */
Component* Component::replaceSim(Component *newCompList)
{
  Component *ptr = this;
  Component *saveNext = ptr->next;
  Component *newComp = newCompList->next;
  double scale = volFraction;

  /* change this component */
  *ptr = *newComp;
  ptr->volFraction *= scale;

  /* insert all subsequent components from this mixture */
  while (newComp->next != NULL)
    {
      newComp = newComp->next;
      ptr->next = new Component(*newComp);
      memCheck(ptr->next,"Component::replaceSim(...) : ptr->next");
      ptr = ptr->next;
      ptr->volFraction *= scale;
    }

  ptr->next = saveNext;

  return ptr;
  
}

/* expand all the components of a mixture into a root list */
/* called by Mixture::makeRootList(...) */
Root* Component::expand(Mixture *mix)
{
  Component* ptr = this;
  Root* rootList = new Root;
  Root* compRootList;
  memCheck(rootList,"Component::expand(...) : rootList");

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      
      switch (ptr->type)
	{
	case COMP_MAT:
	  compRootList = ptr->expandMat(mix);
	  rootList = rootList->merge(compRootList);
	  verbose(6,"Merged material %s into rootList for mixture",
		  ptr->compName);
	  delete compRootList;
	  break;
	case COMP_ELE:
	case TARGET_ELE:
	  compRootList = ptr->expandEle(mix,ptr);
	  rootList = rootList->merge(compRootList);
	  verbose(6,"Merged element %s into rootList for mixture",
		  ptr->compName);
	  delete compRootList;
	  break;
//	case COMP_ISO:
	case TARGET_ISO:
	  Root* newRoot = new Root(ptr->compName,ptr->density,mix,ptr);
	  memCheck(newRoot,"Component::expand(...) : newRoot");
	  rootList = rootList->merge(newRoot);
	  verbose(6,"Merged isotope %s into rootList for mixture",
		  ptr->compName);
	  delete newRoot;
	  break;
	}
    }
  return rootList;

}

/* expand a mixture component of type element */
/* called by Component::expand(...) */
Root* Component::expandEle(Mixture* mix, Component* comp)
{
  Root *rootList = new Root;
  memCheck(rootList,"Component::expandEle(...) : rootList");
  int numIsos, Z, eleNameLen;
  ifstream elelib;
  char testName[64],isoName[64];
  double isoDens, eleDens, A;

  /* rewind the element library */
  eleLib.seekg(0L,ios::beg);

  verbose(4,"Expanding element %s",compName);

  /* Extract the standard element name from the potentially
   * fabricated name.  e.g. enriched Li might be li:90 */
  eleNameLen = strchr(compName,':')-compName;
  if (eleNameLen <= 0) eleNameLen = strlen(compName);

  /* search for this element */
  clearComment(eleLib);
  eleLib >> testName >> A >> Z >> eleDens >> numIsos;
  while (strcmp(testName,compName) && !eleLib.eof())
    {
      verbose(5,"Skipping element %s in element library",testName);
      while (numIsos-->0)
	{
	  clearComment(eleLib);
	  eleLib >> isoName >> isoDens;
	}
      clearComment(eleLib);
      eleLib >> testName >> A >> Z >> eleDens >> numIsos;
    }
      
  if (!eleLib.eof())
    {
      if (density >= 0)
	density *= eleDens;
      else
	density = -density;

      double Ndensity = volFraction * density * AVAGADRO/A;
      mix->incrTotalDensity(density*volFraction);

      /* if element is found, add a new root for each isotope */
      verbose(5,"Found element %s with %d isotopes in element library",
	      testName, numIsos);

      while (numIsos-->0)
	{
	  clearComment(eleLib);
	  eleLib >> isoName >> isoDens;
	  isoDens *= Ndensity/100.0;
	  strncpy(testName,compName,eleNameLen);
	  testName[eleNameLen] = '\0';
	  strcat(testName,"-");
	  strcat(testName,isoName);
	  Root* newRoot = new Root(testName,isoDens,mix,comp);
	  memCheck(newRoot,"Component::expandEle(...) : newRoot");
	  rootList = rootList->merge(newRoot);
	  verbose(6,"Merged isotope %s into rootList for element %s",
		  testName,compName);
	  debug(5,"Accounted for isotope %s in Root List",testName);
	  delete newRoot;
	}
    }
  else
    error(310,"Could not find element %s in element library.",compName);

  return rootList;

}

/* expand a mixture component of type material */
/* called by Component::expand(...) */
Root* Component::expandMat(Mixture* mix)
{
  Root *rootList = new Root;
  memCheck(rootList,"Component::expandMat(...) : rootList");
  Component *element;
  int numEles, eleZ;
  ifstream matlib;
  char testName[64],eleName[64];
  double eleDens, matDens;

  /* rewind the material library */
  matLib.seekg(0L,ios::beg);

  verbose(4,"Expanding material %s",compName);

  /* search for this material */
  clearComment(matLib);
  matLib >> testName >> matDens >> numEles;
  while (strcmp(testName,compName) && !matLib.eof())
    {
      verbose(5,"Skipping material %s in material library.",testName);
      while (numEles-->0)
	{
	  clearComment(matLib);
	  matLib >> eleName >> eleDens >> eleZ;
	}
      clearComment(matLib);
      matLib >> testName >> matDens >> numEles;
    }

  if (!matLib.eof())
    {
      /* scale relative density by material density from lib */
      density *= matDens;

      verbose(5,"Found material %s in material library.",testName);
      /* if material found, read list of elements,
       * supplementing the root list for each one */
      while (numEles-->0)
	{
	  clearComment(matLib);
	  matLib >> eleName >> eleDens  >> eleZ;
	  eleDens *= -density*volFraction/100.0;
	  element = new Component(COMP_ELE,eleName,eleDens);
	  memCheck(element,"Component::expandMat(...) : element");
	  Root *elementRootList = element->expandEle(mix,this);
	  rootList = rootList->merge(elementRootList);
	  delete elementRootList;
	  verbose(6,"Merged element %s into rootList for material %s",
		  eleName,compName);
	  delete element;
	}
    }
  else
    error(311,"Could not find material %s in material library.",
	  compName);
  
  return rootList;
}

/****************************
 ********* Utility **********
 ***************************/

int Component::getCompNum(Component *compPtr)
{
  int compNum = -1;
  Component *ptr = this;

  while (ptr != NULL & ptr != compPtr)
    {
      ptr = ptr->next;
      compNum++;
    }

  return compNum;
}

/* search sequentially for a particular type of coponent */
Component* Component::exists(int srchType)
{
 
  Component* ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->type == srchType)
	return ptr;
    }

  /* return the NULL to denote the end of this component list */
  return ptr->next;
}


  
