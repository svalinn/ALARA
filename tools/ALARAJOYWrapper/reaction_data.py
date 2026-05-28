import csv
from numpy import array
import pandas as pd

# Define a dictionary containing all of the pathways for neutron and proton
# changes in a nucleus following neutron activation

#             emitted         delta N       delta P
NP_dict = {
             'n'     : array([-1      ,      0      ]), # neutron emission
             'g'     : array([ 0      ,      0      ]), # gamma emission
             'p'     : array([ 0      ,     -1      ]), # proton emission
             'd'     : array([-1      ,     -1      ]), # deuteron emission
             't'     : array([-2      ,     -1      ]), # triton emission
             'h'     : array([-1      ,     -2      ]), # helium-3 emission
             'a'     : array([-2      ,     -2      ])  # alpha emission
}
GASES = list(NP_dict.keys())[2:]
GAS_DF = pd.DataFrame({
    'gas'       : GASES,
    'kza'       : [10010, 10020, 10030, 20030, 20040],
    'total_mt'  : range(203, 207 + 1)
})
DECAY_MF = 8
DECAY_MT = 457
TEND_RECORD =  ' ' * 68 + '-1 0  0    0'

# Track edge cases of unquantifiable MT reaction types
spec_reactions = [
    'total', 'z0', 'nonelas.', 'anything', 'contin.',
    'fission', 'f', 'RES', 'X', 'disap', 'abs'
    ]

SPEC_MTS = {
    1,  # (n, total)
    2,  # (z,z0)
    3,  # (z, nonelas.)
    5,  # (z, anything)
    18, # (z, fission)
    19, # (n,f)
    20, # (n,nf)
    21, # (n,2nf)
    38  # (n,3nf)
}

def count_emitted_particles(particle, emitted_particle_string):
    """
    Count emitted particles from a reaction given a target particle
        and the particles produced in a neutron activation reaction.

    Arguments:
        particle (str): Name of the target particle produced in the reaction.
            Options include n, p, alpha, d, t, and 3He, corresponding to
            neutrons, protons, alpha particles, deuterons, tritons,
            and helium-3 nuclides.
        emitted_particle_string (str): Particle product(s) of the neutron
            activation, of the format 'p' for a single proton for example or
            '2n' for two neutrons etc.

    Returns:
        particle_count (int): Count of the target particle present in
            the product.
    """

    num_str = ''
    particle_count_index = emitted_particle_string.find(particle) - 1
    while (particle_count_index >=0 and
           emitted_particle_string[particle_count_index].isdigit()):
            num_str = emitted_particle_string[particle_count_index] + num_str
            particle_count_index -= 1

    if num_str:
        particle_count = int(num_str)
    elif particle in emitted_particle_string:
        particle_count = 1
    else:
        particle_count = 0
    
    return particle_count

def emission_breakdown(emitted_particles):
    """
    Identify the particles and their respective counts from a particular
        nuclear decay in response to neutron activation. These tallies are
        saved in a dictionary with the name of the particles as the keys.

    Arguments:
        emitted_particles (str): Particle product(s) of a neutron activation
            reaction formatted in a singular string (i.e. np for a 
            (neutron, proton) emission.

    Returns:
        emission_dict (dict): Dictionary containing each individual particle
            type and their respetive counts. For an np emission, this would
            read out as {'n': 1, 'p': 1}.
    """

    emission_dict = {
        particle: count_emitted_particles(particle, emitted_particles)
        for particle in NP_dict.keys()
        if particle in emitted_particles and not any(
            spec_case in emitted_particles for spec_case in spec_reactions
        )
    }

    # Handle gas production totals (MTs 203-207) of the format Xz
    if 'X' in emitted_particles:
        for gas in GASES:
            if gas in emitted_particles:
                emission_dict[emitted_particles] = 1
    
    return emission_dict

def nucleon_changes(emission_dict):
    """
    Calculate the change in neutrons and protons in a nucleus in response to
        neutron activation and given a particular set of emitted particles.
    
    Arguments:
        emission_dict (dict): Dictionary containing each individual particle
            type in an emission from a nuclear decay and their respective
            counts. For an np emission, this would read out as
            {'n': 1, 'p': 1}.
    
    Returns:
        NP_change (numpy.array): A one-dimensional array indicating the net
            change in neutrons and protons in a nucleus as a result of neutron
            activation and subsequent decay. The array is in the format of
            array([neutron_change, proton_change]).

            If emission_dict is empty, NP_change will be returned as an array
            of None values, in the format array([None, None]).
    """
    
    NP_change = None
    if emission_dict:
        #                  delta N        delta P
        NP_change = array([1       ,      0      ])  # neutron activation

        for particle, count in emission_dict.items():
            if 'X' not in particle:
                NP_change += count * NP_dict[particle]
        
    return NP_change

