#include <iostream>
#include <cctype>
#include <cmath>
#include <cstdlib>

#include "DecayEndf6.h"
#include "LibDefine.h"
#include "RamLib.h"

using namespace FEIND;
using namespace std;

DecayEndf6::DecayEndf6(const LibDefine& lib) :
  InFile(lib.Args[0].c_str()),
  FileName(lib.Args[0].c_str())
{
}

void DecayEndf6::LoadLibrary() throw(ExFileOpen, ExDecayMode)
{
  unsigned int num_spec;

  if(!InFile.is_open())
    throw ExFileOpen("DecayEndf6::LoadLibrary() function", FileName);

  Kza parent_kza;

  Parent parent;

  string str;

  while(getline(InFile, str, '\n'))
    {
      // Decay data only exists on lines ending with 8457:
      if(!Is8457(str)) continue;

      // First line in section contains the parent:
      parent_kza = ExtractParent(str);

      // First line also contains the number of decay spectra:
      num_spec = atoi(str.substr(55,11).c_str());

      // Second line contains half life:
      getline(InFile, str, '\n');
      Library.AddDecayConstant(parent_kza, ExtractDecayConst(str));

      // Get the Decay Energies, and add them to the library:
      ExtractEnergies(parent_kza, str);

      // Get the information for each decay daughter:
      ExtractDecayModes(parent_kza);

      ExtractSpectrum(num_spec, parent_kza);
      
      while(Is8457(str))
	getline(InFile, str, '\n');
    }
}

bool DecayEndf6::Is8457(const string& str)
{
  // Make sure to check that the string is long enough:
  if(str.size() == 80 && str.substr(71,4) == "8457") 
    return true;

  return false;
}

double DecayEndf6::FormatFloat(string str)
{
  if( (str.find('+') != string::npos || str.find('-') != string::npos) &&
      (str.find('e') == string::npos && str.find('E') == string::npos) )
    {
      // There is a + or - present, and no E, so put one in:
      unsigned int loc;

      if((loc = str.find('+')) == string::npos)
        loc = str.find('-');

      str.insert(loc,"E");
    }

  return atof(str.c_str());
}

Kza DecayEndf6::ExtractParent(const std::string& str)
{
  return Kza( FormatFloat(str.substr(0,11)) * 10 + 
	      FormatFloat(str.substr(33,11)) );

}

double DecayEndf6::ExtractDecayConst(const std::string& str)
{
  // Get the half life, the convert to decay constant:
  return log(2.0)/FormatFloat(str.substr(0,11));
}

void DecayEndf6::ExtractEnergies(Kza parent, string& str)
{
  map<int, double> ret;

  // NOTE: Normally we would use str to find the number of decay energies
  //       either 3 or 17. But from examining the data it appears that ALL
  //       of the decay entries have only 3 decay energies.
  //       Because of this we'll just skip this line and get the 3 energies.
  getline(InFile, str, '\n');
  
  // Order of decay energies is: 
  // Light Particles (any electrons), 
  // Electromagnetic Radiation (any photons that leave nucleus)
  // Heavy Particles (alphas, i think)
  Library.AddDecayEnergy(parent, LIGHT_PARTICLES, 
			 FormatFloat(str.substr(0,11)));

  Library.AddDecayEnergy(parent, EM_RADIATION, 
			 FormatFloat(str.substr(22,11)));

  Library.AddDecayEnergy(parent, HEAVY_PARTICLES, 
			 FormatFloat(str.substr(44,11)));
}

void DecayEndf6::ExtractDecayModes(Kza parent) throw(ExDecayMode)
{
  string str;
  int num_modes;
  int i;
  DecayModeType decay_mode;

  double branching_ratio;

  getline(InFile, str, '\n');
  num_modes = atoi(str.substr(55,11).c_str());

  for(i = 0; i < num_modes; i++)
    {
      getline(InFile, str, '\n');
      
      decay_mode = ModeEtoF(FormatFloat(str.substr(0,11)));

      branching_ratio = FormatFloat(str.substr(44,11));

      try{
	Library.AddDecayMode(parent, decay_mode,
			     int(FormatFloat(str.substr(11,11))),
			     branching_ratio);
      } catch (ExDecayMode& ex){ 
	throw; 
      }
      
    }
}

