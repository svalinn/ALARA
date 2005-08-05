#include <iostream>
#include <cctype>

#include "Eaf41.h"
#include "LibDefine.h"
#include "RamLib.h"

using namespace FEIND;
using namespace std;

const int Eaf41::NumGroups = 175;

Eaf41::Eaf41(const LibDefine& lib) :
  InFile(lib.Args[0].c_str())
{
  Cpt.push_back(PROTON);
  Cpt.push_back(DEUTERON);
  Cpt.push_back(TRITON);
  Cpt.push_back(HELIUM3);
  Cpt.push_back(ALPHA);
}

ErrCode Eaf41::LoadLibrary()
{
  int non_zero,i, fission_type = 0;
  string str;

  Kza parent_kza;
  Kza daughter_kza;

  string path_str;
  string daughter_str;
  
  Path path;

  if(!InFile.is_open())
    return FEC_FILE_OPEN;

  SkipHeader();

  getline(InFile, str, '\n');
  while(!InFile.eof())
    {
      fission_type = 0;

      // Get the parent:
      parent_kza = atoi(str.substr(0,7).c_str());

      // Now get the number of non-zero groups:
      non_zero = atoi(str.substr(14,3).c_str());

      // Get path and daughter info:
      path_str = str.substr(28,5);
      daughter_str = str.substr(34,7);

      if(path_str[2] == 'F')
	{
	  // Fission Reaction
	  // Find the type of fission:
	  fission_type = FissionType(path_str[0]);
	}
      else
	{
	  path = ConvertPath(path_str);	  
	  daughter_kza = DaughterEtoF(daughter_str);
	}

      // Skip 2 lines:
      getline(InFile,str,'\n');
      getline(InFile,str,'\n');

      for(i = 0; i < 175; i++)
	{
	  if(i < non_zero)
	    {
	      InFile >> str;
	      path.CrossSection.push_back(atof(str.c_str()));
	    }
	  else
	    {
	      path.CrossSection.push_back(0.0);
	    }
	}

      if(!fission_type)
	{
	  // Add the parent, parent/daughter contributions:
	  Library.SetPCs(parent_kza, TOTAL_CS, path.CrossSection, true);
	  Library.SetDCs(parent_kza, daughter_kza, TOTAL_CS, 
			 path.CrossSection, true);

	  // Add the parent/daughter/path data:
	  Library.AddPath(parent_kza, daughter_kza, path);

	  // Add secondary daughters to the daughter list:
	  AddCPPCS(parent_kza,path);
	}
      else
	{
	  // Set the fission cross-section:
	  Library.SetPCs(parent_kza, fission_type, path.CrossSection);
	}
      
      // Every reaction in this file format contributes to the total
      // cross-section
      Library.SetPCs(parent_kza, TOTAL_CS, path.CrossSection, true);      
      
      path.CrossSection.clear();

      // Get the header for this reaction:
      getline(InFile, str, '\n');
      if(str == "") getline(InFile, str, '\n');
    }

  return FEC_NO_ERROR;
}

void Eaf41::SkipHeader()
{
  string str;

  while(str[0] != '#')
    getline(InFile, str, '\n');
}

Path Eaf41::ConvertPath(const string& str)
{
  Path ret;
  int i;
  int num_part = 1;

  string character;

  // First character in str is the projectile:
  ret.Projectile = ParticleEtoF(str[0]);

  for(i = 2; i < str.size(); i++)
    {
      character = str[i];
      // Ignore blank spaces:
      if(str[i] != ' ')
	{
	  // Check for a number:
	  if( !atoi(character.c_str()) )
	    {
	      // Not a number:
	      ret.Emitted[ParticleEtoF(str[i])] += num_part;
	      num_part = 1;
	      
	    }
	  else
	    {
	      // It is a number:
	      num_part = atoi(character.c_str());
	    }
	}
    }

  return ret;
}

int Eaf41::ParticleEtoF(const char part)
{
  switch(toupper(part))
    {
    case 'N':
      return NEUTRON;
    case 'G':
      return GAMMA;
    case 'D':
      return DEUTERON;
    case 'A':
      return ALPHA;
    case 'T':
      return TRITON;
    case 'P':
      return PROTON;
    case 'H':
      return HELIUM3;
    }

  return 0;
}

Kza Eaf41::DaughterEtoF(const string& daughter)
{
  int i;
  string element;
  int z;
  int a;
  int iso = 0;

  // Take care of Z:
  if(daughter[1] == ' ')
    {
      // element symbol is single character...
      // ie C, O, N...
      element = daughter[0];
    }
  else
    {
      // element symbol is two characters...
      // ie He, Be, Xe...
      element = daughter.substr(0,2);
      element[0] = toupper(element[0]);
      element[1] = tolower(element[1]);
    }
  z = GetAtomicNumber(element);

  // Take care of A:
  a = atoi(daughter.substr(2,3).c_str());

  // Take care of Isomeric state:
  if(daughter.find('M') != string::npos)
    iso = atoi(daughter.substr(6,1).c_str()); 

  return z*10000 + a*10 + iso;
}

int Eaf41::FissionType(const char projectile)
{
  switch(ParticleEtoF(projectile))
    {
    case NEUTRON:
      return NEUTRON_FISSION_CS;
    }
}

void Eaf41::AddCPPCS(Kza parent, Path& path)
{
  // This function will add any charged particles produced to the production
  // cross-section.

  vector<double> cs;
  int i,j;

  for(i = 0; i < Cpt.size(); i++)
    {
      if(path.Emitted.find(Cpt[i]) != path.Emitted.end())
	{
	  if(!cs.size()) cs.assign(path.CrossSection.size(), 0.0);

	  Library.SetDCs(parent, Cpt[i], TOTAL_CS, path.CrossSection,
			 true, path.Emitted[Cpt[i]]);
	}
    }
}
