# Import packages
import ENDFtk
from pathlib import Path
from reaction_data import GAS_DF
from collections import defaultdict
import warnings
import numpy as np

VITAMIN_J_ENERGY_GROUPS = 175
EXCITATION_REACTIONS = set(np.concatenate((
    np.arange(51 ,  92), # (n,n*)  reactions
    np.arange(601, 650), # (n,p*)  reactions
    np.arange(651, 700), # (n,d*)  reactions
    np.arange(700, 750), # (n,t*)  reactions
    np.arange(750, 800), # (n,h*)  reactions
    np.arange(800, 850), # (n,a*)  reactions
    np.arange(875, 892), # (n,2n*) reactions
)))

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
        file_info (list of dicts): List of dictionaries containing the
            chemical symbol, mass number, and paths to the TENDL (ENDF) files
            for a given isotope. These dictionaries are formatted as such:
                {
                    'Element'     : Isotope's chemical symbol,
                    'Mass Number' : Isotope's mass number,
                    'File Paths'  : endf_path, pendf_path
                }
    """

    file_info = []
    for suffix in ['tendl', 'endf']:
        # Iterate alphabetically for debugging to spot where process fails
        for file in sorted(dir.glob(f'*.{suffix}')):
            element, A = get_isotope(file.stem)
            for existing_file in file_info:
                if (
                    existing_file['Element'] == element
                    and existing_file['Mass Number'] == A
                ):
                    warnings.warn(
                        f'Multiple files present in {dir} for {element}-{A}'
                    )
                    break

            else:
                file_info.append({
                    'Element'         :         element,
                    'Mass Number'     :               A,
                    'TENDL File Path' :            file
                })

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
    """

    # Set MF for cross-sections
    xs_MF=3
    file, matb = create_endf_file_obj(path, xs_MF)
    
    if file:
        # Extract the MT numbers that are present in the file
        MTs = [MT.MT for MT in file.sections.to_list()]

        return (matb, MTs)
    
    else:
        return (matb, None)

def determine_all_excitations(endf_path, MTs, pKZA, mt_dict):
    """
    Reference an ENDF file's MF9 and MF10 file data and explicitly defined
        excitation reactions to construct a nested dictionary keyed by
        reaction type (MT) and then file type (MF) containing lists of all 
        possible isomeric states of residual daughters produced from each
        reaction type with cross-section data in the TENDL file. Reactions
        without explicit isomeric pathways continue to reference MF3 general
        cross-section data.

    Arguments:
        endf_path (pathlib._local.PosixPath): Path to the ENDF (TENDL) file to
            be processed.
        MTs (set): Set of all MT reaction numbers contained in the TENDL file.
        pKZA (int): KZA identifier of the TENDL file's single nuclide.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.

    Returns:
         isomer_dict (collections.defaultdict): Dictionary keyed by reaction
            type (MT), with each MT containing a subdictionary of the MF from
            which the isomeric pathways are extracted. At the lowest MT/MF
            level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
    """

    isomer_dict = defaultdict(lambda: defaultdict(list))

    for MT in MTs:
        if MT not in EXCITATION_REACTIONS:
            za = f'{(pKZA // 10) * 10 + mt_dict[MT]['delKZA']}'[:-1]

            # Isomer pathways contained either in MF 9 ("Multiplicities for
            # Production of Radioactive Nuclides") and MF 10 ("Cross Sections
            # for Production of Radioactive Nuclides").
            for MF in [9, 10]:
                file, _ = create_endf_file_obj(endf_path, MF)
                if file and MT in [MT.MT for MT in file.sections]:
                    section = file.section(MT)
                    for line in section.content.split('\n'):
                        if f' {za} ' in line:
                            # Line formatted with ZA cushioned by whitespace
                            # on both sides
                            isomer_dict[MT][MF].append(int(
                                line.split(za)[1].strip().split(' ')[0]
                            ))

            if not isomer_dict[MT]:
                isomer_dict[MT][3].append(0)

    return isomer_dict

