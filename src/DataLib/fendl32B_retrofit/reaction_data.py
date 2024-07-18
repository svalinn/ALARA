import csv
from numpy import array

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
        csv_path (str): File path to mt_table.csv
            This should be in the same repository.
    
    Returns:
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv.
    """

    mt_dict = {}

    with open(csv_path, 'r') as f:
        csv_reader = csv.DictReader(f)
        for row in csv_reader:
            mt_dict[row['MT']] = {'Reaction' : row['Reaction']}

    return mt_dict

