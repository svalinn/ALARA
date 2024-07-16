import csv
from numpy import array
import pandas as pd

# Define a dictionary containing all of the pathways for neutron and proton
# changes in a nucleus following neutron activation

#             emitted         delta N       delta P
NP_dict   = {'n'     : array([-1      ,      0      ]), # neutron emission
             'p'     : array([ 0      ,     -1      ]), # proton emission
             'd'     : array([-1      ,     -1      ]), # deuteron emission
             't'     : array([-2      ,     -1      ]), # triton emission
             '3He'   : array([-1      ,     -2      ]), # helium-3 emission
             'α'     : array([-2      ,     -2      ]), # alpha emission
             'γ'     : array([ 0      ,      0      ])  # gamma emission
}

def initialize_dataframe():
    """
    Initialize an empty Pandas DataFrame in which to store extracted data from
        TENDL 2017 files.
    
    Arguments:
        None
    
    Returns:
        None
    """
    return pd.DataFrame({
        'Parent KZA'        : [],
        'Daughter KZA'      : [],
        'Emitted Particles' : [],
        'Non-Zero Groups'   : [],
        'Cross Sections'    : []
    })

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
        particle_count (int or None): Count of the target particle present in
            the product. If particles not present, returns None rather than 0.
    """

    particle_index = emitted_particle_string.find(particle)
    number_str = ''
    for i in range(particle_index - 1, -1, -1):
        if emitted_particle_string[i].isdigit():
            number_str = emitted_particle_string[i] + number_str
        else:
            break

    if number_str:
        particle_count = int(number_str)
    elif particle in emitted_particle_string:
        particle_count = 1
    else:
        particle_count = None
    
    return particle_count

def isomer_check(emitted_particle_string):
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
        particle : count_emitted_particles(particle, emitted_particles)
        for particle in NP_dict.keys()
        if particle in emitted_particles
    }
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
    """

    #                  delta N        delta P
    NP_change = array([1       ,      0      ])  # neutron activation

    for particle, count in emission_dict.items():
        NP_change += count * NP_dict[particle]
        
    return NP_change

def process_mt_table(csv_path):
    """
    Load in the mt_table.csv file which contains Table B.1 -
        "Reaction Type Numbers MT" from the ENDF-6 manual which can be found
        at https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf.
        Given this, calculate the resultant change in KZA associated with each
        MT value in the table and tally the particle emissions associated with
        these reactions. Store all of this data in a dictionary of the format:
        {'MT' : {'Reaction' : (z , emission),
                 'delKZA' : change_in_KZA,
                 'Emitted Particles' : string_of_particles
                }
        }
    
    Arguments:
        csv_path (str): File path to mt_table.csv
            This should be in the same repository.
    
    Returns:
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv,
            along with changes in KZA and emitted_particles.
    """

    mt_dict = {}

    with open(csv_path, 'r') as f:
        csv_reader = csv.DictReader(f)
        for row in csv_reader:
            emitted_particles = row['Reaction'].split(',')[1][:-1]
            emission_dict = emission_breakdown(emitted_particles)
            change_N, change_P = nucleon_changes(emission_dict)
            M = isomer_check(emitted_particles)
            mt_dict[row['MT']] = {
                'Reaction' : row['Reaction'],
                'delKZA' : (change_P * 1000 + change_P + change_N) * 10 + M,
                'Emitted Particles' : emitted_particles
            }

    return mt_dict