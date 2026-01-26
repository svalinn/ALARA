# Import packages
import ENDFtk
from pathlib import Path
from reaction_data import GAS_DF
from collections import defaultdict
import warnings
import numpy as np

VITAMIN_J_ENERGY_GROUPS = 175
EXCITATION_REACTIONS = np.concatenate((
    np.arange(51 ,  92), # (n,n*)  reactions
    np.arange(601, 650), # (n,p*)  reactions
    np.arange(651, 700), # (n,d*)  reactions
    np.arange(700, 750), # (n,t*)  reactions
    np.arange(750, 800), # (n,h*)  reactions
    np.arange(800, 850), # (n,a*)  reactions
    np.arange(875, 892), # (n,2n*) reactions
))

def get_isotope(stem):
    """
    Extract the element name and mass number from a given filename.
    Arguments:
        stem (str): Stem of a an ENDF (TENDL) and/or PENDF file, formatted
            as f'{element}{mass_number}.ext'

    Returns:
        element (str): Chemical symbol of the isotope whose data is contained
            in the file.
        A (str): Mass number of the isotope, potentially including the letter
            "m" at the end if the isotope is in a metastable state.
    """

    for i, char in enumerate(stem):
        if char.isdigit():
            break

    element = stem[:i]
    A = stem[i:]       

    return element, A

def search_for_files(dir = Path.cwd()):
    """
    Search through a directory for all pairs of ENDF (TENDL) and PENDF files
        that have matching stems. If so, save the paths and the isotopic
        information to a dictionary.
    
    Arguments:
        directory (pathlib._local.PosixPath, optional): Path to the directory
            in which to search for ENDF and PENDF files.
            Defaults to the present working directory (".").

    Returns:
        file_info (dict): Dictionary containing the chemical symbol, mass
            number, and paths to the ENDF and PENDF files for a given isotope.
            The dictionary is formatted as such:
            {f'{element}{mass_number}' :
                                    {'Element'} : Isotope's chemical symbol,
                                    {'Mass Number'} : Isotope's mass number,
                                    {'File Paths'} : (endf_path, pendf_path)
            }
    """

    file_info = {}
    for suffix in ['tendl', 'endf']:
        # Iterate alphabetically for debugging to spot where process fails
        for file in sorted(dir.glob(f'*.{suffix}')):
            element, A = get_isotope(file.stem)
            file_info[f'{element}{A}'] = {
                'Element'         :                             element,
                'Mass Number'     :                                   A,
                'TENDL File Path' :                                file
            }

    return file_info

def extract_endf_specs(path):
    """
    Extract the material ID and MT numbers from an ENDF file.
    Arguments:
        path (pathlib._local.PosixPath): File path to the selected ENDF file.
    
    Returns:
        matb (int): Unique material ID extracted from the file.
        MTs (list): List of reaction types (MT's) present in the file.
        file (ENDFtk.tree.File or None): ENDFtk file object containing the
            contents for a specific material's cross-section data.
            Only returns the file for GENDF filetypes.
    """

    tape = ENDFtk.tree.Tape.from_file(str(path))
    matb = tape.material_numbers[0]
    # Set MF for cross sections
    xs_MF = 3
    try:
        file = tape.material(matb).file(xs_MF)
    except RuntimeError:
        file = None
    
    if file:
        # Extract the MT numbers that are present in the file
        MTs = [MT.MT for MT in file.sections.to_list()]

        return (matb, MTs, file)
    
    else:
        return (matb, None, None)

def extract_gendf_pkza(gendf_path):
    """
    Read in and parse the contents of a GENDF file to construct the parent
        KZA. KZA values are defined as ZZAAAM, where ZZ is the isotope's
        atomic number, AAA is the mass number, and M is the isomeric state
        (0 if non-isomeric).
    
    Arguments:
        gendf_path (pathlib._local.PosixPath): File path to the GENDF file
            being analyzed.
        dir (str): String identifying the directory from which the function is
            being called.
    
    Returns:
        pKZA (int): Parent KZA identifier.
    """

    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    Z, element, A = first_line.split('-')[:3]

    Z = int(Z)
    A = A.split(' ')[0]

    # Metastable states classified by TENDL as m = 1, n = 2, etc.
    # (Generally expecting only m, occasionally n, but physically,
    # values could go higher, so isomeric_states goes up to z = 14)
    M = 0
    isomeric_states = 'mnopqrstuvwxyz'
    isomer_tag = next(
        (tag for tag in isomeric_states if tag in A.lower()), None
    )
    if isomer_tag:
        M = isomeric_states.find(isomer_tag) + 1
    if M > 2:
        warnings.warn(
            f'Isomeric state greater than 2. Unexpected case for TENDL2017.',
            UserWarning
        )
    A = int(A.lower().split(' ')[0].split(isomer_tag)[0])
    pKZA = (Z * 1000 + A) * 10 + M
    return pKZA

def extract_cross_sections(file, MT):
    """
    Parse through the contents of a GENDF file section to extract the
        cross-section data for a specific reaction type (MT).
    
    Arguments:
        file (ENDFtk.tree.File): ENDFtk file object containing a specific
            material's cross-section data.
        MT (int): Numerical identifier for the reaction type corresponding to
            the file's sectional organization.
    
    Returns:
        sigma_list (list): All of the cross-sections for a given reaction type
            and material, listed as floating point numbers. If the run fails,
            the function will just return an empty list.
    """

    section = file.section(MT).content

    # Only every 2nd line starting at the 3rd line has cross-section data.
    lines = section.split('\n')[2:-2:2]

    # Extract the 3rd token and convert to more conventional string
    # representation of a float
    sigma_list = [
        float(line.split(' ')[2].replace('+','E+').replace('-','E-'))
        for line in lines
    ]

    return sigma_list[::-1]

