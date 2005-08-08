#include <iostream>

#include "RamLib.h"
#include "Parent.h"

using namespace std;
using namespace FEIND;

std::vector<Kza> RamLib::Parents()
{
  map<Kza,Parent>::iterator iter;
  iter = Data.begin();

  vector<Kza> ret;

  while(iter != Data.end())
    {
      ret.push_back(iter->first);
      iter++;
    }

  return ret;
}

ErrCode RamLib::SetPCs(const Kza parent, const int csType, 
		       const std::vector<double>& cs, bool add)
{

  if(!add || Data[parent].CrossSections.find(csType) == 
     Data[parent].CrossSections.end())
    {
      Data[parent].CrossSections[csType] = cs;
    }
  else {

    // EXCEPTION: cs size

    vector<double>& old_cs = Data[parent].CrossSections[csType];
    int i;
    
    for(i = 0; i < cs.size(); i++)
      old_cs[i] += cs[i];
  }

  return FEC_NO_ERROR;
}

const vector<double>& RamLib::GetPCs(Kza parent, int csType)
{
  return Data[parent].CrossSections[csType];
}

ErrCode RamLib::SetDCs(const Kza parent, const Kza daughter, const int csType,
		       const std::vector<double>& cs, bool add)
{
  if(!add || Data[parent].Daughters[daughter].CrossSections.find(csType) ==
     Data[parent].Daughters[daughter].CrossSections.end() )
    {
      // The cross-section does not exist...or the user wants to replace it
      Data[parent].Daughters[daughter].CrossSections[csType] = cs;
    }
  else
    {
      // EXCEPTION: cs size

      vector<double>& old_cs = Data[parent].Daughters[daughter].
	CrossSections[csType];

      int i;

      for(i = 0; i < cs.size(); i++)
	old_cs[i] += cs[i];
    }

  return FEC_NO_ERROR;
}

const vector<double>& RamLib::GetDCs(Kza parent, Kza daughter, int csType)
{
  return Data[parent].Daughters[daughter].CrossSections[csType];
}

