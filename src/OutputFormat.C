/* $Id: OutputFormat.C,v 1.37 2009-02-23 15:20:26 wilsonp Exp $ */
#include "OutputFormat.h"

#include "GammaSrc.h"

#include "CoolingTime.h"
#include "Volume.h"
#include "Mixture.h"
#include "Loading.h"

#include "Node.h"

const char *Out_Types = "ucnstabgpdflvwi";

const int nOutTypes = 14;
const int firstResponse = 2;
const int lastSingularResponse = 13;
const char *Out_Types_Str[nOutTypes] = {
  "Response Units",
  "Break-down by Constituent",
  "Number Density [atoms%s]",
  "Specific Activity [%s%s]",
  "Total Decay Heat [W%s]",
  "Alpha Decay Heat [W%s]",
  "Beta Decay Heat [W%s]",
  "Gamma Decay Heat [W%s]",
  "Photon Source Distribution [gammas/s%s] : %s\n\t    with Specific Activity [%s%s]",
  "Contact Dose [ Sv/hr] : %s",
  "Folded (Adjoint/Biological) Dose [units defined by adjoint flux] : %s",
  "Exposure Rate [mR/hr] - Infinite Line Approximation: %s",
  "Exposure Rate [mR/hr] - Cylindrical Volume Source: %s",
  "WDR/Clearance index"};

/***************************
 ********* Service *********
 **************************/

/** When called without arguments, the default constructor creates a
    blank list head with no problem data.  Otherwise, it sets the
    'resolution' to the first argument and initializes the 'next'
    pointer to NULL. */
OutputFormat::OutputFormat(int type)
{
  resolution = type;
  outTypes = 0;
  actUnits = new char[3];
  strcpy(actUnits,"Bq");
  actMult = 1;

  normUnits = new char[5];
  strcpy(normUnits,"/cm3");
  normType = 1;

  cooltimeUnits = new char[5];
  strcpy(cooltimeUnits,"def");
  cooltimeType = 1;

  gammaSrc = NULL;
  contactDose = NULL;

  next = NULL;
}

OutputFormat::OutputFormat(const OutputFormat& o) :
  resolution(o.resolution), outTypes(o.outTypes), normType(o.normType), actMult(o.actMult), cooltimeType(o.cooltimeType)

{
  actUnits = new char[strlen(o.actUnits)+1];
  strcpy(actUnits,o.actUnits);

  normUnits = new char[strlen(o.normUnits)+1];
  strcpy(normUnits,o.normUnits);

  cooltimeUnits = new char[strlen(o.cooltimeUnits)+1];
  strcpy(cooltimeUnits,o.cooltimeUnits);

  next = NULL;
}
  
OutputFormat::~OutputFormat()
{
  delete[] actUnits;
  delete[] normUnits;
  delete[] cooltimeUnits;
  delete gammaSrc;
  delete contactDose;
  delete next;
  next = NULL;
  
}

