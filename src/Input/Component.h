/* $Id: Component.h,v 1.14 2003-01-13 04:34:54 fateneja Exp $ */
#include "alara.h"

#ifndef _COMPONENT_H
#define _COMPONENT_H

/* component types */
#define COMP_HEAD   0
#define COMP_MAT    1
#define COMP_ELE    2
//#define COMP_ISO    3
#define COMP_SIM    4

#define TARGET_ELE 12
#define TARGET_ISO 13

/** \brief This class is invoked as a linked list, with each object of 
 *         class Mixture having a distinct list.
 *
 *  A full list describes all the components of that mixture.  The 
 *  first element in each list has type COMP_HEAD, and contains no problem 
 *  data.
 */

class Component
{
 public:
  
  /// This is a stream connected to the material description library as
  /// given in the input file.
  static ifstream matLib;
  
  /// This a stream connected to the element library as given in the
  /// input file.
  static ifstream eleLib;
  
  /// This indicates the type of this component and is based on the
  /// definitions given below.
  int type;
  
  /// This is the density of this component.
  /** This number is input as a relative density factor,
      and is then converted to a mass density by multiplying by 
      the theoretical density (from eleLib of matLib). */
  double density;
  
  /// This is the volume fraction of this component in the mixture.
  double volFraction;
  
  /// This is the name of this component, as input by the user.
  /** For materials, this must match an entry in the material library.
      For elements, this must be a 'modified' chemical symbol matching
        an entry in the element library.
      For isotopes (only valid for targets), this must be have the
        standard notation 'cc-ddd', where 'cc' is the chemical symbol
        and 'ddd' the mass number.
      For similar components, this must match an entry in the list
        of mixtures read from the input file. */
  char *compName;
  
  /// This is the pointer to the next component in this mixture's list.
  Component* next;
  
  /// This function, called with reference to a Component object of type
  /// element, expands the element into a list of Root objects.
  Root* expandEle(Mixture*, Component*);

  /// This function, called with reference to a Component object of type
  /// material, expands the material into a list of Root objects.
  Root* expandMat(Mixture*);
  
public:
  /// This function expects the reference to the open input file stream
  /// and reads the name of the material library, opens it, and attaches
  /// it to the static class member 'matLib' described above.
  static void getMatLib(istream&);

  /// This function expects the reference to the open input file stream
  /// and reads the name of the element library, opens it, and attaches
  /// it to the static class member 'eleLib' described above.
  static void getEleLib(istream&);

  /// Default constructor
  Component(int compType=COMP_HEAD, char* name=NULL,double dens=0, 
	    double volFrac=1);

  /// Copy constructor
  Component(const Component&);

  /// Inline destructor destroys *entire* chain by deleting 'next'.
  /// Also deletes storage for 'compName'.
  ~Component()
    { delete compName; delete next; };
  
  /// Overloaded assignment operator
  Component& operator=(const Component&);

  /// This function is called with reference to the last component in
  /// the list, and points its 'next' at a new object read from the
  /// input file.
  Component* getComponent(int,istream&,Mixture*);

  /// This function replaces the Component object of type 'similar'
  /// through which it is called with the component list of the mixture
  /// to which it is similar.
  Component* replaceSim(Component*);

  /// This function is used to expand a full list of Component objects
  /// into a list of Root objects.
  Root* expand(Mixture*);

  /// Inline function to return the boolean result of the equality of
  /// this Component's type to the type which indicates the head of the
  /// list.
  int head() {return (type == COMP_HEAD);};

  /// Inline function providing access to the name of the component.
  char *getName() {return compName;};

  /// Inline function providing access to the volume fraction of this
  /// component.
  double getVolFrac() {return volFraction;};

  /// Inline function providing access to the mass density of this
  /// component.
  double getDensity() {return density;};

  /// Given a pointer to a Component object, this function returns an
  /// ordinal number for that object in the current list.
  int getCompNum(Component*);

  /// This function provides a pointer to the first object of a given
  /// type in a Component list.
  Component* exists(int);

  /// This inline function provides access to the 'next' pointer.
  Component* advance() { return next; };

};

#endif

