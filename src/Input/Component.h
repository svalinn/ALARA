/* $Id: Component.h,v 1.11 2002-12-07 17:48:06 fateneja Exp $ */
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
  static ifstream 
    /// This is a stream connected to the material description library as
    /// given in the input file.
    matLib;

  static ifstream 
    /// This a stream connected to the element library as given in the
    /// input file.
    eleLib;

  int
    /// This indicates the type of this component and is based on the
    /// definitions given below.
    type;

  double
    /// This is the density of this component.
    /** This number is input as a relative density factor,
        and is then converted to a mass density by multiplying by 
        the theoretical density (from eleLib of matLib). */
    density,
    
    /// This is the volume fraction of this component in the mixture.
    volFraction;
  
  char
    /// This is the name of this component, as input by the user.
    /** For materials, this must match an entry in the material library.
        For elements, this must be a 'modified' chemical symbol matching
          an entry in the element library.
        For isotopes (only valid for targets), this must be have the
          standard notation 'cc-ddd', where 'cc' is the chemical symbol
          and 'ddd' the mass number.
        For similar components, this must match an entry in the list
          of mixtures read from the input file. */
    *compName;

  Component
    /// This is the pointer to the next component in this mixture's list.
    *next;

  /// This function, called with reference to a Component object of type
  /// element, expands the element into a list of Root objects.
  /** For cross-referencing, it expects a pointer to the mixture and
      component which contain this particular element - it does not
      automatically use the 'this' component pointer, since it can be
      called through a temporary object (such as might be created in
      expandMat() while expanding a material).  It returns a pointer to
      an object of class Root, which serves as the head of a list. */
  Root* expandEle(Mixture*, Component*);

  /// This function, called with reference to a Component object of type
  /// material, expands the material into a list of Root objects.
  /** For cross-referencing, it expects a pointer to the mixture and
      component which contain this particular element - it uses the
      'this' component pointer since it will never be ambiguous.  It
      returns a pointer to an object of class Root, which serves as the
      head of a list. */
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
  /** This constructor creates a blank list head, when no arguments
      are given.  Otherwise, it sets the type, the name and the density.
      The 'next' element is initialized to NULL. */
  Component(int compType=COMP_HEAD, char* name=NULL,double dens=0, 
	    double volFrac=1);

  /// Copy constructor
  /** This constructor initializes 'type', 'density', and 'volFraction'
      and then creates and fills space for 'compName'. 'next' is NULL */
  Component(const Component&);

  /// Inline destructor destroys *entire* chain by deleting 'next'.
  /// Also deletes storage for 'compName'.
  ~Component()
    { delete compName; delete next; };
  
  /// Overloaded assignment operator
  /** The assignment operator is similar to the copy constructor, but it
      uses an already allocated object on the left hand side.  The
      correct implementation of this operator must ensure that
      previously allocated space is returned to the free store before
      allocating new space into which to copy the object. Note that
      'next' is NOT copied, the left hand side object will continue to
      be part of the same list unless explicitly changed. */
  Component& operator=(const Component&);

  /// This function is called with reference to the last component in
  /// the list, and points its 'next' at a new object read from the
  /// input file.
  /** It expects an integer type, detemined by the calling function, 
      and a reference to the input file's stream.  It returns
      pointer to the new object of class Component which has just been
      read. */
  Component* getComponent(int,istream&,Mixture*);

  /// This function replaces the Component object of type 'similar'
  /// through which it is called with the component list of the mixture
  /// to which it is similar.
  /** It expects a list of components, through a pointer to the head 
      of the list.  Rather than deleting the current component, it is
      changed to a copy of the first object, and the others in the list
      are inserted as new objects.  This is used to replace all 'similar'
      components before a mixture is expanded. */
  Component* replaceSim(Component*);

  /// This function is used to expand a full list of Component objects
  /// into a list of Root objects.
  /** It is always called through the head of the Component list for a 
      given mixture.  For cross-referencing, a pointer to that mixture 
      is expected as an argument. */
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
  /** It is used to primarily to search for Components of type 'similar'.
      It expects an integer argument giving the component type of 
      interest. */
  Component* exists(int);

  /// This inline function provides access to the 'next' pointer.
  Component* advance() { return next; };

};

#endif

