/* $Id: OutputFormat.C,v 1.15 1999-08-27 18:47:35 wilson Exp $ */
#include "OutputFormat.h"

#include "Input/CoolingTime.h"
#include "Input/Volume.h"
#include "Input/Mixture.h"
#include "Input/Loading.h"

#include "Chains/Node.h"

#define Bq2Ci 2.7027e-11

const char *Out_Types = "ucnstabgw";

const int nOutTypes = 9;
const int firstResponse = 2;
const int lastSingularResponse = 8;
const char *Out_Types_Str[nOutTypes] = {
  "Response Units",
  "Break-down by Component",
  "Number Density [atoms/cm3]",
  "Specific Activity [%s/cm3]",
  "Total Decay Heat [W/cm3]",
  "Alpha Decay Heat [W/cm3]",
  "Beta Decay Heat [W/cm3]",
  "Gamma Decay Heat [W/cm3]",
  "WDR/Clearance index"};

/***************************
 ********* Service *********
 **************************/

OutputFormat::OutputFormat(int type)
{
  resolution = type;
  outTypes = 0;
  actUnits = new char[3];
  strcpy(actUnits,"Bq");
  actMult = 1;

  normUnits = new char[4];
  strcpy(normUnits,"cm3");

  next = NULL;
}

OutputFormat::OutputFormat(const OutputFormat& o) :
  resolution(o.resolution), outTypes(o.outTypes), actMult(o.actMult)
{
  actUnits = new char[strlen(o.actUnits)+1];
  strcpy(actUnits,o.actUnits);

  normUnits = new char[strlen(o.normUnits)+1];
  strcpy(normUnits,o.normUnits);

  next = NULL;
}
  
OutputFormat::~OutputFormat()
{
  delete actUnits;
  delete normUnits;
  delete next;
}

OutputFormat& OutputFormat::operator=(const OutputFormat& o)
{
  if (this == &o)
    return *this;

  resolution = o.resolution;
  outTypes = o.outTypes;
  delete actUnits;
  actUnits = new char[strlen(o.actUnits)+1];
  strcpy(actUnits,o.actUnits);
  actMult = o.actMult;

  delete normUnits;
  normUnits = new char[strlen(o.normUnits)+1];
  strcpy(normUnits,o.normUnits);
  

  return *this;
}

/***************************
 ********** Input **********
 **************************/

OutputFormat* OutputFormat::getOutFmts(istream& input)
{
  const char *Out_Res = " izm";

  int type;
  char token[64];

  input >> token;
  type = strchr(Out_Res,tolower(token[0]))-Out_Res;

  next = new OutputFormat(type);

  verbose(2,"Added output at resolution %d (%s)",type,token);
  
  /* read a list of output types until keyword "end" */
  clearComment(input);
  input >> token;
  while (strcmp(token,"end"))
    {
      /* match the first character of the type in the constant string */
      type = strchr(Out_Types,tolower(token[0]))-Out_Types;
      if (type<0)
	error(230,"Output type '%s' is not currently supported.",
	      token);

      /* use logical and to set the correct bit in the outTypes field */
      next->outTypes |= 1<<type;

      verbose(3,"Added output type %d (%s)",1<<type,token);

      switch (1<<type)
	{
	case OUTFMT_UNITS:
	  input >> token;
	  next->actUnits = new char[strlen(token)+1];
	  strcpy(next->actUnits,token);
	  next->actMult = (tolower(token[0]) == 'c'?Bq2Ci:1);

	  input >> token;
	  next->normUnits = new char[strlen(token)+1];
	  strcpy(next->normUnits,token);

	  break;
	case OUTFMT_WDR:
	  input >> token;
	  next->wdrFilenames.insert(token);
	  verbose(4,"Added WDR/Clearance file %s", token);
	}


      clearComment(input);
      input >> token;
    }	      

  return next;

}


void OutputFormat::write(Volume* volList, Mixture* mixList, Loading* loadList,
			 CoolingTime *coolList, int targetKza)
{
  

  OutputFormat *ptr = this;
  char buffer[64];

  int outTypeNum;

  /* for each output description */
  while (ptr->next != NULL)
    {
      
      ptr = ptr->next;

      /* write a header */
      switch(ptr->resolution)
	{
	case OUTRES_INT:
	  cout << "Interval output requested:"<< endl;
	  break;
	case OUTRES_ZONE:
	  cout << "Zone output requested:"<< endl;
	  break;
	case OUTRES_MIX:
	  cout << "Mixture output requested:"<< endl;
	  break;
	}
      
      /* list the reponses and features to come */
      /* units */
      outTypeNum = 0;
      cout << "\t" << Out_Types_Str[outTypeNum] << ": "
	   << ptr->actUnits << " " << ptr->normUnits << endl;
      /* regular singular responses */
      for (++outTypeNum;outTypeNum<lastSingularResponse;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	    sprintf(buffer,Out_Types_Str[outTypeNum],
		    ptr->actUnits,ptr->normUnits);
	    cout << "\t" << buffer << endl;
	  }
      /* WDR header */
      if (ptr->outTypes & OUTFMT_WDR)
	for(filenameList::iterator fileName = ptr->wdrFilenames.begin();
	    fileName != ptr->wdrFilenames.end(); ++fileName)
	  cout << "\t" << Out_Types_Str[outTypeNum] << ": " 
	       << *fileName << endl;
	      
	

      cout << endl << endl;
      
      /* set units for activity */
      Result::setActMult(ptr->actMult);

      /* for each indicated response */
      for (outTypeNum=firstResponse;outTypeNum<lastSingularResponse;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	    /* write a response title */
	    sprintf(buffer,Out_Types_Str[outTypeNum],ptr->actUnits);
	    cout << "*** " << buffer << " ***" << endl;

	    /* call write() on the appropriate object determined by
               the resulotition */
	    switch(ptr->resolution)
	      {
	      case OUTRES_INT:
		volList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
			       coolList,targetKza);
		break;
	      case OUTRES_ZONE:
		loadList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
				coolList,targetKza);
		break;
	      case OUTRES_MIX:
		mixList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
			       coolList,targetKza);
		break;
	      }

	    cout << endl << endl << endl;
	  }

      if (ptr->outTypes & OUTFMT_WDR)
	{
	  cout << "*** WDR ***" << endl;
	  for(filenameList::iterator fileName = ptr->wdrFilenames.begin();
	      fileName != ptr->wdrFilenames.end(); ++fileName)
	    {
	      
	      /* write a response title */
	      cout << "*** " << Out_Types_Str[outTypeNum] << ": " 
		   << *fileName << " ***" << endl;
	      
	      Node::loadWDR(*fileName);
	      
	      /* call write() on the appropriate object determined by
		 the resulotition */
	      switch(ptr->resolution)
		{
		case OUTRES_INT:
		  volList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				 coolList,targetKza);
		  break;
		case OUTRES_ZONE:
		  loadList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				  coolList,targetKza);
		  break;
		case OUTRES_MIX:
		  mixList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				 coolList,targetKza);
		  break;
		}
	      
	      cout << endl << endl << endl;
	    }
	  
	} 
    }

}