def load_mt_table(csv_path):
    """
    Load in the mt_table.csv file which contains Table B.1 -
        "Reaction Type Numbers MT" from the ENDF-6 manual which can be found
        at https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf.
        Given this, calculate the resultant change in KZA associated with each
        MT value in the table and tally the particle emissions associated with
        these reactions. Store all of this data in a dictionary of the format:
        {'MT' : {'reaction' : (z , emission)}}
    
    Arguments:
        csv_path (pathlib._local.PosixPath): File path to mt_table.csv.
            This should be in the same repository.
    
    Returns:
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.
    """

    mt_dict = {}

    with open(csv_path, 'r') as f:
        csv_reader = csv.DictReader(f)
        for row in csv_reader:
            mt_dict[int(row['MT'])] = {'reaction' : row['Reaction']}
    if mt_dict:
        return mt_dict
    else:
        raise Exception('CSV file is empty or missing.')

def check_for_isomer(emitted_particle_string):
    """
    Check the isomeric status of a neutron-activated nucleus.
        By the formatting conventions of ENDF reaction types,
        if the string of a reaction product ends with a digit,
        that signifies the excitation state of the nucleus, so 
        this function looks for and stores these values.

    Arguments:
        emitted_particle_string (str): Particle product(s) of the neutron
            activation.
    
    Returns:
        isomeric_value (int): Nuclear excitation level of the activated
            nucleus. For a nucleus in the ground state, isomeric_state = 0.
    """

    last_digits_str = ''
    for char in reversed(emitted_particle_string):
        if char.isdigit():
            last_digits_str = char + last_digits_str
        else:
            break
    isomeric_value = int(last_digits_str) if last_digits_str else 0

    return isomeric_value

def process_mt_data(mt_dict):
    """
    Read in the dictionary containing the data from mt_table.csv that is
        imported using load_mt_data() and process the reaction data to 
        tally the emitted particles by their respective types and calculate
        the change in KZA associated with each reaction. Write out these
        reaction-specific values to new keys in the dictionary.
    
    Arguments:
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv,
            with the general format: {'MT' : {'reaction' : (z , emission)}}.
    
    Returns:
        mt_dict (dict): Processed dictionary containing the original data from
            the dictionary formatted mt_table.csv, with additional information
            on changes in KZA and emitted particles. The returned format is:
            {'MT' : 
                    {'reaction':                               (z, emission)},
                    {'delKZA'  :                       integer change in KZA},
                    {'gas'     : name of total gas production, if MT=203-207},
                    {'emitted' :                 string of emitted particles}
            }
    """

    for MT in (set(mt_dict) & SPEC_MTS):
        del mt_dict[MT]

    for MT, data in list(mt_dict.items()):
        emitted_particles = data['reaction'].split(',')[1][:-1]
        emission_dict = emission_breakdown(emitted_particles)
        change_NP = nucleon_changes(emission_dict)
        M = check_for_isomer(emitted_particles)
        emitted_list = list(emitted_particles)
        gas = emitted_list[1] if (
            emitted_list[0] == 'X' and emitted_list[1] in GASES
        ) else None

        # Conditionally remove isomer tags from emitted particle strings
        if M > 0:
            emitted_particles = emitted_particles[:-len(str(M))]

        # Set gas total tag to standard ALARA tag for gas reaction residual
        if 'X' in emitted_particles:
            emitted_particles = 'x'
        
        if change_NP is not None:
            change_N, change_P = change_NP
            data['delKZA'] = (change_P * 1000 + change_P + change_N) * 10 + M
            data['gas'] = gas
            data['emitted'] = emitted_particles
        
        else:
            del mt_dict[MT]

    return mt_dict

def append_to_compiled_lib(lines, compiled_file):
    """
    Append the contents of a single- or multi-nuclide decay file to a compiled
        file for the whole library.

    Arguments:
        lines (list of str): List of parsed lines from a decay file.
        compiled_file (pathlib._local.PosixPath): Path to the compiled decay
            library file.

    Returns:
        None
    """

    # Ensure final line includes a newline signifier
    if not lines[-1].endswith('\n'):
        lines[-1] += '\n'

    with open(compiled_file, 'a') as f:
        f.writelines(lines)

