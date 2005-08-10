#include <iostream>
#include <cctype>

#include "RamLib.h"
#include "DecayEndf6.h"
#include "EndfIeaf.h"
#include "LibDefine.h"

using namespace std;
using namespace FEIND;

EndfIeaf::EndfIeaf(const LibDefine& lib) :
  InFile(lib.Args[0].c_str())
{  
}

ErrCode EndfIeaf::LoadLibrary()
{
  string str;
  Kza parent_kza;
  Kza daughter_kza;
  int num_groups;

  if(!InFile.is_open())
    return FEC_FILE_OPEN;

  while(getline(InFile,str,'\n'))
    {
      if(!Is35(str))
	continue;
      
      parent_kza = int(DecayEndf6::FormatFloat(str.substr(0,11)));
      daughter_kza = int(DecayEndf6::FormatFloat(str.substr(11,11)));

      num_groups = atoi(str.substr(55,11).c_str());

      ExtractCs(parent_kza, daughter_kza, num_groups);
    }
}

bool EndfIeaf::Is35(const string& str)
{
  // Make sure the string is long enough:
  if(str.size() < 75)
    return false;

  if(str.substr(71,4) == "3  5")
    return true;
  return false;
}

void EndfIeaf::ExtractCs(Kza parent, Kza daughter, int num)
{
  XSec cs(num);
  string str;
  int first_group;

  // Get the number and cross-section of the first group:
  getline(InFile,str,'\n');
  first_group = atoi(str.substr(55,11).c_str());
  getline(InFile,str,'\n');
  cs[first_group-1] = DecayEndf6::FormatFloat(str.substr(11,11));


  for(int i = first_group; i < num; i++)
    {
      getline(InFile,str,'\n');
      getline(InFile,str,'\n');
      cs[i] = DecayEndf6::FormatFloat(str.substr(11,11));
    }

  // Add the data to the library:
  if(daughter)
    {
      Library.SetDCs(parent,daughter,TOTAL_CS,cs);
    }
  else
    {
      Library.SetPCs(parent,TOTAL_CS,cs);
    }
}
