/* $Id: alara.C,v 1.13 2002-01-07 22:00:46 wilsonp Exp $ */
#include "alara.h"

#include "Input/Input.h"
#include "Chains/Root.h"
#include "Util/Statistics.h"
#include "Output/Result.h"

int chainCode = 0;

const char *SYMBOLS=" h  he li be b  c  n  o  f  ne na mg al si p  s  cl ar \
k  ca sc ti v  cr mn fe co ni cu zn ga ge as se br kr rb sr y  zr nb mo tc ru \
rh pd ag cd in sn sb te i  xe cs ba la ce pr nd pm sm eu gd tb dy ho er tm \
yb lu hf ta w  re os ir pt au hg tl pb bi po at rn fr ra ac th pa u  np \
pu am cm bk cf es fm md no lr ";

static char *id="$Name: not supported by cvs2svn $";

static char *helpmsg="\
usage: %s [-h] [-r] [-t <tree_filename>] [-V] [-v <n>] [<input_filename>] \n\
\t -h                 Show this message\n\
\t -r                 Restart option for calculating new respones\n\
\t -t <tree_filename> Create tree file with given name\n\
\t -V                 Show version\n\
\t -v <n>             Set verbosity level\n\
\t <input_filename>   Name of input file\n\
See Users' Guide for more info.\n\
(http://www.cae.wisc.edu/~wilsonp/projects/ALARA/users.guide.html/)\n";

int main(int argc, char *argv[])
{
  int argNum = 1;
  int solved = FALSE;
  char *inFname = NULL;
  Root* rootList = new Root;
  topSchedule* schedule;

  verbose(-1,"ALARA %s",id);

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
	case 'r':
          verbose(0,"Reusing binary dump data.");
	  solved=TRUE;
	  argNum+=1;
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
	case 'h':
	  verbose(-1,helpmsg,argv[0]);
	case 'V':
	  exit(0);
	  break;
	default:
	  {
	    verbose(-1,helpmsg,argv[0]);
	    error(0,"Invlaid option: %s.",argv[argNum]);
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

  if (!solved)
    {
      verbose(0,"Starting problem solution.");
      
      rootList->solve(schedule);
      
      verbose(1,"Solved problem.");
    }

  Result::resetBinDump();
  problemInput.postProc(rootList);

  verbose(0,"Output.");

  Result::closeBinDump();

  delete rootList;
  delete inFname;

}
