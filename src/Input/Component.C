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

#include "Chains/Root.h"

ifstream Component::matLib;
ifstream Component::eleLib;

/***************************
 ********* Service *********
 **************************/
Component::Component(int compType, char *name, double dens) :
  type(compType),density(dens)
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
  type(comp.type), density(comp.density)
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
Component* Component::getComponent(int setType,istream &input)
{

  char name[64];
  double dens=0;

  input >> name;
  if (setType <= COMP_SIM)
    input >> dens;

  next = new Component(setType,name,dens);
  memCheck(next,"Component::getComponent(...) : next");

  verbose(3,"type code: %d name: %s   density %g",setType,name,dens);

  return next;

}

void Component::getMatLib(istream& input)
{
  char fname[256];
  input >> fname;
  matLib.open(fname,ios::in);

  if (matLib == 0)
    error(150,"Unable to open material library: %s",fname);

  verbose(2,"Openned material library %s",fname);
}

void Component::getEleLib(istream& input)
{
  char fname[256];
  input >> fname;
  eleLib.open(fname,ios::in);

  if (eleLib == 0)
    error(151,"Unable to open element library: %s",fname);

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
  double scale = density;

  /* change this component */
  *ptr = *newComp;
  ptr->density *= scale;

  /* insert all subsequent components from this mixture */
  while (newComp->next != NULL)
    {
      newComp = newComp->next;
      ptr->next = new Component(*newComp);
      memCheck(ptr->next,"Component::replaceSim(...) : ptr->next");
      ptr = ptr->next;
      ptr->density *= scale;
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
	case COMP_ISO:
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
  int numIsos, Z;
  ifstream elelib;
  char testName[64],isoName[64];
  double isoDens, eleDens, A;

  /* rewind the element library */
  eleLib.seekg(0L,ios::beg);

  verbose(4,"Expanding element %s",compName);

  /* search for this element */
  eleLib >> testName >> A >> Z >> eleDens >> numIsos;
  while (strcmp(testName,compName) && !eleLib.eof())
    {
      verbose(5,"Skipping element %s in element library",testName);
      while (numIsos-->0)
	eleLib >> isoName >> isoDens;
      eleLib >> testName >> A >> Z >> eleDens >> numIsos;
    }
      
  if (!eleLib.eof())
    {
      density *= AVAGADRO/A;

      /* if element is found, add a new root for each isotope */
      verbose(5,"Found element %s in element library",testName);
      while (numIsos-->0)
	{
	  eleLib >> isoName >> isoDens;
	  isoDens *= density;
	  strcpy(testName,compName);
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
    error(150,"Could not find element %s in element library.",compName);

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
  matLib >> testName >> matDens >> numEles;
  while (strcmp(testName,compName) && !matLib.eof())
    {
      verbose(5,"Skipping material %s in material library.",testName);
      while (numEles-->0)
	matLib >> eleName >> eleDens >> eleZ;
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
	  matLib >> eleName >> eleDens  >> eleZ;
	  eleDens *= density/100.0;
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
    error(151,"Could not find material %s in material library.",
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

