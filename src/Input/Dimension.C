/* $Id: Dimension.C,v 1.3 1999-08-24 22:06:21 wilson Exp $ */
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

#include "Dimension.h"

#include "Zone.h"
#include "Geometry.h"

/***************************
 ********* Service *********
 **************************/

Dimension::Dimension(int dimType) :
  type(dimType),  nInts(0), nZones(0), start(0)
{
  zoneListHead = new Zone(ZONE_HEAD);
  memCheck(zoneListHead,"Dimension::Dimension(...) constructor: zoneListHead");

  next = NULL;
}

Dimension::Dimension(const Dimension& d) :
  type(d.type),  nInts(d.nInts), nZones(d.nZones), start(d.start)
{
  zoneListHead = new Zone(ZONE_HEAD);
  memCheck(zoneListHead,"Dimension::Dimension(...) copy constructor: zoneListHead");

  next = NULL;
}

Dimension::~Dimension()
{
  delete next; 
  delete zoneListHead; 
}  

Dimension& Dimension::operator=(const Dimension& d)
{
  if (this == &d)
    return *this;

  type = d.type;
  nInts = d.nInts;
  nZones = d.nZones;
  start = d.start;
  
  delete zoneListHead;
  zoneListHead = new Zone(ZONE_HEAD);
  memCheck(zoneListHead,"Dimension::operator=(...): zoneListHead");


  return *this;

}

/****************************
 *********** Input **********
 ***************************/

/* read a single dimension */
/* called by Input::read(...) */
Dimension* Dimension::getDimension(istream& input)
{
  char token[64];
  int dimType;
  double zoneBound;

  input >> token;
  switch(tolower(token[0]))
    {
    case 'x':
      dimType = DIM_X;
      break;
    case 'y':
      dimType = DIM_Y;
      break;
    case 'z':
      dimType = DIM_Z;
      break;
    case 'r':
      dimType = DIM_R;
      break;
    case 't':
      dimType = DIM_THETA;
      break;
    case 'p':
      dimType = DIM_PHI;
      break;
    default:
      error(130,"Invalid dimension type: %s",token);
    }

  next = new Dimension(dimType);
  memCheck(next,"Dimension::getDimension(...): next");

  Dimension *dimPtr = next;

  /* read first zone boundary */
  input >> dimPtr->start;

  /* read list of zone definitions until keyword "end" */
  Zone* zoneList = dimPtr->zoneListHead;

  verbose(2,"Reading zone boundaries for Dimension %s:",token);
  verbose(3,"Start: %g",dimPtr->start);

  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      input >> zoneBound;
      zoneList = zoneList->addZone(atoi(token),zoneBound);

      clearComment(input);
      input >> token;
    }

  if (zoneList->head())
    warning(131,"Dimension has no boundaries");

  return dimPtr;         
  

}


/***************************
 ********* xCheck **********
 **************************/

/* cross-check dimensions: 
 *  ensure that there are no duplicates and 
 * that all dimenions are in context */
void Dimension::checkTypes(int geom_type)
{
  Dimension *ptr = this;
  int xCheck[7] = {0,0,0,0,0,0,0};
  int dim_min,dim_max,dim;
  int fail;
  
  /* used to fill unit dimensions */
  double dimUnit[7] = {0,1,1,1,1,2*PI,PI};

  /* for use in error output */
  char *dimTypeText[7] = {"","X","Y","Z","R","THETA","PHI"};
  char *geomTypeText[5] = {"","Rectangular","Cylindrical","Spherical","Toroidal"};

  switch (geom_type)
    {
    case GEOM_R:
      dim_min = DIM_X;
      dim_max = DIM_Z;
      break;
    case GEOM_C:
      dim_min = DIM_Z;
      dim_max = DIM_THETA;
      break;
    case GEOM_S:
    case GEOM_T:
      dim_min = DIM_R;
      dim_max = DIM_PHI;
      break;
    }


  verbose(2,"Checking that dimension types match geometry type: %s.",
	  geomTypeText[geom_type]);

  while (ptr->next != NULL)
    {
      ptr=ptr->next;
      
      /* check for duplicates */
      if (xCheck[ptr->type])
	error(330,"Duplicate dimensions of type %s.",dimTypeText[ptr->type]);
      xCheck[ptr->type] = 1;

      /* check context */
      fail = (ptr->type < dim_min || ptr->type > dim_max);
      if (fail)
	{
	  error(331,"%s geometries don't have dimensions of type %s.",
		  geomTypeText[geom_type],dimTypeText[ptr->type]);
	}
    }

  /* fill out dimensions with unit blocks in other directions */
  for (dim=dim_min;dim<=dim_max;dim++)
    if (!xCheck[dim])
      {
	ptr->next = new Dimension(dim);
	memCheck(ptr->next,"Dimension::checkTypes(...): ptr->next");
	ptr = ptr->next;
	verbose(3,"Setting dimension %s with unit size.",dimTypeText[dim]);
	ptr->zoneListHead->addZone(1,dimUnit[dim]);
      }
}

/****************************
 ********* Preproc **********
 ***************************/

/* convert to a basic data type:
 *   list of zone boundaries and numbers of intervals in each zone
 *      +  zone loading pattern
 *   = list of intervals with zone membership
 * this function sets up conversion and passes it to Zone::convert(...) */
/* called by Input::preproc(...) */
void Dimension::convert(Volume* volList, Loading *loadList, Geometry *geom)
{
  int numDims = 0;
  int coord[3];
  double d0[3];
  Dimension *ptr = this;
  Zone *zonePtr[3];

  verbose(2,"Converting zone boundaries into list of intervals.");

  /* for each dimensin */
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      debug(3,"Setting up dimension %d.",ptr->type);
      switch (ptr->type)
	{
        /* X or R are the 'first' dimension for the purposes
	 * of calculating invterval volumes */
	case DIM_X:
	case DIM_R:
	  coord[0] = numDims;
	  break;
        /* Y or THETA are the 'second' dimension for the purposes
	 * of calculating invterval volumes */
	case DIM_Y:
	case DIM_THETA:
	  coord[1] = numDims;
	  break;
        /* Z or PHI are the 'third' dimension for the purposes
	 * of calculating invterval volumes */
	case DIM_Z:
	case DIM_PHI:
	  coord[2] = numDims;
	  break;
	}
      /* save the first (zeroeth) zone boundary and other info */
      d0[numDims] = ptr->start;
      zonePtr[numDims++] = ptr->zoneListHead;
    }

  debug(3,"Calling Zone::convert with starting point (%g,%g,%g).",
	d0[0],d0[1],d0[2]);
  /* do the final conversion */
  Zone::convert(d0,coord,zonePtr,geom,loadList,volList);
  
  verbose(3,"Converted all zone boundaries into list of intervals.");
}

/****************************
 ********* Utility **********
 ***************************/

/* search for a dimension of given type */
Dimension* Dimension::find(int srchType)
{

  Dimension* ptr = this;
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->type == srchType)
	return ptr;
    }

  return NULL;
}


void Dimension::count()
{
  nZones = zoneListHead->numZones();
  nInts = zoneListHead->numInts();
}

/* count number of zones in all dimensions */
int Dimension::totZones()
{
  int numZones = 1;
  Dimension *ptr=this;
  
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
      if (ptr->nZones < 1)
	ptr->count();
      numZones *= ptr->nZones;
    }

  return numZones;
}
  