/** The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object. Note that
    'next' is NOT copied, the object will continue to be part of the
    same list unless explicitly changed. */
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
  normType = o.normType;

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
  char *fileNamePtr;

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

      verbose(3,"Added output type %d (%s)",1<<type,token);

      switch (1<<type)
	{
	case OUTFMT_UNITS:
	  next->outTypes |= 1<<type;
	  delete[] next->actUnits;
	  input >> token;
	  next->actUnits = new char[strlen(token)+1];
	  strcpy(next->actUnits,token);
	  next->actMult = (tolower(token[0]) == 'c'?BQ_CI:1);

	  delete[] next->normUnits;
	  input >> token;
	  next->normUnits = new char[strlen(token)+2];
	  strcpy((next->normUnits)+1,token);
	  if (tolower(token[0]) == 'v') {
	    next->normUnits[0] = ' ';
	  } else {
	    next->normUnits[0] = '/';
	  }
	  switch (tolower(token[0]))
	    {
	    case 'm':
	      next->normType = OUTNORM_M3;
	      break;
	    case 'g':
	      next->normType = OUTNORM_G;
	      break;
	    case 'k':
	      next->normType = OUTNORM_KG;
	      break;
	    case 'v':
	      next->normType = OUTNORM_VOL_INT;
	      break;
	    default:
	      next->normType = OUTNORM_CM3;
	      break;
	    }
	  break;
	case OUTFMT_WDR:
          next->outTypes |= 1<<type;
	  input >> token;
	  fileNamePtr = new char[strlen(token)+1];
	  strcpy(fileNamePtr,token);
	  next->wdrFilenames.insert(fileNamePtr);
	  verbose(4,"Added WDR/Clearance file %s", token);
	  break;
	case OUTFMT_SRC:
	  next->outTypes |= 1<<type;
	  /* set gamma source file name here */
	  next->gammaSrc= new GammaSrc(input,GAMMASRC_RAW_SRC);
	  break;
	case OUTFMT_CDOSE:
          next->outTypes |= 1<<type;
	  //Need to determine which dose approximation is defined
          char approx_token[64];
          input >> approx_token;
	  if (approx_token[0] == 'l' || approx_token[0] == 'L') {

	    //adjust outTypes
	    //next->outTypes -= OUTFMT_CDOSE;
	    next->outTypes += OUTFMT_EXP; 
	    /* setup gamma source for exposure dose with line approximation */
	    next->exposureDose = new GammaSrc(input, GAMMASRC_EXPOSURE);
	  }
	  else if ( approx_token[0] == 'v' || approx_token[0] == 'V' ) {
	    //adjust outTypes
	    //next->outTypes -= OUTFMT_CDOSE;
	    next->outTypes += OUTFMT_EXP_CYL_VOL; 
	    /* setup gamma source for exposure dose with line approximation */
	    next->exposureCylVolDose = new GammaSrc(input, GAMMASRC_EXPOSURE_CYLINDRICAL_VOLUME);
	  }
	  else if (approx_token[0] == 'c' || approx_token[0] == 'C') {
	    next->outTypes |= 1<<type;
	     /* setup gamma source for contact dose */
	     next->contactDose = new GammaSrc(input,GAMMASRC_CONTACT);
	    }

	  //Add more types of dose output here
	  break;

	case OUTFMT_ADJ:
	  next->outTypes |= 1<<type;
	  /* setup gamma source for adjoint dose */
	  next->outTypes |= 1<<type;
	  next->adjointDose = new GammaSrc(input,GAMMASRC_ADJOINT);
          break;	

	case OUTFMT_INT_ENG:
	  verbose(4,"Calculating gamma source by preserving power.");
	  next->outTypes |= 1<<type;
        break;

	default:
        /* use logical and to set the correct bit in the outTypes field */
	   next->outTypes |= 1<<type;
	  break;
	   
	}

      clearComment(input);
      input >> token;
    }	      

  return next;

}

/** It does this by calling the write() function on the list of intervals,
    zones or mixtures, as determined by the 'resolution' member. The last
    argument is the kza number for the target isotope for which the
    current invocation is being called. */
