#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "../FeindNs.h"
#include "../Consts.h"

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

/// The main FEIND exception class
/** Functions should not throw objects of this type. Instead, they should use
 *  one of the derived exception objects, which are tailored for specific
 *  errors. Users can use this class to easily catch any FEIND exception. Once
 *  exception objects are caught, the Print() and Abort() functions can be used
 *  to print error messages and exit.
 */
class FEIND::Exception
{
 public:
  
  /// Print the error message associated with this exception
  /** The print function displays a header which contains information about
   *  where the error occurred. Additionally, it prints a detailed error
   *  message which contains information about the problem.
   */
  void Print() const;

  /// Accessor function for the ErrorCode member
  inline FEINDErrorType GetErrorCode() const;

  /// Add argument str to member Detailed
  /** This function allows users of the exception class to add to the detailed
   *  description of an error, if the default detailed description is not
   *  adequate.
   *
   *  \param[in] str
   *  String to be apended to the detailed error description
   */
  inline void AddToDetailed(const std::string& str);
  
  /// Print error message and abort program with ErrorCode
  /** Users that simply want to print the error message and exit the program
   *  should use this function. Otherwise, they can handle the exception and
   *  continue. If a user wants to print the error message without aborting,
   *  the Print() member function can be used.
   */
  void Abort() const;

 protected:

  /// Constructor for setting error codes
  /** \param[in] loc
   *  String identifying the location where the error occurred. This string
   *  is used to set the Location member of FEIND::Exception.
   *
   *  \param[in] ec
   *  The error code of the exception. This code is eventually used to set the
   *  ErrorCode member of this class.
   */
  Exception(const std::string& loc, FEINDErrorType ec);
  
  /// The location string contains information about where the error occured
  /** This usually means specifying the function, class.
   */
  std::string Location;
  
  /// This string provides detailed information about the error
  std::string Detailed;

  /// An enum that is used to store error codes.
  /** This member exists to simplify writing a fortran interface around FEIND.
   *  It is also used as the exit value when the Abort() function is called.
   */
  FEINDErrorType ErrorCode;
};

inline FEIND::FEINDErrorType FEIND::Exception::GetErrorCode() const
{
  return ErrorCode;
}

inline void FEIND::Exception::AddToDetailed(const std::string& str)
{
  Detailed += str;
}

#endif
