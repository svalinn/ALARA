/* $Id: Input.C,v 1.12 2000-01-18 02:37:35 wilson Exp $ */
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


#include "Input.h"
#include "CoolingTime.h"
#include "Dimension.h"
#include "Flux.h"
#include "Geometry.h"
#include "History.h"
#include "Loading.h"
#include "Mixture.h"
#include "Norm.h"
#include "Component.h"
#include "Schedule.h"
#include "Volume.h"

#include "Chains/NuclearData.h"
#include "Chains/Chain.h"
#include "Chains/Root.h"

#include "Calc/topSchedule.h"

#include "Util/input_tokens.h"

#include "Output/OutputFormat.h"
#include "Output/Result.h"

#include "DataLib/DataLib.h"

/***************************
 ********* Service *********
 **************************/


Input::Input(char* inputFname)
{
  if (inputFname != NULL)
    {
      debug(1,"Openning %s for input.",inputFname);
      input = openFile(inputFname);
      verbose(1,"Openned %s for input.",inputFname);
    }
  else
    {
      debug(1,"Openning stdin for input.");
      input = &cin;
      verbose(1,"Opened stdin for input.");
    }
      
  inGeom = NULL;

  mixListHead = new Mixture(IN_HEAD);
  memCheck(mixListHead,"Input::Input() constructor: mixListHead");
  mixList = mixListHead;

  fluxListHead = new Flux(FLUX_HEAD);
  memCheck(fluxListHead,"Input::Input() constructor: fluxListHead");
  fluxList = fluxListHead;

  historyListHead = new History(IN_HEAD);
  memCheck(historyListHead,"Input::Input() constructor: historyListHead");
  historyList = historyListHead;

  schedListHead = new Schedule(IN_HEAD);
  memCheck(schedListHead,"Input::Input() constructor: schedListHead");
  schedList = schedListHead;

  dimListHead = new Dimension(DIM_HEAD);
  memCheck(dimListHead,"Input::Input() constructor: dimListHead");
  dimList = dimListHead;

  coolList = new CoolingTime(COOL_HEAD);
  memCheck(coolList,"Input::Input() constructor: coolList");

  volList = new Volume(VOL_HEAD);
  memCheck(volList,"Input::Input() constructor: volList");

  normList = new Norm(NORM_HEAD);
  memCheck(normList,"Input::Input() constructor: normList");
  
  loadList = new Loading(IN_HEAD);
  memCheck(loadList,"Input::Input() constructor: loadList");
  
  solveList = new Loading(IN_HEAD);
  memCheck(loadList,"Input::Input() constructor: solveList");
  
  skipList = new Loading(IN_HEAD);
  memCheck(loadList,"Input::Input() constructor: skipList");
  
  outListHead = new OutputFormat(OUTRES_HEAD);
  memCheck(outListHead,"Input::Input() constructor: outListHead");

}

Input::~Input()
{
  delete inGeom;
  delete mixListHead;
  delete fluxListHead;
  delete historyListHead;
  delete schedListHead;
  delete dimListHead;
  delete coolList;
  delete volList;
  delete normList;
  delete loadList;
  delete solveList;
  delete skipList;
  delete outListHead;
}
  

/***************************
 ********** Input **********
 **************************/

