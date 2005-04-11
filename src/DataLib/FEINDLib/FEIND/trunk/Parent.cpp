#include "Parent.h"

using namespace std;
using namespace FEIND;

Parent::Parent() :
  DecayConstant(0.0),
  Sfbr(0.0)
{

}

void ContSpec::Clear()
{
  Point.clear();
  IntMethod = 0;
}

Daughter::Daughter() :
  BranchingRatio(0.0)
{

}
