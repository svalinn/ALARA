/* $Id: alara.C,v 1.5 1999-08-24 22:06:14 wilson Exp $ */
#include "alara.h"

#include "Input/Input.h"
#include "Chains/Root.h"
#include "Util/Statistics.h"

int chainCode = 0;

const char *SYMBOLS=" h  he li be b  c  n  o  f  ne na mg al si p  s  cl ar \
k  ca sc ti v  cr mn fe co ni cu zn ga ge as se br kr rb sr y  zr nb mo tc ru \
rh pd ag cd in sn sb te i  xe cs ba la ce pr nd pm sm eu gd tb dy ho er tm \
yb lu hf ta w  re os ir pt au hg tl pb bi po at rn fr ra ac th pa u  np \
pu am cm bk cf es fm md no lr ";


int main(int argc, char *argv[])
{
  int argNum = 1;
  char *inFname = NULL;
  Root* rootList = new Root;
  topSchedule* schedule;

  while (argNum<argc)
    {
      if (argv[argNum][0] != '-')
	if (inFname == NULL)
	  {
	    inFname = new char[strlen(argv[argNum])+1];
	    strcpy(inFname,argv[argNum]);
	    argNum++;
	    /* get next argument */
	    continue;
	  }
	else
	  error(1,"Only one input filename can be specified: %s.",inFname);

      while (argv[argNum][0] == '-')
	argv[argNum]++;
      switch (argv[argNum][0])
	{
#ifdef DVLPR	  
	case 'd':
	  if (argv[argNum][1] == '\0')
	    {
	      debug_level = atoi(argv[argNum+1]);
	      argNum+=2;
	    }
	  else
	    {
	      debug_level = atoi(argv[argNum]+1);
	      argNum++;
	    }
	  debug(0,"Set debug level to %d.",debug_level);
	  break;
#endif
	case 'v':
	  if (argv[argNum][1] == '\0')
	    {
	      verb_level = atoi(argv[argNum+1]);
	      argNum+=2;
	    }
	  else
	    {
	      verb_level = atoi(argv[argNum]+1);
	      argNum++;
	    }
	  verbose(0,"Set verbose level to %d.",verb_level);
	  break;
	case 't':
	  if (argv[argNum][1] == '\0')
	    {
	      Statistics::initTree(argv[argNum+1]);
	      verbose(0,"Openned tree file %s.",argv[argNum+1]);
	      argNum+=2;
	    }
	  else
	    {
	      Statistics::initTree(argv[argNum]+1);
	      verbose(0,"Openned tree file %s.",argv[argNum]+1);
	      argNum++;
	    }
	  break;
	default:
	  {
	    warning(0,"Option %s is not implemented yet.",argv[argNum]);
	    argNum++;
	  }
	}
    }

  Input problemInput(inFname);

  /* INPUT */
  verbose(0,"Starting problem input processing.");
  verbose(1,"Reading input.");
  problemInput.read();
  verbose(1,"Cross-checking input for completeness and self-consistency.");
  problemInput.xCheck();
  verbose(1,"Preprocessing input.");
  problemInput.preProc(rootList,schedule);

  verbose(0,"Starting problem solution.");
  
  rootList->solve(schedule);

  verbose(1,"Solved problem.");

  problemInput.postProc(rootList);

  verbose(0,"Output.");

  delete rootList;

}