def _gendf_parse_control(line):
    """
    Extract the integer values of the MF (file) and MT (reaction) numbers from
        a given line of an ENDF-formatted file. By ENDF formatting
        conventions, these values are always in a fixed position, but may
        contain additional whitespace.
    
    Arguments:
        line (str): Text of a whole line of an ENDF-formatted file.
    
    Returns:
        MF (int or None): ENDF file number. None if the file is incorrectly
            formatted.
        MT (int): Reaction number. None if the file is incorrectly formatted.
    """

    if len(line) < 75:
        return None, None
    
    try:
        return (
            int(line[70:72]), # MF
            int(line[72:75])  # MT
        )
    except ValueError:
        return None, None

def get_gendf_MTs(gendf_path):
    """
    Parse a GENDF file to find all unique MT reaction numbers. Necessary to
        be called after running GROUPR as there may be some MTs present in the
        reference ENDF/PENDF files used to produced the GENDF file that may
        not be present in the final GENDF file.

    Arguments:
        gendf_path (pathlib._local.PosixPath): Path to the GENDF file from
            which to extract cross-section data.

    Returns:
        MTs (set): Set of all unique MTs contained in the GENDF file.    
    """

    MTs = set()
    with open(gendf_path, 'r') as f:
        for line in f:
            mf, mt = _gendf_parse_control(line)
            if mf == 3 and mt != 0:
                MTs.add(mt)

    return MTs

def extract_gendf_xs_lines(gendf_path, MTs):
    """
    Parse a GENDF-formatted (post-GROUPR processing) file for lines containing
        cross-section data for each reaction type in the provided set of MTs.

    Arguments
         gendf_path (pathlib._local.PosixPath): Path to the GENDF file from
             which to extract cross-section data.
         MTs (set of ints): All reaction types with cross-section data in the
            provided GENDF.

    Returns:
        xs_line_dict (collections.defaultdict): Dictionary keyed by MT number
            with values of lists of all line strings in the MT-section from
            the parsed GENDF file containing cross-section data.
    """
    
    xs_line_dict = defaultdict(list)
    current_section_lines = []
    current_MT = None
    line_count = 0

    with open(gendf_path, 'r') as f:
        for line in f:
            mf, mt = _gendf_parse_control(line)
            if mf == 3 and mt in MTs:
                if mt != current_MT:
                    if current_section_lines:
                        xs_line_dict[current_MT].append(current_section_lines)
                        current_section_lines = []
                    current_MT = mt
                    line_count = 0

                line_count += 1

                if line_count >= 3 and line_count % 2 == 1:
                    current_section_lines.append(line)

            else:
                if current_section_lines:
                    xs_line_dict[current_MT].append(current_section_lines)
                    current_section_lines = []
                current_MT = None
                line_count = 0

    return xs_line_dict

def process_xsections(xs_lines, MT, M_values):
    """
    Given a list of lines containing all cross-section data for a given
        reaction type, identify all excitation pathways and save reformatted
        and padded cross-section arrays to a dictionary keyed by isomeric
        states (M). Each reaction type may have multiple actual reaction
        pathways, corresponding to different isomeric states of the daughter,
        which appear within the file in ascending order of excitation.
    
    Arguments:
        xs_lines (line of str): List of all lines in the MT-section containing
            cross-section data.
        MT (int): Numerical identifier for the reaction type corresponding to
            the file's sectional organization.
        M_values (list of int): All possible isomeric states of the residual
            daughter produced from the given reaction type.
    
    Returns:
        sigma_dict (dict): Dictionary keyed by excitation levels (M) with
            values of padded NumPy arrays containing groupwise cross-sections
            following the VITAMIN-J group structure.
    """

    sigma_dict = {}
    excitations = len(M_values)
    N = min(excitations, len(xs_lines))
    
    # If multiple excitations, M values equal the list indices of LFS values
    if excitations > 1:
        M_values = list(range(excitations))

    for M, lines in zip(M_values[:N], xs_lines[:N]):
        sigmas = [
            float(line.split(' ')[2].replace('+','E+').replace('-','E-'))
            for line in lines
        ][::-1]
        sigma_dict[M] = np.pad(
            sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas))
        )
    
    return sigma_dict

