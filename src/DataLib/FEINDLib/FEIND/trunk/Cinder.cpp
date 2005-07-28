#include <iostream>
#include <string>
#include <cctype>

#include "Parent.h"
#include "Cinder.h"
#include "LibDefine.h"
#include "RamLib.h"

using namespace std;
using namespace FEIND;

Cinder::Cinder(const LibDefine& lib) :
  InFile(lib.Args[0].c_str())
{
}

ErrCode Cinder::LoadLibrary()
{
  string str;

  if(!InFile.is_open())
    return FEC_FILE_OPEN;

  // This file is divided into three sections:
  // - The first contains the groups structure information
  // - The second contains transmutation and decay data
  // - The third contains fission yield data

  // Read gamma and neutron groups structures
  GetGroupInfo();

  // Begin Reading Activation Data:  
  ActivationData();

  // Read the Fission Yields:
  FissionYields();

  return FEC_NO_ERROR;
}

void Cinder::ActivationData()
{
  const string start_iso = "_______________________";
  const string begin_fission = "Fission Yield Data";
  const string fission_rxn = "(n,f)";

  string str;
  string path_str;
  vector<double> cs(NumNeutronGroups,0);
  bool total_flag = false;
  vector<double> parent_cs;

  int i;
  int j;
  int num_rxn;
 
  Kza parent_kza;
  Kza daughter_kza;

  Path path;

  getline(InFile, str, '\n');
  while(str.find(begin_fission) == string::npos)
    {
      if(str.find(start_iso) != string::npos)
	{
	  // Found new isotope:
	  parent_kza = ExtractActParent();
	  
	  getline(InFile,str,'\n');
	  while(str.find(fission_rxn) == string::npos)
	    {
	      getline(InFile,str,'\n');
	    }

	  // Check for fission cross section. If it exists, add it to the
	  // library
	  if(CheckFission(str))
	    {
	      // There is a non-zero fission cross-section:
	      for(i = 0; i < NumNeutronGroups; i++)
		{
		  InFile >> str;
		  cs[i] = atof(str.c_str());
		} 

	      Library.AddPCs(parent_kza, NEUTRON_FISSION_CS, cs);
	      AddToCs(parent_kza, cs);
	    }

	  // Done with fission, deal with other transmutation reactions:
	  InFile >> str;
	  num_rxn = atoi(str.c_str());
	  getline(InFile,str,'\n');

	  for(i = 0; i < num_rxn; i++)
	    {
	      getline(InFile,str,'\n');
	      daughter_kza = atoi(str.substr(10,7).c_str());
	      daughter_kza = CinderToKza(daughter_kza);

	      // Grab the CINDER path string:
	      path_str = str.substr(50,4);

	      // Create total parent/daughter cross sections:
	      if(path_str.find('c') == string::npos || path_str.find('x') || 
		 path_str == "    ") 
		  total_flag = true;
	      else 
		  path = MakePath(path_str);

	      for(j = 0; j < NumNeutronGroups; j++)
		{
		  InFile >> str;
		  cs[j] = atof(str.c_str());
		}

	      if(!total_flag) path.CrossSection = cs;	      

 	      if(IsPri(parent_kza, daughter_kza)) 
		{
		  // This is a primary cross-section
		  // It must be added to the parent total cross-section:
		  AddToCs(parent_kza, cs);

		  // Since there is only one cross-section per daughter, this
		  // will be used as the total daughter cross-section
		  Library.AddDCs( parent_kza, daughter_kza, TOTAL_CS, cs );

		  if(!total_flag) 
		    Library.AddPath(parent_kza, daughter_kza, path);
		}
 	      else
		{
		  Library.AddSec(parent_kza, daughter_kza, TOTAL_CS, cs);

		  if(!total_flag) 
		    Library.AddSecPath(parent_kza, daughter_kza, path);
		}

 	      if(!total_flag)
 		{
 		  if(IsPri(parent_kza,daughter_kza))
 		    Library.AddPath(parent_kza, daughter_kza, path);
 		  else
 		    Library.AddSecPath(parent_kza, daughter_kza, path);
 		}

	      getline(InFile,str,'\n');
	      
	      total_flag = false;
	    }
	}

      getline(InFile, str, '\n');
    }    
}

Kza Cinder::ExtractActParent()
{
  string str;

  for(int i = 0; i < 6; i++)
    InFile.get();

  InFile >> str;
  return CinderToKza(atoi(str.c_str()));
}

bool Cinder::CheckFission(const string& str)
{
  if(str.find("0, 0, 0") == string::npos)
    return true;
  return false;
}

void Cinder::FissionYields()
{
  int i;
  int j;
  int num_parents;
  int num_daughters;
  string str;
  vector<Kza> parent_list;
  vector<char> fission_type;
  vector<double> yields;
  Kza daughter;

  // Read Parent List:
  InFile >> str;
  num_parents = atoi(str.c_str());
  yields.assign(num_parents, 0);
  getline(InFile, str, '\n');

  // Get the kza of all fission parents, store in parent_list:
  ExtractFissionParents(num_parents, parent_list, fission_type);
  
  // The next section contains atomic masses of parents we don't need this 
  // information:
  SkipMasses(num_parents);
  
  // Get number of daughters:
  InFile >> str;
  num_daughters = atoi(str.c_str());
  getline(InFile, str, '\n');

  // Get the Yields, and store them:
  for(i = 0; i < num_daughters; i++)
    {
      yields = ExtractYield(num_parents, daughter);
      
      for(j = 0; j < num_parents; j++)
	{
	  Library.AddFissionYield(parent_list[j], daughter, fission_type[j],
				  yields[j]);
	}
    }
}

