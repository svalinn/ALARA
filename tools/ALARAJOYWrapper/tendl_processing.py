# Import packages
from pathlib import Path
from reaction_data import GAS_DF
from njoy_tools import elements
from collections import defaultdict
import warnings
import numpy as np
from io import StringIO
from openmc.data import Reaction, endf

EXCITATION_DICT = {
    4   : np.arange(51 ,  92), # (n,n*)  reactions
    103 : np.arange(600, 650), # (n,p*)  reactions
    104 : np.arange(650, 700), # (n,d*)  reactions
    105 : np.arange(700, 750), # (n,t*)  reactions
    106 : np.arange(750, 800), # (n,h*)  reactions
    107 : np.arange(800, 850), # (n,a*)  reactions
    16  : np.arange(875, 892), # (n,2n*) reactions
}
EXCITATION_REACTIONS = set(np.concatenate(list(EXCITATION_DICT.values())))
REVERSE_EXCITATION_DICT = {
    val: key
    for key, arr in EXCITATION_DICT.items()
    for val in arr
}
ISOMERIC_STATES = 'mnopqrstuvwxyz'
PATH_SPECIFIC_MFS = (9,10)

def compile_MTs_from_ENDF_obj(endf_obj):
    """
    Collect all reaction types (MTs) contained in the MF3 
        ("Reaction Cross Sections") file of an `openmc.data.endf.Evaluation()`
        parsed ENDF-formatted file.

    Arguments:
        endf_obj (openmc.data.endf.Evaluation): OpenMC ENDF file parsing
            object.
    
    Returns:
        MTs (set): Set of all MT reaction numbers contained in the ENDF-
            formatted file.
    """

    return {MT for (MF, MT) in endf_obj.section if MF == 3}

def calculate_KZA_from_ENDF(filepath):
    """
    By parsing a single nuclide ENDF-formatted file with
        `openmc.data.endf.Evaluation()`, produce the nuclide's unique KZA
        (ZZAAAM) identifier.

    Arguments:
        filepath (pathlib._local.PosixPath): Path to an ENDF-formatted file.

    Returns:
        KZA (int): Unique ZZZAAAM identifier for a given nuclide.
    """

    nuc_data = endf.Evaluation(filepath).target

    return (
        (nuc_data['atomic_number'] * 1000 + nuc_data['mass_number']) * 10
        + nuc_data['isomeric_state']
    )

def interpret_KZA(kza):
    """
    Infer the chemical symbol and mass number from a KZA (ZZAAAM) number.

    Arguments:
        kza (int): Unique ZZZAAAM for a given nuclide.

    Returns:
        element (str): Chemical symbol of the target nuclide.
        A (str): Mass number for selected isotope.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.
    """

    M = kza % 10
    za = kza // 10
    A = str(za % 1000)
    Z = za // 1000
    element = list(elements.keys())[Z - 1]
    if M > 0:
        A += ISOMERIC_STATES[M - 1]

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
            for a given nuclide. These dictionaries are formatted as such:
                {
                    'Element'     : Nuclide's chemical symbol,
                    'Mass Number' : Nuclide's mass number,
                    'pKZA'        : Nuclide's KZA value,
                    'File Paths'  : endf_path, pendf_path
                }
    """

    file_info = []
    for suffix in ['tendl', 'endf']:
        # Iterate alphabetically for debugging to spot where process fails
        for file in sorted(dir.glob(f'*.{suffix}')):
            pKZA = calculate_KZA_from_ENDF(file)
            element, A = interpret_KZA(pKZA)

            for existing_file in file_info:
                if existing_file['pKZA'] == pKZA:
                    warnings.warn(
                        f'Multiple files present in {dir} for ' \
                        f'{element}-{A}.'
                    )
                    break

            else:
                new_filename = dir / f'{element}{A}.tendl'
                if new_filename != file:
                    file.rename(new_filename)
                    warnings.warn(
                        f'Improperly named TENDL file {file} renamed to ' \
                        f'{new_filename} to match nuclide ID.'
                    )

                file_info.append({
                    'Element'         :            element,
                    'Mass Number'     :                  A,
                    'pKZA'            :               pKZA,
                    'TENDL File Path' :       new_filename
                })

    return file_info

def collect_excitation_pathways(endf_obj, MT, single_MF=None):
    """
    For a given excitation-nonspecific nuclear reaction, organize all distinct
        excitation pathways into a list of tuples, each containing the
        isomeric state and the `openmc.data.product.Product` object containing
        the nuclear data for the pathway to each product in ascending order of
        excitation.

    Arguments:
        endf_obj (openmc.data.endf.Evaluation): OpenMC parsed-ENDF object.

    Returns:
        pathways (list): List of tuples, each containing the isomeric state
            and the `openmc.data.Tabulated1D` object containing the reaction's
            TAB1 data.
        matched_MF (int): ENDF file number corresponding to the MF containing
            excitation pathway data.
    """

    pathways = []
    matched_MF = None
    MF_search = [single_MF] if single_MF else PATH_SPECIFIC_MFS
    for MF in MF_search:
        section_text = endf_obj.section.get((MF, MT))
        if not section_text:
            continue

        io_obj = StringIO(section_text)
        head_record = endf.get_head_record(io_obj)
        n_states = head_record[4]

        for _ in range(n_states):
            tab1_params, tab1 = endf.get_tab1_record(io_obj)
            LFS = tab1_params[3]
            pathways.append((LFS, tab1))

        if pathways:
            matched_MF = MF
            break

    return  sorted(pathways, key=lambda pathway: pathway[0]), matched_MF

def determine_all_excitations(endf_obj, MTs):
    """
    Reference an ENDF file's MF9 and MF10 file data and explicitly defined
        excitation reactions to construct a nested dictionary keyed by
        reaction type (MT) and then file type (MF) containing lists of all 
        possible isomeric states of residual daughters produced from each
        reaction type with cross-section data in the TENDL file. Reactions
        without explicit isomeric pathways continue to reference MF3 general
        cross-section data.

    Arguments:
        endf_obj (openmc.data.endf.Evaluation): OpenMC parsed-ENDF object.
        MTs (set): Set of all MT reaction numbers contained in the TENDL file.

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
        cumulative_MT = REVERSE_EXCITATION_DICT.get(MT)
        if MT not in EXCITATION_REACTIONS:
            # Isomer pathways contained either in MF 9 ("Multiplicities for
            # Production of Radioactive Nuclides") and MF 10 ("Cross Sections
            # for Production of Radioactive Nuclides").
            MF = next((
                MF for MF in PATH_SPECIFIC_MFS if (MF, MT) in endf_obj.section
            ), None)

            if MF:
                pathways, _ = collect_excitation_pathways(
                    endf_obj, MT, MF
                )
                isomer_dict[MT][MF].extend([p[0] for p in pathways])

            if not isomer_dict[MT]:
                isomer_dict[MT][3].append(0)

        # Account for cases of explicit MF3 excitation reactions without
        # corresponding cumulative MTs (i.e. (n,a0) [MT=800] for C-13)
        elif cumulative_MT is not None and cumulative_MT not in MTs:
            M = np.where(EXCITATION_DICT[cumulative_MT] == MT)[0][0]
            # MT = 50 is forbidden in ENDF6, so scattering reactions require
            # an index adjustment compared to other state-explcit reactions
            if cumulative_MT == 4:
                M += 1
            isomer_dict[MT][3].append(M)

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

