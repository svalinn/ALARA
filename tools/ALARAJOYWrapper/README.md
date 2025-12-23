# FENDL3 Preprocessor with NJOY Wrapping for ALARA (*ALARAJOYWrapper*)

This preprocessor is designed to update ALARA's data input capabilities for updated FENDL3.2x data sets formatted as TENDL (ENDF-6) files. Unlike previous versions of [FENDL](https://www-nds.iaea.org/fendl_library/websites/fendl32b/) (Fusion Evaluated Nuclear Data Library), which have available groupwise cross-section data for neutron activation, as is required for ALARA's functionality, FENDL3 data requires conversion to that format.

This preprocessor uses [NJOY 2016](https://github.com/njoy/NJOY2016) Nuclear Data Processing System to produce requisite pointwise data (PENDF) to subsequntly convert to the Vitamin-J 175 energy group groupwise format (GENDF) for activation cross-sections and to handle the processed data to be fed back to ALARA. For pointwise conversion, ALARAJOYWrapper uses the NJOY modules MODER, RECONR, BROADR, UNRESR, and GASPR, and for the ultimate conversion to the groupwise format, GROUPR.

## Dependencies

- Standard Python libraries
  * [ArgParse](https://docs.python.org/3/library/argparse.html)
  * [Collections](https://docs.python.org/3/library/collections.html)
  * [Csv](https://docs.python.org/3/library/csv.html)
  * [Pathlib](https://docs.python.org/3/library/pathlib.html)
  * [Pytest](https://docs.pytest.org/en/stable/getting-started.html)
  * [Re](https://docs.python.org/3/library/re.html)
  * [String](https://docs.python.org/3/library/string.html#module-string)
  * [Subprocess](https://docs.python.org/3/library/subprocess.html)
  * [Warnings](https://docs.python.org/3/library/warnings.html)
- Generic Python packages
  * [NumPy](https://numpy.org/install/)
  * [Pandas](https://pandas.pydata.org/docs/getting_started/install.html)
- Domain-specific packages
  * [ENDFtk](https://github.com/njoy/ENDFtk)
  * [NJOY 2016](https://github.com/njoy/NJOY2016)


## Usage
This preprocessor can and should be used independently of any ALARA run. While the data format produced from ALARAJOYWrapper is necessary to run `convert_lib` on ALARA to convert TENDL data to an ALARA binary, once the user has a space-delimited DSV file produced from this preprocessor, usage of this preprocessor is not strictly necessary for the particular isotopes processed by the user.

ALARAJOYWrapper is designed to produce a space-delimited DSV containing data that can be directly read by the ALARA data library, ALARAJOY, without any further processing.

To run this preprocessor, the user must first have acquired TENDL files for each isotope to be processed from [TENDL 2017](https://tendl.web.psi.ch/tendl_2017/tendl2017.html), which is the source for FENDL3.2x neutron activation data. All TENDL files to be processed must be in the same directory as each other to be properly identified by ALARAJOYWrapper.

Running ALARAJOYWrapper can be done with one Python command:

    python preprocess_fendl3.py -g gas_handling_method -f /path/to/fendl3_data_dir/ -d /path/to/eaf_decay_library/ -a

This command takes five arguments,`-g `, `-f`, `-d`, `-i`, and `-a`. Of these, `-g`, `-f`, and `-d` are required. The `-g` argument sets the gas handling method for gas totals, as calculated by the NJOY GASPR module. The two possible methods are `r` (remove) and `s` (subtract). Method `r` removes all reactions with a light gas daughter (protons through alpha particles), with the exception of total gas production cross-sections corresponding to MT = 203-207 reactions (see Appendix B of the ENDF-6 Manual at `../../developer-info/endf6-manual.pdf`) when a total gas production cross-section exists. Method `s` subtracts the cross-sections for each energy group of reactions that produce a gas daughter from the total gas production cross-sections instead. In effect, for gas producing reactions, selecting `r` will show only the *total* gas production cross sections for each gas, whereas `s` will show each *individual* gas production pathway with its respective cross section data.

The `-f` argument directs the program to the directory containing the TENDL files. This argument is optional, and if left blank, will default to the current working directory.

The `-d` argument directs the program to an EAF decay library necessary for cross-referencing short-lived isomeric daughters against known half-life data.'

The `-i` argument is an optional flag to amalgamate all isomers lacking decay data to an "Other" daughter with a KZA isomer value of `*`, instead of forcing their decay to their respective ground states.

The `-a` argument is an optional flag to amalgamate all like-daughter nuclides of a given parent into a single row of the resultant DSV file by adding the groupwise cross-section data for all reaction pathways that produce said daughter from each parent.


## Data Output
Running `preprocess_fendl3.py` will return the file path to the resultant space-delimited DSV file containing transmutation reaction pathways for the neutron activation of the given isotope(s). Each row in the DSV represents a different reaction, and contains the following data needed by ALARA for a library conversion:

- Parent KZA: Unique isotope identifier for the parent isotope in the format **ZZAAAM**, where ZZ is the isotope's atomic number, AAA is the mass number, and M is the isomeric state (0 if non-excited).
- Daughter KZA: Unique isotope identifier of the daughter isotope produced from a particular transmutation reaction in the format **ZZAAAM**.
- Emitted Particles: Consecutive string of particles emitted from the transmutation reaction. The possible particles are:
  - Neutron (n)
  - Gamma photon (g)
  - Proton (p)
  - Deuteron (d)
  - Triton (t)
  - <sup>3</sup>He nucleus (h)
  - Alpha particle (a)

  For reactions with multiple emitted particles, the format of the text string is along the lines '3n2p', representing an emission of three neutrons and two protons. An emission containing only one of a certain particle will just contain the particle identifier, without any numerical tag (e.g. 'n' for a single neutron).

- Non-Zero Groups: Count of total number of energy groups in the Vitamin-J 175 energy group structure with non-zero neutron cross sections.
- Cross Sections: List of all non-zero neutron cross sections.

It should be noted that TENDL 2017 activation data contains many (n,n) reactions that leave the residual nucleus in various quantized excited states, potentially up to a 40<sup>th</sup> excited state. Most of these daughter isomers, however, are extremely short-lived and lack decay data. When converting an ALARAJOY DSV file in ALARA, as discussed below, an EAF decay library is required, given that FENDL3.2x only contains activation data. Cross-referencing exotic isomers against this decay data ensures that ultra-short-lived reaction products are not passed on to ALARA's library conversion appearing as stable nuclides. Rather, the lack of decay data signifies such a short half-life that decay evaluations are not practical. As such, as these isomers are processed, ALARAJOYWrapper defaults to assume that they decay to the ground state and their cross-sections are accumulated for the ground state (n,n) reaction (MT = 4). To avoid making this assumption and instead to accumulate all of these reactions for each parent into an "Other" row, use the optional flag `-i` when running ALARAJOYWrapper (see above.)

## Application of Processed Data to ALARA Data Conversion Methods
Data library conversion to ALARA binary libraries is done with the `convert_lib` input block in the ALARA input file. Converting preprocessed TENDL data contained in the resultant space-delimited DSV from ALARAJOYWrapper is done as such in the input file:

    convert_lib ajoylib alaralib transFname decayFname alaraFname
wherein: 
- `ajoylib` is the library value for ALARAJOY
- `transFname` is the file path to the DSV containing the preprocessed FENDL3 transmutation data
- `decayFname` is the file path to EAF decay data.
- `alaraFname` is the file path template for resultant ALARA data libraries (.lib, .idx. .gam, .gdx)

The ALARA run for library conversion should precede and be independent from subsequent activation calculations with ALARA, as specified in the [ALARA Users' Guide](https://svalinn.github.io/ALARA/usersguide/index.html):

> *This input block is used to convert library formats. If this input block is included, ALARA will stop immediately after converting the library (ie. it should not be used as part of a normal ALARA input file).*