#include "OutputFormat.h"

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

  next = NULL;
}

OutputFormat::OutputFormat(const OutputFormat& o) :
  resolution(o.resolution), outTypes(o.outTypes)
{

  next = NULL;
}
  
OutputFormat::~OutputFormat()
{
  delete next;
}

OutputFormat& OutputFormat::operator=(const OutputFormat& o)
{
  if (this == &o)
    return *this;

  resolution = o.resolution;
  outTypes = o.outTypes;

  return *this;
}

/***************************
 ********** Input **********
 **************************/

OutputFormat* OutputFormat::getOutFmts(istream& input)
{
  const char *Out_Res = " izm";
  const char *Out_Types = "censtabg";

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

      clearComment(input);
      input >> token;
    }	      

  return next;

}


void OutputFormat::write(Volume* volList, Mixture* mixList, Loading* loadList,
			 CoolingTime *coolList, int targetKza)
{
  
  const int nOutTypes = 8;
  const int firstResponse=2;
  const char *Out_Types_Str[nOutTypes] = {
  "Break-down by Component",
  "Binary Data export",
  "Number Density",
  "Specific Activity",
  "Total Decay Heat",
  "Alpha Decay Heat",
  "Beta Decay Heat",
  "Gamma Decay Heat"};

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
      for (outTypeNum=firstResponse;outTypeNum<nOutTypes;outTypeNum++)
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
    }

}