void OutputFormat::write(Volume* volList, Mixture* mixList, Loading* loadList,
			 CoolingTime *coolList, int targetKza)
{

  OutputFormat *ptr = this;
  char buffer[256];

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
	   << ptr->actUnits << " " << ptr->normUnits << " " << ptr->cooltimeUnits << endl;
      /* regular singular responses */
      for (++outTypeNum;outTypeNum<lastSingularResponse;outTypeNum++)
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	    switch(1<<outTypeNum)
	      {
	      case (OUTFMT_ACT):
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->actUnits,ptr->normUnits,ptr->cooltimeUnits);
		break;
	      case (OUTFMT_SRC) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
				/* deliver gamma src filename, */
			ptr->normUnits, ptr->gammaSrc->getFileName(),ptr->actUnits,ptr->normUnits,ptr->cooltimeUnits); 
		break;
	      case (OUTFMT_CDOSE) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->contactDose->getFileName());
		break;
	      case (OUTFMT_ADJ) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->adjointDose->getFileName());
		break;
	      case (OUTFMT_EXP) : 
		sprintf(buffer, Out_Types_Str[outTypeNum],
			ptr->exposureDose->getFileName());
		break;
	      case (OUTFMT_EXP_CYL_VOL) :
		sprintf(buffer, Out_Types_Str[outTypeNum],
			ptr->exposureCylVolDose->getFileName());
		break;
	      default:
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->normUnits);
	      }
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
      Result::setNorm(ptr->actMult,ptr->normType);

      /* for each indicated response */
      for (outTypeNum=firstResponse;outTypeNum<lastSingularResponse;outTypeNum++) {
	if (ptr->outTypes & 1<<outTypeNum)
	  {
	    /* write a response title */
	    switch(1<<outTypeNum)
	      {
	      case(OUTFMT_ACT):
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->actUnits,ptr->normUnits);
		break;
	      case (OUTFMT_SRC) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->normUnits, ptr->gammaSrc->getFileName(),ptr->actUnits,ptr->normUnits);
                bool integrate_energy;
                if(ptr->outTypes & OUTFMT_INT_ENG){
                  integrate_energy = true;
                  }
                else
                  integrate_energy = false;
		ptr->gammaSrc->setIntEng(integrate_energy);
		/* set gamma source to use for this */
		Result::setGammaSrc(ptr->gammaSrc);
		break;
	      case (OUTFMT_CDOSE) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->contactDose->getFileName());
		/* setup gamma attenuation coefficients */
		ptr->contactDose->setGammaAttenCoef(mixList);
		/* set gamma source to use for this */
		Result::setGammaSrc(ptr->contactDose);
		break;
	      case (OUTFMT_ADJ) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->adjointDose->getFileName());
		/* read/set flux-dose conversion factors */
		ptr->adjointDose->setAdjDoseData(volList);
		/* set gamma source to use for this */
		Result::setGammaSrc(ptr->adjointDose);
		break;
	      case (OUTFMT_EXP) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
                       ptr->exposureDose->getFileName());
        	/* setup gamma attenuation coefficients */
		ptr->exposureDose->setGammaAttenCoef(mixList);
		Result::setGammaSrc(ptr->exposureDose);
		break;
	      case (OUTFMT_EXP_CYL_VOL) :
		sprintf(buffer,Out_Types_Str[outTypeNum],
			ptr->exposureCylVolDose->getFileName());
        	/* setup gamma attenuation coefficients */
		ptr->exposureCylVolDose->setGammaAttenCoef(mixList);
		Result::setGammaSrc(ptr->exposureCylVolDose);
		break;
	      default:
		sprintf(buffer,Out_Types_Str[outTypeNum],ptr->normUnits);
	      }
	    cout << "*** " << buffer << " ***" << endl; 

	    Result::setReminderStr(buffer);

	    /* call write() on the appropriate object determined by
               the resulotition */
	    switch(ptr->resolution)
	      {
	      case OUTRES_INT:
		volList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
			       coolList,targetKza,ptr->normType);
		break;
	      case OUTRES_ZONE:
		loadList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
				coolList,targetKza,ptr->normType);
		break;
	      case OUTRES_MIX:
		mixList->write(1<<outTypeNum,ptr->outTypes & OUTFMT_COMP,
			       coolList,targetKza,ptr->normType);
		break;
	      }

	    cout << endl << endl << endl;
	  }
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
	      

	      sprintf(buffer,"%s: %s",Out_Types_Str[outTypeNum],*fileName);

	      Result::setReminderStr(buffer);

	      Node::loadWDR(*fileName);
	      
	      /* call write() on the appropriate object determined by
		 the resulotition */
	      switch(ptr->resolution)
		{
		case OUTRES_INT:
		  volList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				 coolList,targetKza,ptr->normType);
		  break;
		case OUTRES_ZONE:
		  loadList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				  coolList,targetKza,ptr->normType);
		  break;
		case OUTRES_MIX:
		  mixList->write(OUTFMT_WDR,ptr->outTypes & OUTFMT_COMP,
				 coolList,targetKza,ptr->normType);
		  break;
		}
	      
	      delete [] *fileName;
	      cout << endl << endl << endl;
	    }
	  
	} 
    }

}
