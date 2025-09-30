# Workflow Examples

**Purpose:**

_These examples are practical use cases that illustrate the ALARA workflow for typical applications._

## Steps needed to perform a fusion activation analysis

* Identify incident particle type causing activation (assume neutron for this document)
* Decide on appropriate multi-group activation library (energy group structure matches nuclear system's neutr
on energy spectrum)
* Perform a neutron transport simulation to obtain neutron flux spectrum in multi-group energy format in the 
region of interest 
_(If one cannot perform a neutron transport simulation, one may need to convert a reference neutron flux spec
trum to a multi-group energy structure matching the activation library)_

* Prepare activation code input file:

1. Identify activation library desired
2. Describe materials (ultimately need isotopic description)
_(Most codes can take a mixture of elements and generate isotopic compositions for you)_
3. Provide pulse schedule
- Provide irradiation times
- Provide flux spectra, flux levels at those irradiation times
- Provide cooling times

4. Identify activation responses desired at the above time intervals

## List of Examples:

   * Example of irradiating a single element (Fe) for 2 years in a typical fusion reactor with DCLL blanket
     - [singleElement](singleElement.md)
