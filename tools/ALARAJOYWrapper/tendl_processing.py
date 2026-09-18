# Import packages
from pathlib import Path
from reaction_data import GAS_DF
from njoy_tools import elements
from collections import defaultdict, abc
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
        MT (int): Reaction number.
        single_MF (int or None, optional): Option to avoid iterating over both
            MF 9 and 10 if the file containing the excitation pathway data is
            already known.
            (Defaults to None)

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

    return sorted(pathways, key=lambda pathway: pathway[0]), matched_MF

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

def _section_to_group_array(section, nGroups):
    sigmas = np.zeros(nGroups)
    for IG, sigma in section.items():
        if IG <= nGroups:
            sigmas[IG - 1] = sigma

    return sigmas[::-1]

def populate_xs(gendf_dict, MT, nGroups, isomer_dict={}, prepro=False):
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
        isomer_dict (collections.defaultdict, optional): Dictionary keyed by
            reaction type (MT), with each MT containing a subdictionary of the
            MF from which the isomeric pathways are extracted. The lowest
            MT/MF level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
            (Defaults to {})
        prepro (bool, optional): Option to handle PREPRO/GROUPIE-processed
            groupwise nuclear data.
            (Defaults to False)
    
    Returns:
        sigma_dict (dict): Dictionary keyed by excitation levels (M) with
            values of padded NumPy arrays containing groupwise cross-sections
            following the group structure provided by the user or the default
            Vitamin-J 175 group structure if none is otherwise specified.
    """

    if MT in gendf_dict[10]['MTs']:
        sections = gendf_dict[10]['non_zero_xs'][MT]
        keys = list(sections)
        M_values = keys
        section_iter = [sections[key] for key in keys]

    else:
        section_iter = gendf_dict[3]['non_zero_xs'][MT]
        M_values = [0] if prepro else list(isomer_dict[MT].values())[0]
        section_iter = section_iter[:len(M_values)]

    if len(M_values) > 1:
        M_values = range(len(M_values))

    return {
        M: _section_to_group_array(section, nGroups)
        for M, section in zip(M_values, section_iter)
    }

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
    gendf_dict, mt_dict, pKZA, all_rxns, all_nucs, nGroups,
    isomer_dict={}, prepro=False
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
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.
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
        nGroups (int): Number of energy groups into which the groupwise cross
            sections were calculated.
        isomer_dict (collections.defaultdict, optional): Dictionary keyed by
            reaction type (MT), with each MT containing a subdictionary of the
            MF from which the isomeric pathways are extracted. The lowest
            MT/MF level has a list of all isomeric states of possible daughter
            nuclides for which there are cross-section data in the original
            TENDL file.
            (Defaults to {})
        prepro (bool, optional): Option to handle PREPRO/GROUPIE-processed
            groupwise nuclear data.
            (Defaults to False)

            
    Returns:
        all_rxns (collections.defaultdict): Updated dictionary for all
            reaction pathways for the given parent and its MTs.
    """

    MTs =  gendf_dict[3]['MTs']
    filtered_MTs = MTs - EXCITATION_REACTIONS
    for MT in MTs:
        cumulative_MT = REVERSE_EXCITATION_DICT.get(MT)
        if cumulative_MT is not None and cumulative_MT not in MTs:
            filtered_MTs.add(MT)

    for MT in filtered_MTs:
        rxn = mt_dict.get(MT)
        if not rxn:
            continue

        gas = rxn['gas']

        # Calculate dKZA values for each excitation pathway in MF9/10
        sigma_dict = populate_xs(
            gendf_dict, MT, nGroups, 
            isomer_dict=isomer_dict, prepro=prepro
        )
        for M, sigmas in sigma_dict.items():
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

