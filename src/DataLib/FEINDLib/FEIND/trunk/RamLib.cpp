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

ErrCode RamLib::AddPCs(const Kza parent, const int csType, 
		       const std::vector<double>& cs)
{
  Data[parent].CrossSections[csType] = cs;
  return FEC_NO_ERROR;
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

ErrCode RamLib::AddSecPath(const Kza parent, const Kza daughter, 
			   const Path& path)
{
  Data[parent].Secondary[daughter].PathList.push_back(path);
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddPath(const Kza parent, const Kza daughter, const Path& path)
{
  Data[parent].Daughters[daughter].PathList.push_back(path);
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddDCs(const Kza parent, const Kza daughter, const int csType,
		       const std::vector<double>& cs)
{
  Data[parent].Daughters[daughter].CrossSections[csType] = cs;
  return FEC_NO_ERROR;
}

ErrCode RamLib::AddDecayConstant(Kza parent, const double constant)
{
  Data[parent].DecayConstant = constant;
  return FEC_NO_ERROR;
}

// ErrCode RamLib::AddBranchingRatio(const Kza parent, const Kza daughter,
// 				  const double br)
// {
//   Data[parent].Daughters[daughter].BranchingRatio = br;
//   return FEC_NO_ERROR;
// }

// ErrCode RamLib::AddDecayMode(const Kza parent, const int mode, const int dIso)
// {
//   if(mode != SPONTANEOUS_FISSION)
//     {
//       int daughter = DecayModetoKza(mode,dIso,parent);
//       Data[parent].Daughters[daughter].DecayMode = dIso;
//     }

//   return FEC_NO_ERROR;
// }

ErrCode RamLib::AddFissionYield(const Kza parent, const Kza daughter,
				const int fissionType, const double yield)
{
  Data[parent].Daughters[daughter].FissionYield[fissionType] = yield;
  return FEC_NO_ERROR;
}

vector<double> RamLib::GetPCs(Kza parent, int csType)
{
  return Data[parent].CrossSections[csType];
}

double RamLib::GetBratio(Kza parent, Kza daughter)
{
  return Data[parent].Daughters[daughter].BranchingRatio;
}

double RamLib::GetSecBratio(Kza parent, Kza sec_daughter)
{
  return Data[parent].Secondary[sec_daughter].BranchingRatio;
}

double RamLib::GetDecayConstant(Kza parent)
{
  return Data[parent].DecayConstant;

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

std::vector<Kza> RamLib::SecDaughters(Kza parent)
{
  map<int,Daughter>::iterator iter = Data[parent].Secondary.begin();
  vector<Kza> ret;
  
  while(iter != Data[parent].Secondary.end())
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
      Data[parent].Daughters[daughter].DecayMode = decayMode;
      Data[parent].Daughters[daughter].BranchingRatio = br;
    }
  else
    {
      Data[parent].Sfbr = br;
    }

  if(sec)
    {
      Data[parent].Secondary[sec].DecayMode = decayMode;
      Data[parent].Secondary[sec].BranchingRatio = br;
    }

  return FEC_NO_ERROR;
}

vector<double> RamLib::GetDCs(Kza parent, Kza daughter, int csType)
{
  return Data[parent].Daughters[daughter].CrossSections[csType];
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
			 vector<Kza>& priDs, vector<double>& priDsLambda,
			 vector<Kza>& secDs, vector<double>& secDsLambda)
{
  double decay_const = GetDecayConstant(parent);
  double total = decay_const;
  int i,j;

  priDs = Daughters(parent);
  secDs = SecDaughters(parent);

  priDsLambda.assign(priDs.size(), 0.0);
  secDsLambda.assign(secDs.size(), 0.0);

  vector<double> cs;

  for(i = 0; i < priDs.size(); i++)
    {
      cs = GetDCs(parent, priDs[i], TOTAL_CS);

      if(cs.size())
	{

	  for(j = 0; j < flux.size(); j++)
	    {
	      priDsLambda[i] += cs[j]*flux[j];
	    }
	}

      total += priDsLambda[i];
      priDsLambda[i] += GetBratio(parent,priDs[i])*decay_const;
    }

  for(i = 0; i < secDs.size(); i++)
    {
      cs = GetSec(parent, secDs[i], TOTAL_CS);

      if(cs.size())
	{
	  for(j = 0; j < flux.size(); j++)
	    {
	      secDsLambda[i] += cs[j]*flux[j];
	    }
	}

      secDsLambda[i] += decay_const * GetSecBratio(parent, secDs[i]);
    }

  return total;
}