/* read a token and start processing that token type block */
void Input::read()
{

  char token[64];
  OutputFormat* outList = outListHead;

  while (input != NULL)
    {
      while (!(*input).eof())
	{
	  token[0] = '\0';
	  clearIncludeComment();
	  *input >> token;

	  if (strlen(token)>0)
	    {
	      debug(1,"Token %s = %d",token,tokenType(token));
	      switch (tokenType(token))
		{
		case INTOK_GEOM:
		  *input >> token;
		  debug(1,"Creating new geometry object: %s.", token);
		  inGeom = new Geometry(token);
		  memCheck(inGeom,"Input::Input() constructor: inGeom");
		  break;
		case INTOK_MIX:
		  debug(1,"Creating new Mixture object.");
		  mixList = mixList->getMixture(*input);
		  break;
		case INTOK_FLUX:
		  debug(1,"Creating new Flux object.");
		  fluxList = fluxList->getFlux(*input);
		  break;
		case INTOK_PULSE:
		  debug(1,"Creating new History object.");
		  historyList = historyList->getHistory(*input);
		  break;
		case INTOK_SCHED:
		  debug(1,"Creating new Schedule object.");
		  schedList = schedList->getSchedule(*input);
		  break;
		case INTOK_DIM:
		  debug(1,"Creating new Dimension object.");
		  dimList = dimList->getDimension(*input);
		  break;
		case INTOK_MINR:
		  debug(1,"Reading Minor Radius.");
		  *input >> token;
		  inGeom->setMinorR(atof(token));
		  verbose(2,"Set torus minor radius = %g",atof(token));
		  break;
		case INTOK_MAJR:
		  debug(1,"Reading Major Radius.");
		  *input >> token;
		  inGeom->setMajorR(atof(token));
		  verbose(2,"Set torus major radius = %g",atof(token));
		  break;
		case INTOK_COOL:
		  debug(1,"Creating new CoolingTime object.");
		  coolList->getCoolingTimes(*input);
		  break;
		case INTOK_MAT:
		  debug(1,"Creating new Loading object.");
		  loadList->getMatLoading(*input);
		  break;
		case INTOK_VOL:
		  debug(1,"Reading Volume List.");
		  volList->getVolumes(*input);
		  break;
		case INTOK_MATLIB:
		  debug(1,"Opening material library");
		  Component::getMatLib(*input);
		  break;
		case INTOK_ELELIB:
		  debug(1,"Opening element library");
		  Component::getEleLib(*input);
		  break;
		case INTOK_DATALIB:
		  debug(1,"Opening data library");
		  NuclearData::getDataLib(*input);
		  break;
		case INTOK_LIBCONV:
		  debug(1,"Converting data library");
		  DataLib::convertLib(*input);
		  verbose(1,"Exiting after library conversion.");
		  exit(0);
		case INTOK_TRUNC:
		  debug(1,"Reading Truncation criteria.");
		  Chain::getTruncInfo(*input);
		  break;
		case INTOK_IMPURITY:
		  debug(1,"Reading Impurity definition and truncation criteria.");
		  Chain::getImpTruncInfo(*input);
		  break;
		case INTOK_NORM:
		  debug(1,"Reading interval normalizations.");
		  normList->getNorms(*input);
		  break;
		case INTOK_OUTPUT:
		  debug(1,"Reading output formatting.");
		  outList = outList->getOutFmts(*input);
		  break;
		case INTOK_DUMPFILE:
		  debug(1,"Openning dump filename.");
		  *input >> token;
		  Result::initBinDump(token);
		  break;
		case INTOK_SOLVELIST:
		  solveList->getSolveList(*input);
		  break;
		case INTOK_SKIPLIST:
		  skipList->getSolveList(*input);
		  break;
		default:
		  error(100,"Invalid token in input file: %s",token);
		}

	    } 
	}
      
      /* delete current stream */
      if (input != &cin)
	{
	  debug(1,"Deleting current stream.");
	  delete input;
	}
      
      debug(1,"Popping next stream.");
      /* end of this input stream */
      streamStack >> input;
      
    }

  debug(0,"Finished reading problem input.");

}

/***************************
 ********* xCheck **********
 **************************/

/* cross-check all input before pre-processing */
void Input::xCheck()
{

  /* check that the dump file has been created */
  Result::xCheck();

  /* check mixture definitions:
   * - warning if a mixture has no components ** DONE during input phase **
   * - if component type "SIM" exists, 
   *   make sure that it points to an existing mixture */
  mixListHead->xCheck();

  /* x-check material loading:
   * - warning if empty ** DONE during input phase **
   * - make sure all referenced mixtures are defined
   * - if using zone dimensions, 
   *   do the number of zones match ** DONE below ** */
  loadList->xCheck(mixListHead,solveList,skipList);
  

  /* if not 0-D problem */
  if (inGeom->getType() != GEOM_P)
    {
      /* if using zone dimensions */
      if (!dimList->head())
	{
	  /* interval volumes not allowed */
	  if (!volList->head())
	    error(300,"Cannot define both zone dimensions and interval volumes.");

	  /* check that we have the correct dimension types for this geometry */
	  dimListHead->checkTypes(inGeom->getType());

	  /* check for torus radii */
	  if (inGeom->getType() == GEOM_T)
	    inGeom->checkTorus(dimListHead);
	  
	  /* check that the number of zones defined here 
	   * matches the number in the material loading */
	  if (dimListHead->totZones() < loadList->numZones())
	    warning(301,"A material loading is given for more zones (%d) than are defined by the zone dimensions (%d).\n\
Those extra zones are being ignored.",loadList->numZones(),dimListHead->totZones());
	  else if (dimListHead->totZones() > loadList->numZones())
	    error(302,"Material loadings were not defined for as many zones (%d) as were defined by the zone dimensions (%d).",
		  loadList->numZones(),dimListHead->totZones());
	  else
	    verbose(2,"Number of zones defined by zone dimensions (%d) matches number of material loadings defined.(%d)",
		  dimListHead->totZones(),loadList->numZones());
	}
      else if (!volList->head())
	{
	  /* ensure that all zones used in the volume list are defined in the material loading list */
	  volList->xCheck(loadList);
	}
      /* reqiure either zone dimensions or interval volumes */
      else 
	error(303,"Must define either zone dimensions or interval volumes for multi-point problems.");
    }

  /* cross-check schedule list
   * - for an empty schedule ** DONE during input phase **
   * - for all items of type sub-schedule, make sure named schedule exists
   * - for all items of type pulse, make sure named flux exists
   * - for all items, make sure named pulse history exists */
  schedListHead->xCheck(fluxListHead,historyListHead);

  /* write the schedule hierarchy for the user to check */
  schedListHead->write();

}