def extract_gendf_data(gendf_path):
    """
    Parse a GENDF-formatted (post-GROUPR processing) file for lines containing
        cross-section data for each reaction type, while compiling a full set
        of all MT numbers corresponding to that reaction.

    Arguments
        gendf_path (pathlib._local.PosixPath): Path to the GENDF file from
             which to extract cross-section data.

    Returns:
        non_zero_xs (collections.defaultdict): Dictionary keyed by MT number
            valued by lists of sub-dictionaries. Each of these are keyed by
                the GROUPR group index tag (`IG`) and valued by the associated
                groupwise cross-section value for that energy group.
         MTs (set of ints): All reaction types with cross-section data in the
            provided GENDF.
        nGroups (int): Number of energy groups into which the groupwise cross
            sections were calculated.
    """

    MTs = set()
    non_zero_xs = defaultdict(list)

    current_MT = None
    current_section = {}
    current_IG = None
    line_count = 0
    nGroups = None

    with open(gendf_path, 'r') as f:
        for line in f:
            mf, mt = _gendf_parse_control(line)
            
            # Extract number of groups, found in second line of file
            # description section
            if mf == 1 and mt == 451 and int(line.rstrip('\n')[-3:]) == 2:
               nGroups = int(line.split()[2])

            if mf == 3 and mt != 0:
                MTs.add(mt)
                if mt != current_MT:
                    if current_section:
                        non_zero_xs[current_MT].append(current_section)
                    
                    current_section = {}
                    current_MT = mt
                    line_count = 0

                line_count += 1

                if line_count < 2:
                    continue

                if line_count % 2 == 0:
                    current_IG = int(line[62:66])

                else:
                    current_section[current_IG] = float(
                        line.split()[1].replace('+','E+').replace('-','E-')
                    )

            else:
                if current_section:
                    non_zero_xs[current_MT].append(current_section)
                    current_section = {}

                current_MT = None
                current_IG = None
                line_count = 0

    if nGroups is None:
        raise ValueError(
            f'{gendf_path} misformatted. Expecting to find group number in ' \
            'MF1, MT451.'
        )

    return non_zero_xs, MTs, nGroups

