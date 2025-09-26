/* $Id: alara.C,v 1.20 2004-07-29 19:24:10 wilsonp Exp $ */
#include "alara.h"

#include "Input.h"
#include "Root.h"
#include "Statistics.h"
#include "Result.h"

int chainCode = 0;

/*!  
 This list of elemental symbols is specially formatted to be used for
 looking up the atomic number of a given element.  For each element
 with atomic number, Z, and symbol, CC, the string " CC " (note
 spaces) exists at index Z-1.
*/
const char *SYMBOLS=" h  he li be b  c  n  o  f  ne na mg al si p  s  cl ar \
k  ca sc ti v  cr mn fe co ni cu zn ga ge as se br kr rb sr y  zr nb mo tc ru \
rh pd ag cd in sn sb te i  xe cs ba la ce pr nd pm sm eu gd tb dy ho er tm \
yb lu hf ta w  re os ir pt au hg tl pb bi po at rn fr ra ac th pa u  np \
pu am cm bk cf es fm md no lr ";

/*!
 This is the standard help/usage message that is printed when an incorrect
 command-line option is used, or when -h is used.
*/
static const char *helpmsg="\
usage: %s [-h] [-r] [-t <tree_filename>] [-V] [-v <n>] [-o <output_filename>] [<input_filename>] \n\
\t -h                 Show this message\n\
\t -c                 Option to only calculate chains and skip post-processing\n\
\t -r                 \"Restart\" option to skip chain calculation and only post-process\n\
\t -t <tree_filename> Create tree file with given name\n\
\t -V                 Show version\n\
\t -v <n>             Set verbosity level\n\
\t -o <output_filename>  Name of file in which output is written (optional)\n\
\t <input_filename>   Name of input file\n\
See Users' Guide for more info.\n\
(http://alara.engr.wisc.edu/)\n";

int main(int argc, char *argv[])
{  
  std::string out_file;
  std::ofstream outfile;
  std::streambuf* oldCout = std::cout.rdbuf();  
  int argNum = 1; /// count command-line arguments
  int solved = FALSE; /// command-line derived flag to indicate whether or not the tree has already been solved
  int doOutput = TRUE; /// command-line derived flag to indicate whether or not to post-process solution
  char *inFname = NULL; /// input filename
  Root* rootList = new Root; /// primary data structure 1: is a linked list of Root objects
  topSchedule* schedule; /// primary data structure 2: irradiation history

  verbose(-1,"%s",PACKAGE_STRING);

  while (argNum<argc)
    {
      if (argv[argNum][0] != '-')
	{
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
	}

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
/*	case 'v':
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
	  break;	*/

	case 'v':
	  if (argv[argNum][1] == '\0')
	    {
	      if (argNum<argc-1)
              {
		verb_level = atoi(argv[argNum+1]);
	        argNum+=2;
	      }
	     else
		error(2,"-v requires parameter."); 
	    }
	  else
	    {
	      verb_level = atoi(argv[argNum]+1);
	      argNum++;
	    }
	  verbose(0,"Set verbose level to %d.",verb_level);
	  break;
	
 	case 'c':
	  verbose(0,"Calculating chains ONLY.");
	  doOutput=FALSE;
	  argNum+=1;
	  break;
	case 'r':
          verbose(0,"Reusing binary dump data.");
	  solved=TRUE;
	  argNum+=1;
	  break;
	case 't':
	  if (argv[argNum][1] == '\0')
	    {
	      if (argNum<argc-1)
              {
                 Statistics::initTree(argv[argNum+1]);
	         verbose(0,"Opened tree file %s.",argv[argNum+1]);
	         argNum+=2;
	      }
	      else
		 error(2,"-t requires parameter."); 
	    }
	  else
	    {
	      Statistics::initTree(argv[argNum]+1);
	      verbose(0,"Opened tree file %s.",argv[argNum]+1);
	      argNum++;
	    }
	  break;

    case 'o':
        int used_args = 1;
		if (argv[argNum][1] == '\0') 
			{
			if (argNum<argc-1) 
					out_file = argv[argNum+1]; 
			else 
				{
				error(2, "-o requires parameter.");
				break;
				}
			} 
		else 
			out_file = argv[argNum]+1;

		outfile.open(out_file);	
		if (!outfile.is_open())
			error(1, "Cannot create output file %s.", out_file.c_str());
		std::cout.rdbuf(outfile.rdbuf());
		verbose(0, "Verbose output redirected to %s", out_file.c_str());

		if (argv[argNum][1] == '\0' && argNum<argc-1)	
			argNum += 2;	
		else
			argNum ++;
		break;

	case 'h':
	  verbose(-1,helpmsg,argv[0]);
	case 'V':
	  exit(0);
	  break;
	default:
	  {
	    verbose(-1,helpmsg,argv[0]);
	    error(0,"Invalid option: %s.",argv[argNum]);
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

  if (doOutput)
    {
      Result::resetBinDump();
      problemInput.postProc(rootList);

      verbose(0,"Output.");
    }

  Result::closeBinDump();

  delete rootList;
  delete inFname;

  std::cout.rdbuf(oldCout);
  outfile.close();

  return 0;
}