/***************************
 ********* Preproc *********
 **************************/

void Input::preProc(Root*& rootList, topSchedule*& top)
{

  /* remove unused mixture definitions */
  mixListHead->removeUnused(loadList);

  /* convert zone dimensions to interval list,
   * automatically cross-referencing with zones */
  if (!dimListHead->head())
    dimListHead->convert(volList,loadList,inGeom);

  /* cross-reference intervals with mixtures */
  volList->xRef(mixListHead);

  /* cross-reference normalization with intervals */
  if (!normList->head())
    volList->xRef(normList);

  /* fill volList with flux data */
  fluxListHead->xRef(volList);

  /* make a root list */
  mixListHead->makeRootList(rootList);

  /* make histories */
  historyListHead->makeHistories();

  /* make schedules */
  top = schedListHead->makeSchedules(coolList);
  verbose(2,"Collapsing schedules from top.");
  top->collapse();

  /* dump header to binary dump */
  Result::dumpHeader();

  /* make schedule T storage */
  volList->makeSchedTs(top);

}


/***************************
 ********* Postproc ********
 **************************/

void Input::postProc(Root *masterRootList)
{

  switch(NuclearData::getMode())
    {
    case MODE_FORWARD:
      {
	masterRootList->readDump();
	volList->postProc();
	outListHead->write(volList,mixListHead,loadList,coolList);
	break;
      }
    case MODE_REVERSE:
      {
	Root *target = masterRootList;
	int targetKza;
	char isoSym[16];

	while (target != NULL)
	  {
	    target = target->readSingleDump(targetKza);
	    volList->postProc();
	    cout << endl << "****** TARGET ****** " 
		 << isoName(targetKza,isoSym) << " ****** TARGET ****** " 
		 << isoSym << " ****** TARGET ****** " << endl << endl;
	    outListHead->write(volList,mixListHead,loadList,coolList,targetKza);
	    volList->resetOutList();
	    mixListHead->resetOutList();
	    loadList->resetOutList();

	  }
	break;
      }
	      
    }
  

}
/***************************
 ********* Utility *********
 **************************/

/* function to clear all blank lines and comment lines 
 * from the input FILE stream */
void Input::clearIncludeComment()
{
  /* look at the next character in the stream */
  char charInput = input->peek();

  while (charInput != 'A')
    switch (charInput)
      {
      case '#' : /* comment or INCLUDE*/
	char buffer[MAXLINELENGTH];

	input->getline(buffer,MAXLINELENGTH,'\n');

	/* if we are including another input file here */
	if (!strncmp(buffer,"#include",8))
	  {
	    char  *inFileName, *endFileName;
	    int findEnd = 1;

	    /* extract name of new input file */
	    inFileName = buffer+8;
	    while (findEnd)
	      switch (inFileName[0])
		{
		case '\'':
		case '"':
		  endFileName = strchr(inFileName+1,inFileName[0]);
		  *endFileName = '\0';
		  findEnd = 0;
		case ' ':
		  inFileName++;
		  break;
		}
	    
	    /* push this stream onto the stack */
	    streamStack << input;


	    verbose(2,"Openning included file: %s.",inFileName);
	    /* open new stream */
	    input = openFile(inFileName);

	    if (*input == 0)
	      error(101,"Unable to open included file: '%s'.",inFileName);

	    verbose(2,"Reading included file: %s.",inFileName);

	  }

	charInput = input->peek();
	break;
      case ' ' : /* whitespace */
      case '\n': /* whitespace */
      case '\t': /* whitespace */
	input->get();
	charInput = input->peek();
	break;
      default:
	charInput = 'A';
      }
      
}