def incrementally_deexcite_isomer(M, dKZA, eaf_nucs):
    """
    Lower an isomer's excitation to the next lowest value with known-decay
        data. Decrease incrementally by one, with a maximum possible
        excitation level of 9 by KZA conventions.

    Arguments:
        M (int): Excitation level of the given nuclide.
        dKZA (int): KZA signifier of the given nuclide.
        eaf_nucs (dict): Dictionary keyed by all radionuclides in the EAF
            decay library, with values of their half-lives.

    Returns:
        trial_KZA (int): KZA with reduced nuclear excitation to match nuclides
            that can be referenced with known-decay data from the provided EAF
            decay library.
    """

    trial_M = min(M-1, 9)
    while (dKZA + trial_M) not in eaf_nucs and trial_M >= 0:
        trial_M -= 1

    return dKZA + trial_M

def iterate_MTs(
    MTs, mt_dict, xs_line_dict, pKZA, all_rxns, eaf_nucs, isomer_dict, gendf_path
):
    """
    Iterate through all of the MTs present in a given GENDF file to extract
        the necessary data to be able to run ALARA. For isomeric daughters
        with an excited state less than 10 that do not have known half-lives
        (determined by the keys of radionucs, itself derived from the provided
        EAF decay library), this function assumes a infinitesimal half-life
        with discrete deexcitations to the next lowest isomeric state, until 
        reaching a level with a known half-life. As such, the cross-sections 
        for these isomer daughters are accumulated to the appropriate isomeric 
        state cross-sections by energy group.
    
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
        eaf_nucs (dict): Dictionary keyed by all radionuclides in the EAF
            decay library, with values of their half-lives.
         isomer_dict (collections.defaultdict): Dictionary keyed by reaction
            type (MT), with each MT containing a subdictionary of the MF from
            which the isomeric pathways are extracted. At the lowest MT/MF
            level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
        gendf_path (pathlib._local.PosixPath): Path to the GENDF file from
            which to extract groupwise cross-sections.
            
    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    # Avoid duplicate isomer cross-sections -- excitations already
    # covered by MF 9/10 cumulative reactions
    for MT in (MTs - EXCITATION_REACTIONS):
        xs_lines = xs_line_dict[MT]
        M_values = list(isomer_dict[MT].values())[0]
        isomer_specific_rxns = process_xsections(xs_lines, MT, M_values)
        rxn = mt_dict[MT]
        gas = rxn['gas']
        
        # Calculate dKZA values for each excitation pathway in MF10
        for M, sigmas in isomer_specific_rxns.items():
            emitted = rxn['emitted']
            dKZA = (((pKZA // 10) * 10 + rxn['delKZA']) // 10) * 10 + M
            if gas:
                dKZA = GAS_DF.loc[GAS_DF['gas'] == gas, 'kza'].iat[0]

            if M > 0:
                emitted += '*'

            if dKZA in eaf_nucs or M == 0:
                all_rxns[pKZA][dKZA][str(MT) + '*' * M] = {
                    'emitted'    :  emitted,
                    'xsections'  :  sigmas
                }

            else:
                dKZA = ((dKZA - M) // 10) * 10
                special_MT = -1
                
                if M > 1:
                    dKZA = incrementally_deexcite_isomer(M, dKZA, eaf_nucs)

                if dKZA not in all_rxns[pKZA]:
                    all_rxns[pKZA][dKZA] = defaultdict(dict)

                if special_MT not in all_rxns[pKZA][dKZA]:
                    all_rxns[pKZA][dKZA][special_MT] = {
                        'emitted'  : emitted,
                        'xsections': np.zeros(VITAMIN_J_ENERGY_GROUPS)
                    }

                all_rxns[pKZA][dKZA][special_MT][
                    'xsections'
                ] += np.pad(
                    sigmas, (0, VITAMIN_J_ENERGY_GROUPS - len(sigmas))
                )

    return all_rxns