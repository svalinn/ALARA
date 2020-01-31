#ifndef XSEC_H
#define XSEC_H

#include "FeindNs.h"
#include "ExInclude.h"

#include <vector>

/// CS is the class for storing cross-sections in FEIND.
/** Presently, it simply implements a smart point/reference counting system
 *  which can be used to prevent duplication of data in the RamLib. This will
 *  also allow pointers that are used to be exception safe.\n\n
 *
 *  Eventually, the CS class will implement member functions for performing
 *  useful tasks with cross-sections...
 */
class FEIND::XSec
{
public:

  /// Default constructor to create an empty cross-section
  /** Most operations performed on an empty cross-section will throw exceptions
   */
  XSec();

  /// Copy constructor
  /** Copies a cross-section, and increases the reference counter
   */
  XSec(const XSec& cs);  

  /// Constructor to create a XSec object with numGroups energy groups
  /** \param[in] numGroups
   *  The number of energy groups to create
   *
   *  \param[in] initValue
   *  The initial value for the data. Defaults to zero.
   */
  XSec(unsigned numGroups, double initValue = 0.0);

  /// Delete the contents of the cross-section and recreate it
  /** \param numGroups
   *  The number of groups the new cross-section should have.
   *
   *  \param initValue
   *  Initial value to use for the new cross-section
   */
  void Reset(unsigned numGroups, double initValue = 0.0);

  const XSec& operator=(const XSec& cs);

  double& operator[](unsigned i) throw(ExEmptyXSec);
  const double& operator[](unsigned i) const throw(ExEmptyXSec);

  /// Returns the number of groups in a XSec
  /** If the XSec object has not been initialized, this returns zero.
   */
  unsigned NumGroups() const;

  /// Function to integrate the XSec object with another vector.
  /** \param mult
   *  The function which the XSec object should be multiplied with. This vector
   *  must match the groups structure of the XSec object.
   *
   *  \return
   *  This function returns the result of the following calculation:
   *  \f[ return = \sum_{j=1}^{n_g} {\sigma_j f_j} \f]
   *  where\n
   *  \f$ n_g \f$ is the number of groups in the cross-section\n
   *  \f$ \sigma \f$ is the XSec object\n
   *  \f$ f \f$ is the function that is multiply the XSec object
   */
  double Integrate(const std::vector<double>& mult) const 
    throw(ExXsecSize, ExEmptyXSec);

  XSec& operator+=(const XSec& rhs) throw(ExXsecSize, ExEmptyXSec);
  XSec& operator*=(double mult) throw(ExEmptyXSec);

  /// boolean operator to test XSec objects in an if statement.
  /** This can be useful when users request XSec objects from the RamLib.
   *  This operator allows users to test these objects to make sure they exist
   *  before they are used.
   */
  operator bool() const;

  /// Insertion strema operator to easily display cross-sections.
  friend std::ostream& operator<<(std::ostream& os, XSec rhs);

  /// Destructor
  ~XSec();

private:

  /// Function to delete the current cross-section
  /** Any time a XSec is changed, a new PCs object must be created. This
   *  function will decrement the reference counter, and destroy the associated
   *  object if necessary.
   */
  void CleanUp();
  
  void Copy();

  /// Structure to store the reference counting information
  /** This structure stores the integer doing the counting along with a pointer
   *  to the vector storing the actual cross-section.
   */
  struct PCsCounter
  {
    /// Main constructor
    /** Creates a PCsCounter from a pointer to a vector.
     */
    PCsCounter(const std::vector<double>* p) :
      Count(1),
      P(p)
    {
    }

    /// The reference count
    int Count;

    /// Pointer to the actual cross-section data.
    const std::vector<double>* P;
  };

  /// PCsCounter for storing the reference counting info, and the cross-section
  PCsCounter* PCs;
};

#endif