def _is_ground_state_daughter(M):
    """
    Determine if a given daughter is in its ground state.
     
    One of two internal Boolean methods to determine whether to create a new
        reaction entry in all_rxns for a given parent, daughter, MT
        combination.

    Arguments:
        M (int): Isomeric state of the given nuclide.
    
    Returns:
        is_ground_state (bool): True if the daughter nuclide is in its ground
            state, False if excited.
    """

    return M == 0

def _is_isomer_with_decay_data(dKZA, pKZA, radionucs, M):
    """
    Determine if a nuclear isomer has a known half-life (determined from 
        parsing of an EAF decay library) and whose excited state is less than
        10. The cut-off at the 9th excited state is necessary because double-
        digit Ms could alter a KZA to represent a different element (i.e. the
        10th excited state of Li-6 would have a KZA of 300610, which by the
        KZA formatting convention would actually represent Zn-61, a 
        radionuclide with decay data in the EAF-2010 decay library).

    One of two internal Boolean methods to determine whether to create a new
        reaction entry in all_rxns for a given parent, daughter, MT
        combination.

    Arguments:
        dKZA (int): Daughter KZA identifier.
        pKZA (int): Parent KZA identifier.
        radionucs (dict): Dictionary keyed by all radionuclides in the EAF
            decay library, with values of their half-lives.
        M (int): Isomeric state of the given nuclide.

    Returns:
        has_known_decay (bool): True if the isomer is in an excited state less
            than 10 and has a known half-life.
    """

    return dKZA in radionucs and ((pKZA % 10) + M in range(1,10))

def iterate_MTs(MTs, file_obj, mt_dict, pKZA, all_rxns, radionucs, to_ground):
    """
    Iterate through all of the MTs present in a given GENDF file to extract
        the necessary data to be able to run ALARA. For isomeric daughters
        with an excited state less than 10 that do not have known half-lives
        (determined by the keys of radionucs, itself derived from the provided
        EAF decay library), this function assumes a infinitesimal half-life
        decaying to the ground state. As such, the cross-sections for these
        isomer daughters are accumulated to the ground state cross-sections by
        energy group.
    
    Arguments:
        MTs (list of int): List of reaction types present in the GENDF file.
        file_obj (ENDFtk.tree.File): ENDFtk file object containing the
            contents for a specific material's cross-section data.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv
        pKZA (int): Parent KZA identifier.
        all_rxns (collections.defaultdict): Hierarchical dictionary keyed by
            parent nuclides to store all reaction data, with structured as:
            {parent:
                {daughter:
                    {MT:
                        {
                            'emitted': (str of emitted particles)
                            'xsections': (array of groupwise XS)
                        }
                    }
                }    
            }
        radionucs (dict): Dictionary keyed by all radionuclides in the EAF
            decay library, with values of their half-lives.
        to_ground (bool): 
            
    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    for MT in MTs:
        rxn = mt_dict[MT]

        # Modify parent M value if it is an isomer and the reaction pathway
        # does not specify a specific excitation level of the daughter nuclide
        parent_excitation = int(str(pKZA)[-1])
        if parent_excitation > 0 and MT not in EXCITATION_REACTIONS:
            rxn['isomer'] += parent_excitation

        sigmas = extract_cross_sections(file_obj, MT)
        sigmas = np.pad(sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas)))
        gas = rxn['gas']

        # Daughter calculated either as an emitted gas nucleus or
        # as the residual for non-gaseous emissions.
        dKZA = (
            GAS_DF.loc[GAS_DF['gas'] == gas, 'kza'].iat[0] if gas
            else pKZA + rxn['delKZA']
        )

        # Process all reactions producing isomer daughters with decay data
        # or any ground-state daughters. Necessarily need to cut off maximum
        # excitation at 9th state by nature of KZA conventions
        if (
            _is_ground_state_daughter(rxn['isomer'], dKZA) or
            _is_isomer_with_decay_data(dKZA, pKZA, radionucs, rxn['isomer'])
        ):
            all_rxns[pKZA][dKZA][MT] = {
                'emitted'    :  rxn['emitted'],
                'xsections'  :  sigmas
            }

        # If a reaction produces an isomer lacking decay data, acccumulate its
        # cross-sections either to the ground-state residual reaction's cross-
        # sections or to a new psuedo-daughter of all isomers with undefined
        # decays for that parent
        else:
            if to_ground:
                decay_KZA = (dKZA // 10) * 10
                special_MT = -1

            else:
                decay_KZA = f'{dKZA // 10}*'
                special_MT = -4

            if decay_KZA not in all_rxns[pKZA]:
                all_rxns[pKZA][dKZA] = defaultdict(dict)

            if special_MT not in all_rxns[pKZA][decay_KZA]:
                all_rxns[pKZA][decay_KZA][special_MT] = {
                    'emitted'     : f'{rxn['emitted']}*',
                    'xsections'   :   np.zeros(VITAMIN_J_ENERGY_GROUPS)
                }

            all_rxns[pKZA][decay_KZA][special_MT]['xsections'] += np.pad(
                sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas))
            )

    return all_rxns