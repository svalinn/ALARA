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

EXCITATION_DICT = {
    4   : [4] + list(range(51,   92)), # (n,n*) reactions
    103 : range(600, 650), # (n,p*) reactions
    104 : range(650, 700), # (n,d*) reactions
    105 : range(700, 750), # (n,t*) reactions
    106 : range(750, 800), # (n,h*) reactions
    107 : range(800, 850), # (n,a*) reactions
    108 : range(875, 892)  # (n,2n*) reactions
}

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

def create_endf_file_obj(path, MF):
    """
    For a TENDL (ENDF) file containing activation data for a single nuclide,
        store a single MF's "file" data in an ENDFtk file object.

    Arguments:
        path (str): Filepath to an ENDF file.
        MF (int): ENDF file number.

    Returns:
        file (ENDFtk.tree.File): Single MF ENDFtk file object.
        matb (int): Unique material ID extracted from the file.
    """

    file = None
    tape = ENDFtk.tree.Tape.from_file(str(path))
    matb = tape.material_numbers[0]
    material = tape.material(matb)
    if MF in [MF.MF for MF in material.files]:
        file = material.file(MF)

    return file, matb

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

    # Set MF for cross-sections
    xs_MF=3
    file, matb = create_endf_file_obj(path, xs_MF)
    
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
        sigma_list (array): All of the cross-sections for a given reaction type
            and material, listed as floating point numbers and padded up to
            175 entries corresponding to the VITAMIN-J group structure.
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

    sigmas = sigma_list[::-1]
    return np.pad(sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas)))

def determine_daughter_excitation(
    endf_path, MT, pKZA, delKZA, excitation_pathways
):
    """
    Reference File 10 ("Cross Sections for Production of Radioactive
        Nuclides") of a nuclide's original TENDL (ENDF) file to identify
        possible isomeric states of a given reaction's daughter nuclide.
        Store all reactions that can produce isomers in the dictionary
        excitation_pathways as reference for subsequent MTs of higher
        excitation.

    Arguments:
        endf_path (pathlib._local.PosixPath): Path to the parent nuclide's
            original TENDL (ENDF) file.
        MT (int): Numerical identifier for the given reaction.
        pKZA (int): KZA identifier of the parent nuclide.
        delKZA (int): Change in KZA from the given reaction.
        excitation_pathways (dict): Dictionary keyed by parent KZA values
            containing sub-dictionaries keyed by MT reactions that can produce
            isomeric daughters, with values of the excitation level of said
            daughters.
        
    Returns:
        M (int): Isomeric level of the resultant daughter.
        excitation_pathways (dict): Updated excitation_pathways dictionary,
            potentially with new parent/reaction excitation data.
    """

    # Reference existing excitation_pathways dict; if a reaction is already
    # accounted for, return its reference excitation
    if pKZA in excitation_pathways:
        if MT in excitation_pathways[pKZA]:
            return excitation_pathways[pKZA][MT], excitation_pathways
        else:
            return 0, excitation_pathways
    
    # Parse File 10 for allowed daughter excitations from a given reaction
    excitation_pathways[pKZA] = {}

    za = f'{(pKZA // 10) * 10 + delKZA}'[:-1]
    mf10 = 10
    file10, _ = create_endf_file_obj(endf_path, mf10)
    excitation_levels = []
    if file10 and MT in [MT.MT for MT in file10.sections]:
        section = file10.section(MT)
    else:
        return 0, excitation_pathways
    
    for line in section.content.split('\n'):
        if za in line:
            excitation_levels.append(int(
                line.split(za)[1].strip().split(' ')[0]
            ))

    if MT in EXCITATION_DICT:
        for iso in excitation_levels:
            excitation_pathways[pKZA][EXCITATION_DICT[MT][iso]] = iso
    else:
        excitation_pathways[pKZA][MT] = 0

    return excitation_pathways[pKZA][MT], excitation_pathways

def iterate_MTs(MTs, file_obj, mt_dict, pKZA, all_rxns, eaf_nucs, endf_path):
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
        eaf_nucs (dict): Dictionary keyed by all nuclides in the EAF decay
            library, with values of their half-lives (zeros for stable
            nuclides).
        endf_path (pathlib._local.PosixPath): Path to the parent nuclide's
            original TENDL (ENDF) file.
            
    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    excitation_pathways = {}
    for MT in MTs:
        rxn = mt_dict[MT]

        sigmas = extract_cross_sections(file_obj, MT)
        gas = rxn['gas']
        delKZA = rxn['delKZA']
        emitted = rxn['emitted']

        if gas:
            dKZA = GAS_DF.loc[GAS_DF['gas'] == gas, 'kza'].iat[0]

        else:
            M, excitation_pathways = determine_daughter_excitation(
                endf_path=endf_path,
                MT=MT,
                pKZA=pKZA,
                delKZA=delKZA,
                excitation_pathways=excitation_pathways
            )

            # Reset deLKZA for (n,n*) excitation reactions
            if delKZA >= 0 and MT in EXCITATION_REACTIONS:
                delKZA = 0

            # Flag reactions that produce an isomer
            if M > 0:
                emitted += '*'

            # Recalculate daughter KZA with explicit isomerism from MF10
            dKZA = (((pKZA // 10) * 10 + delKZA) // 10) * 10 + M

            # Subtract individual excitation reactions from cumulative sums
            if MT in excitation_pathways[pKZA] and MT in EXCITATION_DICT:
                for subMT in excitation_pathways[pKZA]:
                    if subMT != MT and subMT in MTs:
                        sigmas -= extract_cross_sections(file_obj, subMT)
                            
            # Nullify redundant excitation reactions
            elif MT in EXCITATION_REACTIONS and MT not in excitation_pathways[pKZA]:
                sigmas = np.zeros(VITAMIN_J_ENERGY_GROUPS)


        if pKZA == 491090 and emitted == 'n*':
            print(pKZA, dKZA, MT, M, dKZA - M)

        if dKZA in eaf_nucs and M < 10:
            all_rxns[pKZA][dKZA][MT] = {
                'emitted'    :  emitted,
                'xsections'  :  sigmas
            }

        else:
            # Force dKZA to ground; possible for an isomer reaction to exist
            # in TENDL data without having decay data in EAF
            dKZA = ((dKZA - M) // 10) * 10
            special_MT = -1

            if dKZA not in all_rxns[pKZA]:
                all_rxns[pKZA][dKZA] = defaultdict(dict)

            if special_MT not in all_rxns[pKZA][dKZA]:
                all_rxns[pKZA][dKZA][special_MT] = {
                    'emitted'     :   emitted,
                    'xsections'   :   np.zeros(VITAMIN_J_ENERGY_GROUPS)
                }

            all_rxns[pKZA][dKZA][special_MT]['xsections'] += np.pad(
                sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas))
            )

    return all_rxns