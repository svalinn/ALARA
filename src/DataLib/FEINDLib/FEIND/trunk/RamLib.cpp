#include <iostream>
#include <sstream>

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

void RamLib::SetPCs(const Kza parent, XSecType csType, XSec cs, bool add) 
  throw(ExXsecSize, ExEmptyXSec)
{

  if(!add || Data[parent].CrossSections.find(csType) == 
     Data[parent].CrossSections.end())
    {
      // Replace the old cross-section with the new one...
      Data[parent].CrossSections[csType] = cs;
    }
  else {

    // Get the old cs...
    XSec old_cs = Data[parent].CrossSections[csType];

    // Add the two cross-sections together...
    try{ old_cs += cs; }
    catch(ExXsecSize& ex) {
      stringstream ss;
      ss << "\nError in RamLib::SetPCs, An attempt was made to add a parent cross section"
	 << "\nto another cross-section with different group structures. Perhaps you"
	 << "\nmeant to replace the cross-section?"
	 << "\nError occured for parent: " << parent << endl;
      ex.AddToDetailed(ss.str());
      throw;
    }
    
    // WARNGING - As soon as the value of the old cross section was changed,
    // (in operator+=) it was probably copied to a new address!

    // We have to reset the cross-section!
    SetPCs(parent, csType, old_cs, false);
  }
}

XSec RamLib::GetPCs(Kza parent, int csType)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return NULLCS;

  map<int,XSec>::iterator cs_iter = p_iter->second.CrossSections.find(csType);
  if(cs_iter == p_iter->second.CrossSections.end()) return NULLCS;

  // The cross-section does exist...
  return cs_iter->second;
}

void RamLib::SetDCs(const Kza parent, const Kza daughter, XSecType csType,
		    XSec cs, bool add) throw(ExXsecSize, ExEmptyXSec)
{
  if(!add || Data[parent].Daughters[daughter].CrossSections.find(csType) ==
     Data[parent].Daughters[daughter].CrossSections.end() )
    {
      // The cross-section does not exist...or the user wants to replace it
      Data[parent].Daughters[daughter].CrossSections[csType] = cs;
    }
  else
    {
      XSec old_cs = Data[parent].Daughters[daughter].CrossSections[csType];

      try{ old_cs += cs; }
      catch(ExXsecSize& ex) {
	stringstream ss;
	ss << "\nError in RamLib::SetDCs, An attempt was made to add a p/d cross section"
	   << "\nto another cross-section with different group structures. Perhaps you"
	   << "\nmeant to replace the cross-section?"
	   << "\nError occured for parent: " << parent << endl;
	ex.AddToDetailed(ss.str());
	throw;
      }

      SetDCs(parent, daughter, csType, old_cs, false);
    }
}

XSec RamLib::GetDCs(Kza parent, Kza daughter, int csType)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return NULLCS;

  map<Kza,Daughter>::iterator d_iter = p_iter->second.Daughters.find(daughter);
  if(d_iter == p_iter->second.Daughters.end()) return NULLCS;

  map<int,XSec>::iterator cs_iter = d_iter->second.CrossSections.find(csType);
  if(cs_iter == d_iter->second.CrossSections.end()) return NULLCS;

  // The cross-section exists...
  return cs_iter->second;
}

void RamLib::AddSpectrum(const Kza parent, const int specType, 
			    const Spectrum& spec)
{
  Data[parent].SpecList[specType] = spec;
}

void RamLib::AddDecayEnergy(const Kza parent, const int enType, 
			       const double energy)
{
  Data[parent].DecayEnergies[enType] = energy;
}

void RamLib::AddPath(const Kza parent, const Kza daughter, const Path& path)
{
  Data[parent].Daughters[daughter].PathList.push_back(path);
}

vector<Path> RamLib::GetPaths(const Kza parent, const Kza daughter)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return vector<Path>();

  map<Kza,Daughter>::iterator d_iter = p_iter->second.Daughters.find(daughter);
  if(d_iter == p_iter->second.Daughters.end()) return vector<Path>();

  return Data[parent].Daughters[daughter].PathList;
}

void RamLib::AddDecayConstant(Kza parent, const double constant)
{
  Data[parent].DecayConstant = constant;
}

void RamLib::AddFissionYield(const Kza parent, const Kza daughter,
				const int fissionType, const double yield)
{
  Data[parent].Daughters[daughter].FissionYield[fissionType] = yield;
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
  map<Kza,Parent>::iterator parent_iter = Data.find(parent);
  if(parent_iter == Data.end()) return 0;

  map<int,double>::iterator en_iter = 
    parent_iter->second.DecayEnergies.find(enType);

  if(en_iter == parent_iter->second.DecayEnergies.end()) return 0;

  return en_iter->second;
}