ErrCode RamLib::AddSpectrum(const Kza parent, const int specType, 
			    const Spectrum& spec)
{
  Data[parent].SpecList[specType] = spec;
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddDecayEnergy(const Kza parent, const int enType, 
			       const double energy)
{
  Data[parent].DecayEnergies[enType] = energy;
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddPath(const Kza parent, const Kza daughter, const Path& path)
{
  Data[parent].Daughters[daughter].PathList.push_back(path);
  return FEC_NO_ERROR;
}


ErrCode RamLib::AddDecayConstant(Kza parent, const double constant)
{
  Data[parent].DecayConstant = constant;
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddFissionYield(const Kza parent, const Kza daughter,
				const int fissionType, const double yield)
{
  Data[parent].Daughters[daughter].FissionYield[fissionType] = yield;
  return FEC_NO_ERROR;
}


double RamLib::GetSfbr(Kza parent)
{
  map<Kza,Parent>::iterator iter = Data.find(parent);

  // Check to make sure parent exists:
  if(iter != Data.end())
    {
      // Parent found:
      return iter->second.Sfbr;
    }

  // Parent not found, return default value:
  return 0.0;
}

double RamLib::GetBratio(Kza parent, Kza daughter)
{
  map<Kza,Parent>::iterator parent_iter = Data.find(parent);

  // Make sure parent exists:
  if(parent_iter != Data.end())
    {
      // Parent exists, now, check for daughter:
      map<Kza,Daughter>::iterator daughter_iter = 
	parent_iter->second.Daughters.find(daughter);
      
      if(daughter_iter != parent_iter->second.Daughters.end())
	{
	  // Parent and daughter exist, exit:
	  return daughter_iter->second.BranchingRatio;
	}
      // Daughter does not exist...
    }
  // Return default value:
  return 0.0;
}

double RamLib::GetDecayConstant(Kza parent)
{
  map<Kza,Parent>::iterator parent_iter = Data.find(parent);

  // Make sure parent exists:
  if(parent_iter != Data.end())
    {
      // Parent found
      return parent_iter->second.DecayConstant;
    }

  // Return default value:
  return 0;
}

double RamLib::GetDecayEnergy(Kza parent, int enType)
{
  return Data[parent].DecayEnergies[enType];
}

double RamLib::GetTotalDecayEnergy(Kza parent)
{
  map<int,double>::iterator iter = Data[parent].DecayEnergies.begin();

  double ret = 0;

  while(iter != Data[parent].DecayEnergies.end())
    {
      ret += iter->second;
      iter++;
    }

  return ret;
}

vector<pair<double,double> > RamLib::GetDiscreteSpec(Kza parent, int specType)
{
  return Data[parent].SpecList[specType].Discrete;
}


vector<pair<double,double> > 
RamLib::GetFissionYield(Kza parent, int fissionType)
{
  map<int,Daughter>::iterator iter = Data[parent].Daughters.begin();
  vector<pair<double,double> > ret;

  cout << "Start\n";

  while(iter != Data[parent].Daughters.end())
    {
      if(iter->second.FissionYield[fissionType])
	{
	  ret.push_back(pair<double,double>
			(iter->first, iter->second.FissionYield[fissionType]));
	}

      cout << iter->first << '\t' << iter->second.FissionYield[fissionType]
	   << endl;

      iter++;
    }
  
  return ret;
}


double RamLib::GetFissionYield(Kza parent, Kza daughter, int fissionType)
{
  return Data[parent].Daughters[daughter].FissionYield[fissionType];
}

vector<Kza> RamLib::Daughters(Kza parent)
{
  map<int,Daughter>::iterator iter = Data[parent].Daughters.begin();
  vector<Kza> ret;
  
  while(iter != Data[parent].Daughters.end())
    {
      ret.push_back(iter->first);
      iter++;
    }

  return ret;
}

ErrCode RamLib::AddDecayMode(Kza parent, int decayMode, int dIso, double br)
{
  Kza sec;
  Kza daughter = DecayModetoKza(decayMode, dIso, parent, sec);

  if(decayMode != SPONTANEOUS_FISSION)
    {
      Data[parent].DecayModes.push_back(decayMode);
      Data[parent].Daughters[daughter].BranchingRatio += br;
    }
  else
    {
      Data[parent].Sfbr = br;
    }

  if(sec)
    {
      Data[parent].DecayModes.push_back(decayMode);
      Data[parent].Daughters[sec].BranchingRatio += br;
    }

  return FEC_NO_ERROR;
}

ErrCode RamLib::LambdaEff(Kza parent, const vector<double>& flux, 
			  double& result)
{
  int i, j;
  vector<Kza> d_kzas = Daughters(parent);
  vector<double> cs;

  result = 0;

  for(i = 0; i < d_kzas.size(); i++)
    {

      // Check to see if there is a total cross section for this daughter:
      cs = GetDCs(parent, d_kzas[i], TOTAL_CS);

      if(cs.size())
	{
	  // Make sure correct group structure:
	  if(cs.size() != flux.size()) return FEC_BAD_GROUP_STRUCT;

	  for(j = 0; j < flux.size(); j++)
	    {
	      result += flux[j]*cs[j];
	    }	  
	}

    }
  //  Convert from barns:
  result *= 1E-24;

  // Add decay contribution:
  result += GetDecayConstant(parent);

  return FEC_NO_ERROR;
}

ErrCode RamLib::LambdaEff(Kza parent, Kza daughter, 
			  const vector<double>& flux, double& result)
{
  int i;

  vector<double> cs = GetDCs(parent,daughter,TOTAL_CS);

  result = 0;

  if(cs.size())
    {
      if(cs.size() != flux.size()) return FEC_BAD_GROUP_STRUCT;

      for(i = 0; i < flux.size(); i++)
	{
	  result += flux[i]*cs[i];
	}
    }

  result *= 1E-24;

  result += GetDecayConstant(parent)*GetBratio(parent,daughter);

  return FEC_NO_ERROR;
}

double RamLib::LambdaEff(Kza parent, const std::vector<double>& flux,
			 vector<Kza>& daughters, vector<double>& dLambdas,
			 double& secLambda, FissionType ft)
{
  double decay_constant = GetDecayConstant(parent);

  double sfbr = GetSfbr(parent);
  double sfyield = 0.0;
  double fission_sigphi = 0.0;
  vector<double> fission_cs = GetPCs(parent, NEUTRON_FISSION_CS);

  // The return value (primary lambda effective)
  double pri_lambda = decay_constant;
  
  double total_lambda = 0;

  // Reference to total cross-section
  const vector<double>& total_cs = GetPCs(parent, TOTAL_CS);

  // Get the list of daughter kzas
  daughters = Daughters(parent);

  // Dimension the daughter lambda vector appropriately
  dLambdas.assign(daughters.size(), 0.0);

  int i,j;

  // EXCEPTION: cs size

  // Calculate the primary lambda effective:
  if(total_cs.size())
    {
      for(i = 0; i < flux.size(); i++)
	{
	  pri_lambda += flux[i] * total_cs[i];
	}
    }
  // Calculate lambda effective for each daughter:
  
  // Check to see if we should precalculate sigma*phi for fission:
  if(fission_cs.size())
    {
      // EXCEPTION: cs size

      for(i = 0; i < fission_cs.size(); i++)
	fission_sigphi += fission_cs[i]*flux[i];
      
      // Add the fission daughter to the daughter list...
      daughters.push_back(FISSION_DAUGHTER);
      dLambdas.push_back(fission_sigphi);
    }

  for(i = 0; i < daughters.size(); i++)
    {
      const vector<double>& cs = GetDCs(parent, daughters[i], TOTAL_CS);

      // Make sure the cross-section exists:
      if(cs.size())
	{
	  // EXCEPTION: cs size

	  for(j = 0; j < flux.size(); j++)
	    {
	      dLambdas[i] += cs[j] * flux[j];
	    }
	}

      dLambdas[i] += GetBratio(parent, daughters[i])*decay_constant;

      // Make sure to handle spontaneous fission, which is NOT included in the
      // normal decay branching ratios..
      if(sfbr > 0.0 && 
	 (sfyield = GetFissionYield(parent,daughters[i],FISSION_SF)))
	dLambdas[i] += sfbr*decay_constant*sfyield;

      // Add non-spontaneous fission contribution..
      if(fission_sigphi && ft != NO_FISSION)
	dLambdas[i] += fission_sigphi*GetFissionYield(parent,daughters[i],ft);

      total_lambda += dLambdas[i];
    }

  // Calculate the secondary lambda:
  secLambda = total_lambda - pri_lambda;

  return pri_lambda;
}

std::vector<double> RamLib::GetGroupStruct( GSType gst )
{
  // EXCEPTION - Invalid group structure

  return GroupStructs[gst];
}

void RamLib::SetGroupStruct( GSType gst, std::vector<double>& gs)
{
  GroupStructs[gst] = gs;
}


void RamLib::ConstructAdjoint()
{
  
  // Get a list of all of the parents
  vector<Kza> parents = Parents();
  Kza parent;
  Kza daughter;

  map<Kza, Daughter>::iterator iter;
  int i,k;
  
  bool parent_exists;

  // Loop through all parents...
  for(i = 0; i < parents.size(); i++)
    {
      parent = parents[i];
      map<Kza,Daughter>& daughter_map = Data[parent].Daughters;

      iter = daughter_map.begin();

      while(iter != daughter_map.end())
	{
	  daughter = iter->first;
	  parent_exists = false;

	  // Check to see if this parent is already in the daughter list:
	  for(k = 0; k < Adjoint[daughter].size(); k++)
	    {
	      if(parent == Adjoint[daughter][k])
		{
		  // The parent already exists
		  parent_exists = true;
		  break;
		}
	    }
	  
	  if(!parent_exists) Adjoint[daughter].push_back(parent);
	  
	  iter++;
	}
    }
}


const std::vector<Kza>& RamLib::Parents(Kza daughter)
{
  return Adjoint[daughter];
}


double RamLib::ProdRates(Kza daughter, const vector<double>& flux,
			 vector<Kza>& parents, vector<double>& pLambdas, 
			 FissionType ft)
{
  double decay_constant;
  
  // The total destruction rate, return value:
  double pri_lambda = GetDecayConstant(daughter);
  
  int i,j;
  
  const vector<double>& total_cs = GetPCs(daughter, TOTAL_CS);
  
  double fission_yield;
  double b_ratio;
  double sfbr;
  double sigphi = 0;

  // Calculate pri_lambda:
  for(i = 0; i < total_cs.size(); i++)
    {
      pri_lambda += total_cs[i]*flux[i];
    }  

  parents = Parents(daughter);
  
  // Initialize the production rate vector:

  pLambdas.assign(parents.size(), 0.0);

  // Check to see if we should precalculate sigma*phi for fission:
  const vector<double>& fission_cs = GetPCs(daughter, NEUTRON_FISSION_CS);

  if(fission_cs.size())
    {
      // EXCEPTION: cs size

      for(i = 0; i < fission_cs.size(); i++)
	pri_lambda += fission_cs[i]*flux[i];
    }


   for(i = 0; i < parents.size(); i++)
     {
       const vector<double>& cs = GetDCs(parents[i], daughter, TOTAL_CS);

       if(cs.size())
 	{
	  for(j = 0; j < cs.size(); j++)
	    {
	      pLambdas[i] += cs[j]*flux[j];
	    }
 	}

       // Now, check for fission...
       const vector<double>& f_cs = GetDCs(parents[i], daughter, 
					   NEUTRON_FISSION_CS);
       
       if(f_cs.size() && ft != NO_FISSION)
	 {
	   // Get the yield...
	   fission_yield = GetFissionYield(parents[i], daughter, ft);

	   if(fission_yield)
	     {
	       for(j = 0; j < cs.size(); j++)
		 {
		   pLambdas[i] += f_cs[j]*flux[j]*fission_yield;
		 }
	     }
	 }

       // Now decay...
       b_ratio = GetBratio(parents[i], daughter);
       decay_constant = GetDecayConstant(parents[i]);
       pLambdas[i] += decay_constant*b_ratio;

       // Finally...spontaneous fission...
       pLambdas[i] += GetSfbr(parents[i])*decay_constant*
	 GetFissionYield(parents[i],daughter,FISSION_SF);
     }

   return pri_lambda;
}
