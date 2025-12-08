import csv
from numpy import array

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

# Track edge cases of unquantifiable MT reaction types
spec_reactions = [
    'total', 'z0', 'nonelas.', 'anything', 'contin.',
    'fission', 'f', 'RES', 'X', 'disap', 'abs'
    ]

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
        {'MT' : {'Reaction' : (z , emission)}}
    
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
            mt_dict[int(row['MT'])] = {'Reaction' : row['Reaction']}
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
            with the general format: {'MT' : {'Reaction' : (z , emission)}}.
    
    Returns:
        mt_dict (dict): Processed dictionary containing the original data from
            the dictionary formatted mt_table.csv, with additional information
            on changes in KZA and emitted particles. The returned format is:
            {'MT' : 
                    {'Reaction'         :               (z, emission)},
                    {'delKZA'           :       integer change in KZA},
                    {'Emitted Particles : string of emitted particles}
            }
    """

    for MT, data in list(mt_dict.items()):
        emitted_particles = data['Reaction'].split(',')[1][:-1]
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
            data['High M'] = (M > 9)
            data['Gas'] = gas
            data['Emitted Particles'] = emitted_particles
        else:
            del mt_dict[MT]

    return mt_dict