# Import packages
import argparse
import csv
import requests
import sys
sys.path.append('./GROUPR')
from groupr_tools import elements
from logging_config import logger

# Define an argument parser
def fendl_args():
    # Temporarily reset stdout and stderr to the defaults
    # so that arg messages (i.e. --help) print out to terminal
    original_stdout = sys.stdout
    original_stderr = sys.stderr

    try:
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

        parser = argparse.ArgumentParser()

        # Subparsers for 'I' and 'D'
        subparsers = parser.add_subparsers(dest='method', required=True)
        parser_I = subparsers.add_parser('I', help='Local file input')
        parser_I.add_argument('--local-path',
                            required=True,
                            help='Path to the local GENDF file.')
        parser_D = subparsers.add_parser('D', help='Download GENDF file')
        parser_D.add_argument('--element', '-e',
                            required=True,
                            help= 'Chemical symbol for selected element (i.e. Ti).')
        parser_D.add_argument('--A', '-a',
                            required=True,
                            help='Mass number for selected isotope (i.e. 48). If the target is an isomer, "m" after the mass number (i.e. 48m)')

        args = parser.parse_args()
        return args
    
    finally:
        # Restore stdout and stderr to the logger
        sys.stdout = original_stdout
        sys.stderr = original_stderr

# Define a function to read CSV files
def read_csv(csv_path):
    
    # Initialize an empty dictionary
    data_dict = {}

    # Open the CSV file
    with open(csv_path, mode='r') as file:
        # Create a CSV reader object
        csv_reader = csv.DictReader(file)

        # Initialize the dictionary keys with empty lists
        for column in csv_reader.fieldnames:
            data_dict[column] = []

        # Populate the dictionary with the CSV data
        for row in csv_reader:
            for column in csv_reader.fieldnames:
                data_dict[column].append(row[column])
    return data_dict

# Define a function to download the GENDF file from nds.iaea.org
def gendf_download(element, A, M=None, save_path=None):
    # Initialize parameters
    Z = str(elements[element]).zfill(2)
    A = str(A).zfill(3)
    gendf_gen_url = 'https://www-nds.iaea.org/fendl/data/neutron/group/'
    download_url = f'{gendf_gen_url}{Z}{element}_{A}.g'
    save_path = save_path or f'./fendl3_{element}{A}'

    # Check to see if the URL is valid
    response = requests.head(download_url)
    if response.status_code == 404:
        raise FileNotFoundError(f'{download_url} not found')
    elif response.status_code == 301:
        download_url = f'{gendf_gen_url}{Z}{element}{A}.g'

    # Download the file from the URL
    logger.info(f'Downloading file from: {download_url}')
    response = requests.get(download_url, stream=True)
    with open(save_path, 'w') as f:
        f.write(response.content.decode('utf-8'))
    logger.info(f'Downloaded file saved to: {save_path}')

    # Write out the pKZA
    M = M or '0'
    pKZA = int(Z + A + M)

    return save_path, pKZA

# Extract pKZA data from a GENDF file
def gendf_pkza_extract(gendf_path, M=None):
    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    logger.info(f"First line of GENDF file: {first_line}")
    Z, element, A = first_line.split('-')[:3]
    A = A.split(' ')[0]
    if 'm' in A:
        m_index = A.find('m')
        A = A[:m_index]
    M = str(str(M).count('m')) or '0'
    pKZA = int(Z + A + M)
    return pKZA

# Extract cross-section data for a given MT
def extract_cross_sections(file, MT):
    try:
        section = file.section(MT).content
        lines = section.split('\n')[2:-2:2]
        sigma_list = []
        for line in lines:
            sigma = line.split(' ')[2]
            sign = 1 if '+' in sigma else -1
            mantissa, exponent = sigma.split('+') if sign == 1 else sigma.split('-')
            sigma_list.append(float(mantissa) * (10 ** (sign * int(exponent))))
        return sigma_list
    except Exception as e:
        logger.error(f"Error extracting cross sections for MT {MT}: {e}")
        return []

# Count emitted particles
def emitted_particle_count(particle, emitted_particle_string):
    particle_index = emitted_particle_string.find(particle)
    number_str = ''
    for i in range(particle_index - 1, -1, -1):
        if emitted_particle_string[i].isdigit():
            number_str = emitted_particle_string[i] + number_str
        else:
            break
    return int(number_str) if number_str else 1 if particle in emitted_particle_string else None

# Check for isomers
def isomer_check(emitted_particle_string):
    last_digits_str = ''
    for char in reversed(emitted_particle_string):
        if char.isdigit():
            last_digits_str = char + last_digits_str
        else:
            break
    return int(last_digits_str) if last_digits_str else 0

# Calculate reaction
def reaction_calculator(MT, mt_table, pKZA):
    try:
        nucleus_protons = int(str(pKZA)[:2])
        A = int(str(pKZA)[2:5])
        index = mt_table['MT'].index(str(MT))
        reaction = mt_table['Reaction'][index]
        emitted_particles = reaction.split(',')[1][:-1]
        particle_types = ['n', 'd', 'alpha', 'p', 't', '3He']
        emission_tuples = [(emitted_particle_count(p, emitted_particles), p) for p in particle_types if emitted_particle_count(p, emitted_particles)]

        nucleus_neutrons = A - nucleus_protons + 1
        for num_particle, particle in emission_tuples:
            if particle == 'n':
                nucleus_neutrons -= num_particle
            if particle == 'p':
                nucleus_protons -= num_particle

        residual_A = str(nucleus_neutrons + nucleus_protons).zfill(3)
        M = isomer_check(emitted_particles)
        if M != 0:
            emitted_particles = emitted_particles[:-len(str(M))]

        dKZA = int(f"{str(nucleus_protons)}{residual_A}{str(M)}")
        return dKZA, emitted_particles
    except Exception as e:
        logger.error(f"Error in reaction calculation for MT {MT}: {e}")
        return None, None