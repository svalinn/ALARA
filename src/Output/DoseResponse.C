#include "DoseResponse.h"

DoseResponse::DoseResponse(int type) :
  format(type), skip(0), scale(0)
{
  respName = NULL;
  fileName = NULL;

  next = NULL;
}

DoseResponse::DoseResponse(char *name, char *fname, double inScale, 
			   int inSkip, int type ) :
  format(type), skip(inSkip), scale(inScale)
{
  respName = NULL;
  fileName = NULL;

  if (name != NULL)
    {
      respName = new char[strlen(name)+1];
      strcpy(respName,name);
    }

  if (fname != NULL)
    {
      fileName = new char[strlen(fname)+1];
      strcpy(fileName,fname);
    }

  next = NULL;
}

DoseResponse::DoseResponse(const DoseResponse& d) :
  format(d.format), skip(d.skip), scale(d.scale)
{
  respName = NULL;
  fileName = NULL;

  if (d.respName != NULL)
    {
      respName = new char[strlen(d.respName)+1];
      strcpy(respName,d.respName);
    }

  if (d.fileName != NULL)
    {
      fileName = new char[strlen(d.fileName)+1];
      strcpy(fileName,d.fileName);
    }

  next = NULL;
}


DoseResponse& DoseResponse::operator=(const DoseResponse& d)
{
  if (this == &d)
    return *this;

  format = d.format;
  skip = d.skip;
  scale = d.scale;

  delete respName;
  delete fileName;
  respName = NULL;
  fileName = NULL;

  if (d.respName != NULL)
    {
      respName = new char[strlen(d.respName)+1];
      strcpy(respName,d.respName);
    }

  if (d.fileName != NULL)
    {
      fileName = new char[strlen(d.fileName)+1];
      strcpy(fileName,d.fileName);
    }

  return *this;

}


DoseResponse* DoseResponse::add(char* name, char* fname, double inScale, 
				int inSkip, int inFmt);
{
  next = new DoseResponse(name,fname,inScale,inSkip,inFmt);

  return next;
}

