#include "DoseResponse.h"

#include "GammaSrc.h"

#include "Input/Flux.h"

DoseResponse::DoseResponse(char *name,char *flxName, char *gammaSrcName)
{
  respName = NULL;
  if (name != NULL)
    {
      respName = new char[strlen(name)+1];
      strcpy(respName,name);
    }

  fluxName = NULL;
  if (flxName != NULL)
    {
      fluxName = new char[strlen(flxName)+1];
      strcpy(fluxName,flxName);
    }

  gSrcName = NULL;
  if (gammaSrcName != NULL)
    {
      gSrcName = new char[strlen(gammaSrcName)+1];
      strcpy(gSrcName,gammaSrcName);
    }

  next = NULL;
}

DoseResponse::DoseResponse(const DoseResponse& d)
{
  respName = NULL;
  if (d.respName != NULL)
    {
      respName = new char[strlen(d.respName)+1];
      strcpy(respName,d.respName);
    }

  fluxName = NULL;
  if (d.fluxName != NULL)
    {
      fluxName = new char[strlen(d.fluxName)+1];
      strcpy(fluxName,d.fluxName);
    }

  gSrcName = NULL;
  if (d.gSrcName != NULL)
    {
      gSrcName = new char[strlen(d.gSrcName)+1];
      strcpy(gSrcName,d.gSrcName);
    }

  next = NULL;
}


DoseResponse& DoseResponse::operator=(const DoseResponse& d)
{
  if (this == &d)
    return *this;

  delete respName;
  delete fluxName;
  delete gSrcName;
  respName = NULL;
  fluxName = NULL;
  gSrcName = NULL;

  if (d.respName != NULL)
    {
      respName = new char[strlen(d.respName)+1];
      strcpy(respName,d.respName);
    }

  if (d.fluxName != NULL)
    {
      fluxName = new char[strlen(d.fluxName)+1];
      strcpy(fluxName,d.fluxName);
    }

  if (d.gSrcName != NULL)
    {
      gSrcName = new char[strlen(d.gSrcName)+1];
      strcpy(gSrcName,d.gSrcName);
    }

  return *this;

}


DoseResponse* DoseResponse::add(char* name, char *flxName, char* gammaSrcName)
{
  next = new DoseResponse(name, flxName, gammaSrcName);

  return next;
}


void DoseResponse::xCheck(GammaSrc *gSrcList,Flux *fluxList)
{

  DoseResponse *ptr = this;

  while (ptr->next != NULL)
    {
      ptr = ptr->next;

      verbose(3,"Checking for adjoint flux %s and gamma source structure %s in dose response %s.",
	      ptr->fluxName, ptr->gSrcName, ptr->respName);

      /* set ordinal flux number */
      int tmpFlux = fluxList->find(ptr->fluxName,ptr);
      /* check that flux is valid */
      switch (tmpFlux)
	{
	case FLUX_NOT_FOUND:
	  error(9999,"Adjoint flux %s for dose response %s does not exist.",
		ptr->fluxName,ptr->respName);
	  break;
	case FLUX_BAD_FNAME:
	  error(9999,"Bad flux file for adjoint flux %s of dose response %s.",
		ptr->fluxName,ptr->respName);
	  break;
	default:
	  verbose(4,"Adjoint flux %s was found.",ptr->fluxName);
	  delete ptr->fluxName;
	  ptr->fluxNum = tmpFlux;
	}
      
      /* set pointer to gamma source */
      GammaSrc *tmpGSrc = gSrcList->find(ptr->gSrcName);
      if (tmpGSrc == NULL)
	error(9999,"Gamma source structure %s for response does %s not exist.",
	      ptr->gSrcName,ptr->respName);
      else
	{
	  verbose(4,"Gamma source structure %s was found.",ptr->gSrcName);
	  delete gSrcName;
	  ptr->gSrc = tmpGSrc;
	}
    }

}


int DoseResponse::getNumGroups()
{
  return gSrc->getNumGroups();
}
