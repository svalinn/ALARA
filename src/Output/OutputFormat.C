#include "OutputFormat.h"

#include "DoseResponse.h"
#include "GammaSrc.h"

#include "Input/CoolingTime.h"
#include "Input/Volume.h"
#include "Input/Mixture.h"
#include "Input/Loading.h"

/***************************
 ********* Service *********
 **************************/

OutputFormat::OutputFormat(int type)
{
  resolution = type;
  outTypes = 0;

  dose = new DoseResponse;
  gSrcName =  NULL;
  gSrcFName = NULL;

  next = NULL;
}

OutputFormat::OutputFormat(const OutputFormat& o) :
  resolution(o.resolution), outTypes(o.outTypes)
{
  dose = new DoseResponse(*(o.dose));

  gSrcName =  NULL;
  if (o.gSrcName != NULL)
    {
      gSrcName = new char[strlen(o.gSrcName)+1];
      strcpy(gSrcName,o.gSrcName);
    }

  gSrcFName = NULL;
  if (o.gSrcFName != NULL)
    {
      gSrcFName = new char[strlen(o.gSrcFName)+1];
      strcpy(gSrcFName,o.gSrcFName);
    }

  next = NULL;
}
  
OutputFormat::~OutputFormat()
{
  delete dose;
  delete next;
}

OutputFormat& OutputFormat::operator=(const OutputFormat& o)
{
  if (this == &o)
    return *this;

  resolution = o.resolution;
  outTypes = o.outTypes;
  delete dose;
  delete gSrcName;
  delete gSrcFName;
  dose = new DoseResponse(*(o.dose));
  
  gSrcName =  NULL;
  if (o.gSrcName != NULL)
    {
      gSrcName = new char[strlen(o.gSrcName)+1];
      strcpy(gSrcName,o.gSrcName);
    }

  gSrcFName = NULL;
  if (o.gSrcFName != NULL)
    {
      gSrcFName = new char[strlen(o.gSrcFName)+1];
      strcpy(gSrcFName,o.gSrcFName);
    }

  return *this;
}

/***************************
 ********** Input **********
 **************************/

OutputFormat* OutputFormat::getOutFmts(istream& input)
{
  const char *Out_Res = " izm";
  const char *Out_Types = "cnstabgdp";

  int type;
  char token[64];
  char tmpGSrcName[64], tmpAdjFluxName[64];

  input >> token;
  type = strchr(Out_Res,tolower(token[0]))-Out_Res;

  next = new OutputFormat(type);
  DoseResponse *dosePtr = next->dose;

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

      if (1<<type == OUTFMT_DOSE)
	{
	  clearComment(input);
	  input >> token >> tmpAdjFluxName >> tmpGSrcName;
	  dosePtr = dosePtr->add(token,tmpAdjFluxName,tmpGSrcName);
	  verbose(4," -- dose response %s using flux %s and gamma source structure %s",
		  token,tmpAdjFluxName,tmpGSrcName);
	  
	}
      else if (1<<type == OUTFMT_PHOTON)
	{
	  clearComment(input);
	  input >> token;
	  next->gSrcName = new char[strlen(token)+1];
	  strcpy(next->gSrcName,token);

	  clearComment(input);
	  input >> token;
	  next->gSrcFName = new char[strlen(token)+1];
	  strcpy(next->gSrcFName,token);

	  verbose(4," -- gamma source output to file %s using gamma source structure %s",
		  next->gSrcFName,next->gSrcName);
	  
	}

      clearComment(input);
      input >> token;
    }	      

  return next;

}


/***************************
 ********** xCheck *********
 **************************/

void OutputFormat::xCheck(GammaSrc *gSrcList, Flux *fluxList)
{
  OutputFormat *ptr = this;

  int outTypeNum;

  verbose(2,"Checking for adjoint fluxes and gamma source structures referenced in output formats.");

  /* for each output description */
  while (ptr->next != NULL)
    {
      
      ptr = ptr->next;

      if (ptr->outTypes & OUTFMT_DOSE)
	ptr->dose->xCheck(gSrcList,fluxList);
      if (ptr->outTypes & OUTFMT_PHOTON)
	{
	  verbose(3,"Checking for gamma source structure %s for photon source output.",ptr->gSrcName);
	  /* set pointer to gamma source */
	  GammaSrc *tmpGSrc = gSrcList->find(ptr->gSrcName);
	  if (tmpGSrc == NULL)
	    error(9999,"Gamma source structure %s for this response does not exist.",
		  ptr->gSrcName);
	  else
	    {
	      verbose(4,"Gamma source structure %s was found.",ptr->gSrcName);
	      delete gSrcName;
	      ptr->gSrc = tmpGSrc;
	    }
	}
      
    }

}

/***************************
 ******** Post-Proc ********
 **************************/

void OutputFormat::write(Volume* volList, Mixture* mixList, Loading* loadList,
			 CoolingTime *coolList, int targetKza)
{
  
  const int nOutTypes = 9;
  const int firstResponse=1;
  const int lastStdResponse=7;
  const char *Out_Types_Str[nOutTypes] = {
  "Break-down by Component",
  "Number Density",
  "Specific Activity",
  "Total Decay Heat",
  "Alpha Decay Heat",
  "Beta Decay Heat",
  "Gamma Decay Heat",
  "Dose Response",
  "Photon Source"};

  OutputFormat *ptr = this;

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
      for (outTypeNum=0;outTypeNum<nOutTypes;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  cout << "\t" << Out_Types_Str[outTypeNum] << endl;

      cout << endl << endl;
      
      /* for each indicated response */
      for (outTypeNum=firstResponse;outTypeNum<lastStdResponse;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	    /* write a response title */
	    cout << "*** " << Out_Types_Str[outTypeNum] << " ***" << endl;

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

      for (;outTypeNum<nOutTypes;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	  }

    }

}