def populate_xs(xs_by_index, M_values, nGroups):
    """
    Given a dictionary containing all cross-section data for a given reaction
        type, identify all excitation pathways and save group-positioned 
        cross-section arrays to a dictionary keyed by isomeric states (M).
        Each reaction type may have multiple actual reaction pathways,
        corresponding to different isomeric states of the daughter, which
        appear within the file in ascending order of excitation.
    
    Arguments:
        xs_by_index (dict): Dictionary of all non-zero cross-sections for a
            given MT value keyed by the GROUPR group index tag (`IG`) and
            valued by the groupwise cross-section in barns.
        M_values (list of int): All possible isomeric states of the residual
            daughter produced from the given reaction type.
        nGroups (int): Number of energy groups into which the groupwise cross
            sections were calculated.
    
    Returns:
        sigma_dict (dict): Dictionary keyed by excitation levels (M) with
            values of padded NumPy arrays containing groupwise cross-sections
            following the group structure provided by the user or the default
            Vitamin-J 175 group structure if none is otherwise specified.
    """

    sigma_dict = {}
    excitations = len(M_values)
    N = min(excitations, len(xs_by_index))

    if excitations > 1:
        M_values = list(range(excitations))

    for M, ordered_xs in zip(M_values[:N], xs_by_index[:N]):
        sigmas = np.zeros(nGroups)

        for IG, sigma in ordered_xs.items():
            sigmas[IG - 1] = sigma

        sigma_dict[M] = sigmas[::-1]

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
    while (dKZA + trial_M) not in eaf_nucs and trial_M > 0:
        trial_M -= 1

    return dKZA + trial_M

def iterate_MTs(
    MTs, mt_dict, non_zero_xs, pKZA, all_rxns, all_nucs, isomer_dict, nGroups
):
    """
    Iterate through all of the MTs present in a given GENDF file to extract
        the necessary data to be able to run ALARA. For isomeric daughters
        with an excited state less than 10 that do not have known half-lives
        (determined by the keys of radionucs, itself derived from the provided
        decay library), this function assumes a infinitesimal half-life with
        discrete deexcitations to the next lowest isomeric state, until 
        reaching a level with a known half-life or the ground state. As such, the cross-sections 
        for these isomer daughters are accumulated to the appropriate isomeric 
        state cross-sections by energy group.
    
    Arguments:
        MTs (list of int): List of reaction types present in the GENDF file.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv
        non_zero_xs (collections.defaultdict): Dictionary keyed by MT number
            valued by lists of sub-dictionaries. Each of these are keyed by
                the GROUPR group index tag (`IG`) and valued by the associated
                groupwise cross-section value for that energy group.        
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
        all_nucs (dict): Dictionary keyed by all nuclide KZAs in the decay
            library, with values of their half-lives (-1 for stable nuclides).
         isomer_dict (collections.defaultdict): Dictionary keyed by reaction
            type (MT), with each MT containing a subdictionary of the MF from
            which the isomeric pathways are extracted. At the lowest MT/MF
            level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
        nGroups (int): Number of energy groups into which the groupwise cross
            sections were calculated.
            
    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    filtered_MTs = MTs - EXCITATION_REACTIONS
    for MT in MTs:
        cumulative_MT = REVERSE_EXCITATION_DICT.get(MT)
        if cumulative_MT is not None and cumulative_MT not in MTs:
            filtered_MTs.add(MT)

    for MT in filtered_MTs:
        xs_by_index = non_zero_xs[MT]
        M_values = list(isomer_dict[MT].values())[0]
        rxn = mt_dict[MT]
        gas = rxn['gas']
        
        # Calculate dKZA values for each excitation pathway in MF10
        for M, sigmas in populate_xs(xs_by_index, M_values, nGroups).items():
            emitted = rxn['emitted']
            dKZA = (((pKZA // 10) * 10 + rxn['delKZA']) // 10) * 10 + M
            if gas:
                dKZA = GAS_DF.loc[GAS_DF['gas'] == gas, 'kza'].iat[0]

            # Remove ENDF isomer tags for explicit excitation reactions
            if emitted[-1].isdigit() or emitted[-1] == 'c':
                emitted = emitted[:-1]

            # Signify isomeric state in ALARA formatting with a "*" for
            # each excited level  
            if M > 0:
                emitted += '*'

            if dKZA in all_nucs:
                all_rxns[pKZA][dKZA][str(MT) + '*' * M] = {
                    'emitted'    :  emitted,
                    'xsections'  :  sigmas
                }

            else:
                dKZA = ((dKZA - M) // 10) * 10
                special_MT = -1
                
                if M > 1:
                    dKZA = incrementally_deexcite_isomer(
                        M, dKZA, all_nucs
                    )

                # Skip daughters without decay data
                if dKZA not in all_nucs:
                    continue

                if dKZA not in all_rxns[pKZA]:
                    all_rxns[pKZA][dKZA] = defaultdict(dict)

                if special_MT not in all_rxns[pKZA][dKZA]:
                    all_rxns[pKZA][dKZA][special_MT] = {
                        'emitted'  : emitted,
                        'xsections': np.zeros(nGroups)
                    }

                all_rxns[pKZA][dKZA][special_MT]['xsections'] += sigmas

    return all_rxns