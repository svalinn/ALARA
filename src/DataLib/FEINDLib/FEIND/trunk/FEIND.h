#ifndef __FEIND_H
#define __FEIND_H

/** \file FEIND.h
 *  \brief This is the only file that should be included by applications
 *         seeking to use FEIND.
 *
 *  By including this file in their source code, user applications can access
 *  the primary FEIND namespace and all of the associated objects. FEIND.h was
 *  created to make it easier to use FEIND.
 */

#include "FeindNs.h"
#include "LibDefine.h"
#include "RamLib.h"
#include "XSec.h"
#include "exception/ExInclude.h"

/** \mainpage
 *  \page mainpage FEIND - The Fast Easy Interface to Nuclear Data
 *  <CENTER> Written by Milad Fatenejad &copy; 2005 </CENTER>
 * 
 *  \section what What is FEIND?
 *  FEIND is a library designed to help activation codes gain access to
 *  nuclear data. User applications can link to FEIND and instruct it to load
 *  various types of data and store it in memory. User programs can then either
 *  request the data directly from FEIND, or can ask FEIND to perform some
 *  useful calculations. The purpose is to use this library to remove redundant
 *  code from user applications.
 *
 *  \section datatypes What kinds of nuclear data does FEIND load?
 *  This library is capable of loading several types of data, including:
 *    - Transmutation Cross-Sections - These cross-sections can be dependent on
 *      the parent, the daughter, or on the specific emitted particles which
 *      produce the daughter. These cross-sections do not include scattering
 *      and are not dependent on the projectile energy. At present, FEIND only
 *      supports group-wise cross-sections.
 *    - Decay Data - This includes decay constants for isotopes, decay 
 *      energies, decay spectra (continuous and group-wise), and branching
 *      ratios.
 *    - Fission Yields - There can be multiple sets of fission yields for each
 *      fissionable isotope, depending on the flux shape. For example, there 
 *      may be seperate yields corresponding to fast and thermal fluxes.
 *
 *  \section formats What nuclear data formats does FEIND support?
 *  FEIND is capable of loading nuclear data from four different formats.
 *    - EAF 4.1 - Used to store transmutation cross-sections.
 *    - ENDF VI Decay - Used to store decay data.
 *    - ENDF VI Transmutation - Used to store transmutation cross-sections.
 *    - CINDER - Contains transmutation cross-sections, decay data and fission
 *      yields.
 */
#endif