def compile_decay_lib(decay_dir, decay_lib_type, dir):
    """
    Iteratively compile the data from individual-nuclide decay files into a
        single file in ascending order of KZA. Can either be an EAF or UKDD
        decay library.

    Arguments:
        decay_dir (pathlib._local.PosixPath): Filepath to the directory
            exclusively containing decay data files for either an EAF or UKDD
            decay library.
        decay_lib_type (str): Identifier tag for either EAF or UKDD data.
        dir (pathlib._local.PosixPath): Path to the current working directory
            (CWD) from which the command was called.

    Returns:
        compiled_file (pathlib._local.PosixPath): Path to the compiled decay
            library file.
    """

    from tendl_processing import create_endf_file_obj

    compiled_file = dir / f'{decay_dir}_compiled'
    compiled_file.unlink(missing_ok=True)

    ukdd_options = ['ukdd', 'ukaeadd', 'decay_2020', 'decay_2012']
    if decay_lib_type.lower() in ukdd_options:
        decay_lib_type = 'ukdd'

    ukdd_nucs = dict()
    for decay_file in decay_dir.iterdir():
        if '.DS_Store' in str(decay_file):
            decay_file.unlink()
            continue

        with open(decay_file, 'r') as f:
            lines = f.readlines()

        # Remove blank lines
        updated_lines = [line for line in lines if line.strip()]

        # Include missing TEND Records
        if TEND_RECORD.strip().rstrip('0').strip() not in updated_lines[-1]:
            tend_line = TEND_RECORD
            if not updated_lines[-1].endswith('\n'):
                tend_line = '\n' + tend_line
            updated_lines.append(tend_line)

        # Overwrite file if changes were made
        if lines != updated_lines:
            with open(decay_file, 'w') as f:
                f.writelines(updated_lines)

        # Save individual nuclide KZAs from UKDD data for ascending KZA
        # compilation
        if decay_lib_type == 'ukdd':
            endf_file_obj, _ = create_endf_file_obj(decay_file, DECAY_MF)
            decay_data = endf_file_obj.section(DECAY_MT).parse()
            ukdd_nucs[decay_data.ZA * 10 + decay_data.LISO] = decay_file

        elif 'eaf' in decay_lib_type.lower():
            append_to_compiled_lib(updated_lines, compiled_file)

        else:
            raise ValueError(
                'Invalid library type. Must be either an EAF or UKDD release.'
            )

    if decay_lib_type == 'ukdd':
        for kza in sorted(ukdd_nucs):
            with open(ukdd_nucs[kza], 'r') as f:
                lines = f.readlines()

            append_to_compiled_lib(lines, compiled_file)

    print(
        f'Compiled {decay_lib_type.upper()} decay libary to {compiled_file}.'
    )
    return compiled_file

def standardize_float(num_str):
    """
    Process non-uniformly formatted scientific notation numbers from parsed
        decay data to standard floating point numbers.
    
    Arguments:
        num_str (str): Number from an decay file with non-uniform formatting.

    Returns:
        num (float): Reformatted number to be able to be properly converted to
            a float.
    """

    num_str = num_str.strip()
    if not num_str:
        return 0.0

    num_str = num_str.replace(' ', '')
    if 'E' in num_str or 'e' in num_str:
        return float(num_str)

    exp_idx = max(num_str.rfind('+'), num_str.rfind('-'))
    if exp_idx > 0:
        return float(num_str[:exp_idx] + 'E' + num_str[exp_idx:])

    return float(num_str)

def get_MT_from_line(line):
    """
    Extract the MT number from a given line of an EAF file, based on a set
        line formatting.

    Arguments:
        line (str): Text of a single line from an EAF file.

    Returns:
        MT (int): MT reaction number of that line.
    """
    
    return int(line[72:75])

def find_nucs_from_decay_lib(compiled_decay_lib):
    """
    Parse through an pre-compiled decay library file to build a dictionary
        keyed by all radionuclides with their respective half-lives as the
        values. Modeled after the file parsing methods in
        ALARA/src/DataLib/EAFLib.C.

    Arguments:
        compiled_decay_lib (pathlib._local.PosixPath): Filepath to a pre-
            compiled decay library. Must be either an EAF decay library (i.e. 
            EAF4.1, EAF2010, etc.) or a UKDD decay library (UKDD-12, UKDD-20).

    Returns:
        all_nucs (dict): Dictionary keyed by all nuclide KZAs in the decay
            library, with values of their half-lives (-1 for stable nuclides).
    """

    all_nucs = {}
    with open(compiled_decay_lib, 'r') as f:
        
        # Read header and skip introductory comment lines
        first_line = f.readline().strip()
        try:
            n_comment_lines = int(first_line.split()[0])
        except ValueError:
            n_comment_lines = 1

        for _ in range(n_comment_lines):
            f.readline()

        _in_decay_block = False
        line = f.readline()

        while line:
            MT = get_MT_from_line(line)
            if not _in_decay_block and MT == DECAY_MT:
                _in_decay_block = True

                # Parse nuclide KZA
                za = int(standardize_float(line[:11]))
                M = int(line[33:44].strip())
                kza = za * 10 + M
                stability = int(line[44:56].strip()) # 0->unstable , 1->stable

                # Read half-life (next line)
                line = f.readline()
                if stability == 0:
                    thalf = standardize_float(line[:11])
                    all_nucs[kza] = thalf

                else:
                    all_nucs[kza] = -1

            elif MT != DECAY_MT:
                _in_decay_block = False

            line = f.readline()

    return all_nucs