void DecayEndf6::ExtractSpectrum(unsigned int num, Kza parent)
{
  unsigned int i;
  unsigned int j;
  unsigned int k;
  string str;
  int spec_type;
  unsigned int lcon;
  unsigned int num_disc;
  double disc_rel;
  double cont_rel;

  double energy;
  double intensity;
  int line_parm;
  int disc_cont;

  int num_points;
  int num_regions;
  int cov_flag;

  Spectrum spectrum;
  
  for(i = 0; i < num; i++)
    {
      // Read the spectrum header, which consists of two lines:
      getline(InFile, str, '\n');
      spec_type = int(FormatFloat(str.substr(11,11)));
      disc_cont = atoi(str.substr(22,11).c_str());
      num_disc = atoi(str.substr(55,11).c_str());

      getline(InFile, str, '\n');
      disc_rel = FormatFloat(str.substr(0,11));
      cont_rel = FormatFloat(str.substr(44,11));

      // We now have enough information to begin reading discrete spectrum:
      for(j = 0; j < num_disc; j++)
	{
	  // Read information about the energy:
	  getline(InFile, str, '\n');
	  energy = FormatFloat(str.substr(0,11));
	  line_parm = atoi(str.substr(44,11).c_str());
	     
	  // Read information about the intensity:
	  getline(InFile, str, '\n');
	  intensity = FormatFloat(str.substr(22,11));

	  // Check for unecessary information:
	  if(line_parm > 6) getline(InFile, str, '\n');
	  
	  // Now add the spectrum entry, but check to make sure its not zero:
	  if(intensity = intensity*disc_rel)
	    {
	      spectrum.Discrete.push_back(pair<double,double>(energy, 
							      intensity));
	    }	  
	}

      // Now, lets handle continuous:
      // If it exists:
      if(disc_cont == 1 || disc_cont == 2)
	{
	  int count = 0;
	  ContSpec cont;
	  vector<int> last_point;
	  vector<int> int_method;
	  vector<pair<double, double> > region_info;
	  vector<pair<double, double> > energy_info;

	  // Read continuum header:
	  getline(InFile, str, '\n');
	  num_points = atoi(str.substr(55,11).c_str());
	  num_regions = atoi(str.substr(44,11).c_str());
	  cov_flag = atoi(str.substr(33,11).c_str());

	  //region_info.push_back(pair<double,double>(0,0));
	  region_info = ExtractPairs(num_regions);
	  region_info.insert(region_info.begin(),pair<double,double>(0,0));
	  energy_info = ExtractPairs(num_points);

	  for(j = 0; j < num_regions; j++)
	    {
	      cont.IntMethod = int(region_info[j+1].second);

	      for(k = int(region_info[j].first); 
		  k < int(region_info[j+1].first); k++)
		{
		  cont.Point.push_back(energy_info[k]);
		}
	      
	      spectrum.Continuous.push_back(cont);
	      cont.Clear();
	    }

	  if(cov_flag)
	    {
	      // Now, just skip covariance data:
	      getline(InFile, str, '\n');
	      num_points = atoi(str.substr(55,11).c_str());

	      for(j = 0; j < num_points; j += 3) getline(InFile, str, '\n');
	    }
	}

      Library.AddSpectrum(parent, SpectrumEtoF(spec_type), spectrum);
      
      spectrum.Continuous.clear();
      spectrum.Discrete.clear();

    }
}

DecayModeType DecayEndf6::ModeEtoF(double endf)
{
  if ( endf == 0 )
    return GAMMA_DECAY;
  else if ( endf == 1 )
    return BETA_DECAY;
  else if ( endf == 2 )
    return ELECTRON_CAPTURE;
  else if ( endf == 3 )
    return ISOMERIC_TRANSITION;
  else if ( endf == 4 )
    return ALPHA_DECAY;
  else if ( endf == 5 )
    return NEUTRON_EMISSION;
  else if ( endf == 6 )
    return SPONTANEOUS_FISSION;
  else if ( endf == 7 )
    return PROTON_EMISSION;
  else if ( endf == 1.5 )
    return BETA_NEUTRON_EMIT;
  else if ( endf == 1.4 )
    return BETA_ALPHA_EMIT;
  else if ( endf == 2.4 )
    return POSITRON_ALPHA_EMIT;
  else if ( endf == 2.7)
    return POSITRON_PROTON_EMIT;
  else if ( endf == 3.4)
    return IT_ALPHA_EMIT;

  return UNKNOWN;
}

vector< pair<double, double> > DecayEndf6::ExtractPairs(int num)
{
  vector< pair<double, double> > ret;
  double first;
  double second;

  string str;
  int i;
  int j;
  int get;

  for(i = 0; i < num; i += 3)
    {
      // Find out the number of points we should get on this line:
      get = num - i;
      if( get > 3) get = 3;

      getline(InFile, str, '\n');
      
      for(j = 0; j < get; j++)
	{
	  // Extract the numbers from the string:
	  first = FormatFloat(str.substr(j*22,11));
	  second = FormatFloat(str.substr(j*22+11,11));

	  // Add the numbers to our return list:
	  ret.push_back(pair<double,double>(first, second));
	}
    }
  
  return ret;
}

int DecayEndf6::SpectrumEtoF(double endf)
{
  if(endf == 0)      return GAMMA;
  else if(endf == 1) return BETA;
  else if(endf == 2) return POSITRON;
  else if(endf == 4) return ALPHA;
  else if(endf == 5) return NEUTRON;
  else if(endf == 6) return SF_FRAGMENTS;
  else if(endf == 7) return PROTON;
  else if(endf == 8) return ELECTRON;
  else if(endf == 9) return XRAY;

  return 0;
}
