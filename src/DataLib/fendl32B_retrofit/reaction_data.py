import csv

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

