# Import packages
import subprocess
import requests
import os
import contextlib
import sys
sys.path.append('./GROUPR')
from groupr_tools import elements

# Download the GENDF file
def gendf_download(element, A, M=None, save_path=None):
    Z = elements[element]
    A = str(A).zfill(3)
    gendf_gen_url = 'https://www-nds.iaea.org/fendl/data/neutron/group/'
    download_url = f'{gendf_gen_url}{Z}{element}_{A}.g'
    save_path = save_path or f'./fendl3_{element}{A}'

    print(f"Downloading GENDF file from: {download_url}")

    response = requests.head(download_url)
    if response.status_code == 404:
        raise FileNotFoundError(f'{download_url} not found')
    elif response.status_code == 301:
        download_url = f'{gendf_gen_url}{Z}{element}{A}.g'

    print(f"Final download URL: {download_url}")

    subprocess.run(['wget', download_url, '-O', save_path])

    M = M or '0'
    pKZA = int(Z + A + M)
    print(f"Downloaded file saved to: {save_path}, pKZA: {pKZA}")
    return save_path, pKZA

# Suppress unnecessary warnings from ENDFtk
@contextlib.contextmanager
def suppress_output():
    with open(os.devnull, 'w') as fnull:
        old_stdout = os.dup(1)
        old_stderr = os.dup(2)
        os.dup2(fnull.fileno(), 1)
        os.dup2(fnull.fileno(), 2)
        try:
            yield
        finally:
            os.dup2(old_stdout, 1)
            os.dup2(old_stderr, 2)
            os.close(old_stdout)
            os.close(old_stderr)

# Extract pKZA data from a GENDF file
def gendf_pkza_extract(gendf_path, M=None):
    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    print(f"First line of GENDF file: {first_line}")
    Z, element, A = first_line.split('-')[:3]
    A = A.split(' ')[0]
    if 'm' in A:
        m_index = A.find('m')
        A = A[:m_index]
    M = str(M) or '0'
    pKZA = int(Z + A + M)
    print(f"Extracted pKZA: {pKZA}")
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
        print(f"Error extracting cross sections for MT {MT}: {e}")
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
        reaction = mt_table[mt_table['MT'] == MT]['Reaction'].values[0]
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
        print(f"Error in reaction calculation for MT {MT}: {e}")
        return None, None