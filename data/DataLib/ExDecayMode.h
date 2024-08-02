#ifndef EXDECAYMODE_H
#define EXDECAYMODE_H

#include "FeindNs.h"
#include "Exception.h"

#include <string>

/// Exception derived class for handling unknown DecayModeType's
/** This exception is thrown by the FEIND::DecayModetoKza function.
 */
class FEIND::ExDecayMode : public Exception
{
public:

  /// The main constructor for this object.
  /** \param[in] loc
   *  A string identifying the location where the error occurred.
   *
   *  \param[in] mode
   *  The invalid decay mode that was encountered
   */
  ExDecayMode(const std::string& loc, DecayModeType mode);
};

#endif