double RamLib::GetTotalDecayEnergy(Kza parent)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return 0;

  map<int,double>::iterator iter = Data[parent].DecayEnergies.begin();

  double ret = 0;

  while(iter != Data[parent].DecayEnergies.end())
    {
      ret += iter->second;
      iter++;
    }

  return ret;
}

const vector<pair<double,double> >& RamLib::GetDiscreteSpec(Kza parent, 
							    int specType)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return EMPTY_PAIR_DOUBLE;

  map<int,Spectrum>::iterator s_iter = p_iter->second.SpecList.find(specType);
  if(s_iter == p_iter->second.SpecList.end())
    return EMPTY_PAIR_DOUBLE;

  return s_iter->second.Discrete;
}

double RamLib::GetFissionYield(Kza parent, Kza daughter, int fissionType)
{
  map<Kza,Parent>::iterator p_iter = Data.find(parent);
  if(p_iter == Data.end()) return 0;

  map<Kza,Daughter>::iterator d_iter = p_iter->second.Daughters.find(daughter);
  if(d_iter == p_iter->second.Daughters.end()) return 0;

  map<int,double>::iterator f_iter = 
    d_iter->second.FissionYield.find(fissionType);

  if(f_iter == d_iter->second.FissionYield.end()) return 0;

  return f_iter->second;
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

void RamLib::AddDecayMode(Kza parent, DecayModeType decayMode, int dIso, 
			  double br) throw(ExDecayMode)
{
  Kza sec;
  Kza daughter;

  daughter = DecayModetoKza(decayMode, dIso, parent, sec);

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
}

void RamLib::LambdaEff(Kza parent, const vector<double>& flux, 
			  double& result)
{
  int i, j;
  vector<Kza> d_kzas = Daughters(parent);
  XSec cs;

  result = 0;

  for(i = 0; i < d_kzas.size(); i++)
    {

      // Check to see if there is a total cross section for this daughter:
      cs = GetDCs(parent, d_kzas[i], TOTAL_CS);

      if(cs)
	{
	  result = cs.Integrate(flux);
	}

    }
  //  Convert from barns:
  result *= 1E-24;

  // Add decay contribution:
  result += GetDecayConstant(parent);
}

void RamLib::LambdaEff(Kza parent, Kza daughter, 
			  const vector<double>& flux, double& result)
{
  int i;

  XSec cs = GetDCs(parent,daughter,TOTAL_CS);

  result = 0;

  if(cs)
    {
      result = cs.Integrate(flux);
    }

  result *= 1E-24;

  result += GetDecayConstant(parent)*GetBratio(parent,daughter);
}

double RamLib::LambdaEff(Kza parent, const std::vector<double>& flux,
			 vector<Kza>& daughters, vector<double>& dLambdas,
			 double& secLambda, FissionType ft) 
  throw(ExXsecSize, ExEmptyXSec)
{
  double decay_constant = GetDecayConstant(parent);

  double sfbr = GetSfbr(parent);
  double sfyield = 0.0;
  double fission_sigphi = 0.0;
  XSec fission_cs = GetPCs(parent, NEUTRON_FISSION_CS);

  // The return value (primary lambda effective)
  double pri_lambda = decay_constant;
  
  double total_lambda = 0;

  // Reference to total cross-section
  XSec total_cs = GetPCs(parent, TOTAL_CS);

  // Get the list of daughter kzas
  daughters = Daughters(parent);

  // Dimension the daughter lambda vector appropriately
  dLambdas.assign(daughters.size(), 0.0);

  int i,j;

  // Calculate the primary lambda effective:
  if(total_cs)
    {

      try{ pri_lambda += total_cs.Integrate(flux); }
      catch(ExXsecSize& ex) {
	stringstream ss;
	ss << "\nError occurred in LambdaEff(...), user supplied a flux that did not match the"
	   << "\ntotal cross-section group structure loaded in the RamLib!"
	   << "\nError occured for parent isotope: " << parent << endl;
	ex.AddToDetailed(ss.str());
	throw;
      }
    }
  // Calculate lambda effective for each daughter:
  
  // Check to see if we should precalculate sigma*phi for fission:
  if(fission_cs)
    {
      try{ fission_sigphi += fission_cs.Integrate(flux); }
      catch(ExXsecSize& ex) {
	stringstream ss;
        ss << "\nError occurred in LambdaEff(...), user supplied a flux that did not match the"
	   << "\nfission cross-section group structure loaded in the RamLib!"
	   << "\nError occured for parent isotope: " << parent << endl;
	ex.AddToDetailed(ss.str());
	throw;
      }
    }

  if( fission_cs || sfbr )
    {
      // Add the fission daughter to the daughter list...

      if(ft == NO_FISSION) total_lambda += fission_sigphi;

      daughters.push_back(FISSION_DAUGHTER);
      dLambdas.push_back(fission_sigphi + sfbr);
    }

  for(i = 0; i < daughters.size(); i++)
    {
      XSec cs = GetDCs(parent, daughters[i], TOTAL_CS);

      // Make sure the cross-section exists:
      if(cs)
	{
	  try{ dLambdas[i] += cs.Integrate(flux); }
	  catch(ExXsecSize& ex) {
	    stringstream ss;
	    ss << "\nError occurred in LambdaEff(...), user supplied a flux that did not match the"
	       << "\nparent daughter cross-section group structure loaded in the RamLib!"
	       << "\nError occured for parent " << parent
	       << " and daughter " << dLambdas[i] << endl;
	    ex.AddToDetailed(ss.str());
	    throw;
	  }
	}

      dLambdas[i] += GetBratio(parent, daughters[i])*decay_constant;

      // Make sure to handle spontaneous fission, which is NOT included in the
      // normal decay branching ratios..
      if(sfbr > 0.0 && ft != NO_FISSION &&
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
  map<Kza,vector<Kza> >::iterator iter = Adjoint.find(daughter);
  if(iter == Adjoint.end()) return EMPTY_VEC_KZA;

  return iter->second;
}

double RamLib::ProdRates(Kza daughter, const vector<double>& flux,
			 vector<Kza>& parents, vector<double>& pLambdas, 
			 FissionType ft)
  throw(ExXsecSize, ExEmptyXSec)
{
  double decay_constant;
  
  // The total destruction rate, return value:
  double pri_lambda = GetDecayConstant(daughter);
  
  int i,j;
  
  XSec total_cs = GetPCs(daughter, TOTAL_CS);
  
  double fission_yield;
  double b_ratio;
  double sfbr;
  double sigphi = 0;

  // Calculate pri_lambda:

  if(total_cs)
    {
      try{ pri_lambda += total_cs.Integrate(flux); }
      catch(ExXsecSize &ex) {
	stringstream ss;
	ss << "\nError occurred in ProdRate(...), user supplied a flux that did not match the"
	   << "\ntotal cross-section group structure loaded in the RamLib!"
	   << "\nError occured for daughter " << daughter << endl;
	ex.AddToDetailed(ss.str());
	throw;
      }

    }

  parents = Parents(daughter);
  
  // Initialize the production rate vector:

  pLambdas.assign(parents.size(), 0.0);

  // Check to see if we should precalculate sigma*phi for fission:
  XSec fission_cs = GetPCs(daughter, NEUTRON_FISSION_CS);

  if(fission_cs)
    {
      try{ pri_lambda += fission_cs.Integrate(flux); }
      catch(ExXsecSize &ex) {
	stringstream ss;
	ss << "\nError occurred in ProdRate(...), user supplied a flux that did not match the"
	   << "\nfission cross-section group structure loaded in the RamLib!"
	   << "\nError occured for daughter " << daughter << endl;
	ex.AddToDetailed(ss.str());
	throw;
      }
    }

   for(i = 0; i < parents.size(); i++)
     {
       XSec cs = GetDCs(parents[i], daughter, TOTAL_CS);

       if(cs)
 	{
	  pLambdas[i] += cs.Integrate(flux);
 	}

       // Now, check for fission...
       XSec f_cs = GetDCs(parents[i], daughter, NEUTRON_FISSION_CS);
       
       if(f_cs && ft != NO_FISSION)
	 {
	   // Get the yield...
	   fission_yield = GetFissionYield(parents[i], daughter, ft);

	   if(fission_yield)
	     {
	       pLambdas[i] += f_cs.Integrate(flux)*fission_yield;
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

void RamLib::GetFissionYield(Kza parent, vector<Kza>& daughters, 
			     vector<double>& yields, FissionType ft)
{
  daughters = Daughters(parent);
  yields.assign(daughters.size(), 0.0);

  for(unsigned i = 0; i < daughters.size(); i++)
    {
      yields[i] = GetFissionYield(parent, daughters[i], ft);
    }
}