void Cinder::ExtractFissionParents(int num, vector<Kza>& parents, 
				   vector<char>& fissionType)
{
  int i;
  int z;
  int a;
  int d_iso;
  string str_a;
  string str;
  vector<Kza> ret(num);
  char type;
  Kza kza;

  for(i = 0; i < num; i++)
    {
      d_iso = 0;
      // Get rid of parent number (1, 2, ..., num):
      InFile >> str;
      
      // Get Z number:
      getline(InFile, str, '-');
      z = atoi(str.c_str());
      
      // Get A number:
      InFile >> str;

      str_a = "";
      str_a += str[str.size()-4];
      str_a += str[str.size()-3];
      str_a += str[str.size()-2];
       
     if(str_a.find("m") != string::npos)
	{
	  // We have a meta stable state:
	  d_iso = 1;
	  str_a[2] = str_a[1];
	  str_a[1] = str_a[0];
	  str_a[0] = '2';
	}
      
      a = atoi(str_a.c_str());

      kza = z*10000 + a*10 + d_iso;

      // Get fission type:
      type = str[str.size()-1];

      // Add the parent and type of fission to the list:
      parents.push_back(kza);
      fissionType.push_back(FissionType(type));
    }

}

void Cinder::SkipMasses(int num)
{
  string str;
  for(int i = 0; i <= num/4; i++)
    {
      getline(InFile, str, '\n');
    }
}

vector<double> Cinder::ExtractYield(int num, Kza& daughter)
{
  string str;
  vector<double> ret;

  InFile >> str >> str;
  daughter = CinderToKza( atoi(str.c_str()) );
  
  for(int i = 0; i < num; i++)
    {
      InFile >> str;
      ret.push_back(atof(str.c_str()));
    }
  return ret;
}

Kza Cinder::CinderToKza(int cinder)
{
  return cinder/10000*10 + ((cinder/10)%1000)*10000 + cinder%10;
}

int Cinder::FissionType(char t)
{
  switch(t)
    {
    case 't':
      return FISSION_THERMAL;
      break;
    case 's':
      return FISSION_SF;
      break;
    case 'f':
      return FISSION_FAST;
      break;
    case 'h':
      return FISSION_HOT;
      break;
    }

  return 0;
}

Path Cinder::MakePath(const string& str)
{
  int i;
  Path ret;
  ret.Projectile = NEUTRON;
  
  // We only care about paths that are not blank, c's or x's. While x's
  // are total cross sections, and should be added to the Library with the
  // Library.AddDCs(parent,daughter,cs,TOTAL_CS) command, this will happen in
  // the AddToCs function.
  
  int part_id;
  int num = 1;
  string character;

  for(int i = 0; i < str.size(); i++)
    {
      // Ignore blank characters:
      if(str[i] == ' ') continue;

      if(str[i] >= '0' && str[i] <= '9') 
	{
	  character = str[i];
	  num = atoi(character.c_str());
	  continue;
	}

      switch(toupper(str[i]))
	{
	case 'G':
	  part_id = GAMMA;
	  break;
	case 'N':
	  part_id = NEUTRON;
	  break;
	case 'P':
	  part_id = PROTON;
	  break;
	case 'A':
	  part_id = ALPHA;
	  break;
	case 'T':
	  part_id = TRITON;
	  break;
	case 'H':
	  part_id = HELIUM3;
	  break;
	case 'D':
	  part_id = DEUTERON;
	  break;
	default:
	  break;
	  // THROW AN EXCEPTION
	}
      
      ret.Emitted[part_id] = num;
      num = 1;
    }
  
  return ret;
}

void Cinder::AddToCs(Kza parent_kza, vector<double>& newCs)
{
  vector<double> parent_cs = Library.GetPCs(parent_kza, TOTAL_CS);

  if(parent_cs.size() != newCs.size())
    {
      Library.AddPCs(parent_kza, TOTAL_CS, newCs);
    }
  else
    {
      // Add the cross sections:
      for(int i = 0; i < NumNeutronGroups; i++)
	parent_cs[i] += newCs[i];

      Library.AddPCs(parent_kza, TOTAL_CS, parent_cs);
    }
}

bool Cinder::IsPri(Kza parent, Kza daughter)
{  
  if( double((daughter/10)%1000) >= ((parent/10)%1000)/2.0 ) return true;
  
  return false; 
}
  

void Cinder::GetGroupInfo()
{
  unsigned int i;
  string str;

  // Skip the first line...
  getline(InFile, str, '\n');

  // Get the number of neutron groups...
  InFile >> str >> str >> str;
  NumNeutronGroups = atoi(str.c_str());

  // Get the number of gamma groups...
  InFile >> str >> str >> str >> str >> str >> str;
  NumGammaGroups = atoi(str.c_str());

  // Read the neutron group boundaries:
  getline(InFile, str, '\n');
  getline(InFile, str, '\n');
  for(i = 0; i < NumNeutronGroups+1; i++)
    {
      InFile >> str;
      NeutronGroupBounds.push_back( atof(str.c_str()) );
    }
  Library.SetGroupStruct(CINDER_NEUTRON, NeutronGroupBounds);

  // Read the gamma group boundaries:
  getline(InFile, str, '\n');
  getline(InFile, str, '\n');
  for(i = 0; i < NumGammaGroups+1; i++)
    {
      InFile >> str;
      GammaGroupBounds.push_back( atof(str.c_str()) );
    }
  Library.SetGroupStruct(CINDER_GAMMA, GammaGroupBounds);  
}