class GENDFParser:

    def __init__(self, MFs=(3,), prepro=False):
        if not isinstance(MFs, abc.Iterable):
            MFs = [MFs]

        self.MFs = MFs
        self.prepro = prepro
        self.gendf_dict = None
        self.nGroups = None
        self.pKZA = None
        self._reset_section_state()
        self.current_MF = None
        self.current_MT = None

    @staticmethod
    def _parse_control(line):
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

    @staticmethod
    def _reformat_endf_float(num_str):
        """
        Convert a parsed ENDF numeric string to a standard Python-usable
            floating point number.

        Argument:
            num_str (str): Numeric string parsed from an ENDF file. Can be in
                plain decimal for (0.1), explicit E/D notation (1.0E-1,
                1.0D-1), or a Fortran shortand exponent with implicit 'E'
                (1.0-1).

        Returns:
            num_float (float or None): Converted numeric value as a floating
                point number, if one could be found in `num_str`. Otherwise,
                `None`.
        """

        v = num_str.strip()
        if not v:
            return None

        if 'E' in v or 'e' in v or 'D' in v or 'd' in v:
            v = v.replace('D', 'E').replace('d', 'e')
        else:
            for i in range(1, len(v)):
                if v[i] in '+-':
                    v = v[:i] + 'E' + v[i:]
                    break

        return float(v)

    @staticmethod
    def _parse_tab1_header(line, with_lfs=False):
        """
        Extract three integer parameters from an ENDF reaction header needed
            to identify the reaction's file section (or subsection for MF9/10
            specific excitation pathways). For further information on these
            TAB1 parameters, see the ENDF6 Manual
            (https://www.nndc.bnl.gov/endfdocs/ENDF-102-2023.pdf).

        Arguments:
            line (str): Text of a whole line of an ENDF-formatted file.
            with_lfs (bool, optional): Option to search for the LFS parameter.
                Only needed when parsing MF9/10 for reaction subsections for
                specific excitation pathways.
                (Defaults to False)
        
        Returns:
            LFS (int or None): Excited state of the daughter nuclide resultant
                from the given reaction/excitation pathway. `None` if
                `with_lfs` is `False`.
            NR (int): "Number of energy ranges. A different scheme may be
                given for each range" (ENDF6 Manual, Section 9.2).
            NP (int): "Total number of energy points used to specify the
                data" (ENDF6 Manual, Section 9.2).
        """

        LFS = int(line[33:44]) if with_lfs else None
        NR = int(line[44:55])
        NP = int(line[55:66])

        return LFS, NR, NP

    @classmethod
    def _parse_prepro_tab1_xs(cls, line):
        """
        For a given line in a PREPRO-formatted TAB1 data table (i.e. after the
            header), extract, reformat, and organize all cross sections into a
            list.

        Arguments:
            line (str): A single TAB1 data line.

        Returns:
            line_xs (list of float): List of all cross-section values parsed
                from the provided TAB1 data line.
        """

        return [
            v for v in (
                cls._reformat_endf_float(line[i : i + 11])
                for i in range(0, 66, 11)
            ) if v is not None
        ][1::2]

    def _reset_section_state(self):
        """
        Reset all state variables tracked within the section currently being
            parsed.

        Arguments:
            None

        Returns:
            None
        """

        self.current_section = {}
        self.current_LFS = None

        # Standard NJOY/GROUPR (IG) state
        self.line_count = 0
        self.current_IG = None

        # PREPRO/GROUPIE state (TAB1) state
        self.section_line_idx = 0
        self.NR = None
        self.NP = None
        self.interp_lines_left = 0
        self.points_collected = 0
        self.point_idx = 0

    def _save_current_section(self):
        """
        Store the currently accumulated section, if non-empty.

        Arguments:
            None

        Returns:
            None
        """

        if not self.current_section:
            return

        if self.current_MF == 10:
            self.gendf_dict[self.current_MF]['non_zero_xs'][self.current_MT][
                self.current_LFS
            ] = self.current_section

        else:
            self.gendf_dict[self.current_MF]['non_zero_xs'][
                self.current_MT
            ].append(self.current_section)

    def _handle_groupr_line(self, line):
        """
        Parse one line of a standard GROUPR IG-tagged MF section to save
            either the current IG (group-index) for even line counts or the
            associated energy-dependent cross section for odd line counts.

        Arguments:
            line (str): Text of a whole line of an ENDF-formatted file.

        Returns:
            None
        """

        self.line_count += 1
        if self.line_count < 2:
            return

        if self.line_count % 2 == 0:
            self.current_IG = int(line[62:66])
        else:
            self.current_section[self.current_IG] = self._reformat_endf_float(
                line.split()[1]
            )

    def _handle_prepro_line(self, line, MF):
        """
        Parse one line of a PREPRO/GROUPIE TAB1 section. Handles both single-
            TAB1-record MFs and MF10's multi-subsection (per-LFS) layout
            uniformly, by detecting new subsections via NP exhaustion rather
            than a fixed line index.

        Arguments:
            line (str): Text of a whole line of an ENDF-formatted file.
            MF (int): ENDF file number.

        Returns:
            None
        """

        self.section_line_idx += 1
        if self.section_line_idx == 1:
            return

        if self.NP is None or self.points_collected >= self.NP:
            self._save_current_section()
            self.current_section = {}
            self.current_LFS, self.NR, self.NP = self._parse_tab1_header(
                line, with_lfs=(MF == 10)
            )
            self.interp_lines_left = int(np.ceil(self.NR / 3))
            self.points_collected = 0
            self.point_idx = 0
            return

        if self.interp_lines_left > 0:
            self.interp_lines_left -= 1
            return

        for sigma in self._parse_prepro_tab1_xs(line):
            if self.points_collected >= self.NP:
                break

            self.point_idx += 1
            self.current_section[self.point_idx] = sigma
            self.points_collected += 1

    def parse(self, gendf_path):
        """
        Main externally callable function on a GENDFParser parser object to
            parse a GENDF file for all cross-sections for each MF/MT pair, as
            specified in the GENDFParser initialization. Capable of parsing
            either NJOY/GROUPR or PREPRO/GROUPIE processed GENDF files.

        Arguments:
            gendf_path (pathlib._local.PosixPath): Path to the GENDF file from
                which to extract activation data.

        Returns:
            gendf_dict (dict): Dictionary keyed by MF file number and valued
                by a subdictionary with keys `'MTs'` and `'non_zero_xs'`. The
                `'MTs'` key is valued by a set of all reaction types with
                associated activation cross-sections within that `MF`. The
                `'non_zero_xs'` key will be valued by either a
                `collections.defaultdict(dict)` structure keyed by each MT
                with subkeys for each LFS in the case of MF == 10, or a
                `collections.defaultdict(list)` list of dictionaries of
                groupwise-energy bound keys with associated cross-sections.
            nGroups (int): Number of energy groups into which the groupwise
                cross-sections were calculated.
            pKZA (int or None): Parent KZA value. Only extracted when
                `prepro` is True for the parser initialization. Otherwise
                `None`.
        """

        self.gendf_dict = {
            MF : {
                'MTs' : set(),
                'non_zero_xs' : (
                    defaultdict(dict) if MF == 10 else defaultdict(list)
                )
            }
            for MF in self.MFs
        }

        self.nGroups = None
        self.pKZA = None
        self._reset_section_state()
        self.current_MF = None
        self.current_MT = None

        with open(gendf_path, 'r') as f:
            lines = f.readlines()

            if self.prepro:
                self.pKZA = int(
                    self._reformat_endf_float(lines[1].split()[0]) * 10
                    + self._reformat_endf_float(lines[2].split()[3])
                )

            for line in lines:
                mf, mt = self._parse_control(line)

                # Extract number of groups, found in second line of file
                # description sectino
                if (
                    not self.prepro and mf == 1 and mt == 451
                    and int(line.rstrip('\n')[-3:]) == 2
                ):
                    self.nGroups = int(line.split()[2])
                elif self.prepro:
                    groupie_tag = 'Unshielded Group Averages Using'
                    if groupie_tag in line:
                        self.nGroups = int(
                            line.split(groupie_tag)[1].split()[0]
                        )

                if mf in self.MFs and mt != 0:
                    self.gendf_dict[mf]['MTs'].add(mt)

                    if mt != self.current_MT or mf != self.current_MF:
                        self._save_current_section()
                        self._reset_section_state()
                        self.current_MT = mt
                        self.current_MF = mf

                    if self.prepro:
                        self._handle_prepro_line(line, mf)
                    else:
                        self._handle_groupr_line(line)

                else:
                    self._save_current_section()
                    self._reset_section_state()
                    self.current_MT = None
                    self.current_MF = None

            self._save_current_section()

        if not self.gendf_dict.get(10):
            self.gendf_dict.setdefault(10, {'MTs' : [], 'non_zero_xs' : []})

        if not self.nGroups:
            raise ValueError(
                f'{gendf_path} misformatted. Expecting to find group number' \
                ' in MF1, MT451.'
            )

        return self.gendf_dict, self.nGroups, self.pKZA