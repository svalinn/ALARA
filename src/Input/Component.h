/* $Id: Component.h,v 1.7 1999-11-19 23:00:46 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class is invoked as a linked list, with each object of class
Mixture having a distinct list.  A full list describes all the
components of that mixture.  The first element in each list has type
COMP_HEAD, and contains no problem data.

 *** Static Class Members ***

 matLib : ifstream
    This a stream connected to the material description library as
    given in the input file.

 eleLib : ifstream
    This a stream connected to the element library as given in the
    input file.

 *** Class Members ***

 type : int
    This indicates the type of this component and is based on the
    definitions given below.

 density : double
    This is the density of this component.  It is either a number
    density or a mass density - determined by its type and/or its
    format (*** TBD ***)

 compName : char*
    This is the name of this component, as input by the user.
    For materials, this must match an entry in the material library.
    For elements, this must be a chemical symbol matching an entry
       in the element library.
    For isotopes, this must be have the standard notation 'cc-ddd',
       where 'cc' is the chemical symbol and 'ddd' the mass number.
    For similar components, this must match an entry in the list
       of mixtures read from the input file.

 next : Component*
    This is the pointer to the next component in this mixture's list.
 
 *** Protected Member Functions ***

 * - Preproc - *

 protected Root* expandEle(Mixture*, Component*) 
    This function, called with reference to a Component object of type
    element, expands the element into a list of Root objects.  For
    cross-referencing, it expects a pointer to the mixture and
    component which contain this particular element - it does not
    automatically use the 'this' component pointer, since it can be
    called through a temporart object (such as might be created in
    expandMat() while expanding a material).  It returns a pointer to
    an object of class Root, which serves as the head of a list.

 protected Root* expandMat(Mixture*); 
    This function, called with reference to a Component object of type
    material, expands the material into a list of Root objects.  For
    cross-referencing, it expects a pointer to the mixture and
    component which contain this particular element - it uses the
    'this' component pointer since it will never be ambiguous.  It
    returns a pointer to an object of class Root, which serves as the
    head of a list.

 *** Static Member Functions ***

 void getMatLib(istream&)
    This function expects the reference to the open input file stream
    and opens the material library, attaching it to the static class
    member 'matLib' described above.

 void getEleLib(istream&)
    This function expects the reference to the open input file stream
    and opens the element library, attaching it to the static class
    member 'eleLib' described above.

 *** Member Functions ***

 * - Constructors & Destructors - *

 Component(int, char*, double)
    Default constructor creates a blank list head, when no arguments
    are given.  Otherwise, it sets the type, the name and the density.
    The 'next' element is initialized to NULL.

 Component(const Component&)
    Copy constructor initializes 'type' and 'density' and then creates
    and fills space for 'compName'. 'next' is NULL

 ~Component() 
    Inline destructor destroys *entire* chain by deleting 'next'.
    Also deletes storage for 'compName'.

 * - Input - *

 Component* getComponent(int,istream&)
    This function is called with reference to the last component in
    the list, and points its 'next' at a new object read from the
    input file.  It expects an integer type, detemined by the calling
    function, and a reference to the input file's stream.  It returns
    pointer to the new object of class Component which has just been
    read.

 * - Preproc - *

 Component* replaceSim(Component*)
    This function replaces the Component object of type 'similar'
    through which it is called with the component list of the mixture
    to which it is similar.  It expects a list of components, through
    a pointer to the head of the list.  Rather than deleting the
    current component, it is changed to a copy of the first object,
    and the others in the list are inserted as new objects.  This is
    used to replace all 'similar' components before a mixture is
    expanded.

 Root* expand(Mixture*)
    This function is used to expand a full list of Component objects
    into a list of Root objects.  It is always called through the head
    of the Component list for a given mixture.  For cross-referencing,
    a pointer to that mixture is expected as an argument.


 * - Utility - *

 int head()
    Inline function to return the boolean result of the equality of
    this Component's type to the type which indicates the head of the
    list.

 char *getName()
    Inline function providing access to the name of the component.

 int getCompNum(Component*)
    Given a pointer to a Component object, this function returns an
    ordinal number for that object in the current list.

 Component* exists(int)
    This function provides a pointer to the first object of a given
    type in a Component list.  It is used to primarily to search for
    Components of type 'similar'.  It expects an integer argument
    giving the component type of interest.

 Component* advance()
    This inline function provides access to the 'next' pointer.

 */

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

#define AVAGADRO 6.02e23

class Component
{
protected:
  static ifstream matLib;
  static ifstream eleLib;

  int type;
  double density, volFraction;
  char *compName;

  Component *next;

  /* Preproc */
  Root* expandEle(Mixture*, Component*);
  Root* expandMat(Mixture*);
  
public:
  /* Input */
  static void getMatLib(istream&);
  static void getEleLib(istream&);

  /* service */
  Component(int compType=COMP_HEAD, char* name=NULL,double dens=0, 
	    double volFrac=1);
  Component(const Component&);
  ~Component()
    { delete compName; delete next; };

  Component& operator=(const Component&);

  /* Input */
  Component* getComponent(int,istream&,Mixture*);

  /* Preproc */
  Component* replaceSim(Component*);
  Root* expand(Mixture*);

  /* Utility */
  int head() {return (type == COMP_HEAD);};
  char *getName() {return compName;};
  double getVolFrac() {return volFraction;};
  double getDensity() {return density;};
  int getCompNum(Component*);
  Component* exists(int);
  Component* advance() { return next; };

};

#endif

