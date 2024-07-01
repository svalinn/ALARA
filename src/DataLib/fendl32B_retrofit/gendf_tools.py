# Import packages
import csv
import sys
from logging_config import logger, LoggerWriter
import ENDFtk
import contextlib
import os
import pandas as pd

def read_csv(csv_path):
    """
    Read in the mt_table.csv file and store it in a dictionary.
    
    Arguments:
        csv_path (str): File path to mt_table.csv
            This should be in the same repository.
    
    Returns:
        data_dict (dict): Dictionary formatted data structure for mt_table.csv
    """
    mt_dict = {}

    with open(csv_path, 'r') as f:
        csv_reader = csv.DictReader(f)
        
        for row in csv_reader:
            mt_dict[row['MT']] = row['Reaction']

    return mt_dict

def gendf_pkza_extract(gendf_path, M=None):
    """
    Read in and parse the contents of a GENDF file to construct the parent KZA.
        KZA values are defined as ZZAAAM, where ZZ is the isotope's atomic number,
        AAA is the mass number, and M is the isomeric state (0 if non-isomeric).
    
    Arguments:
        gendf_path (str): File path to the GENDF file being analyzed.
        M (str, optional): Identifier of isomer, signified by the letter "M" at the end
            of the mass number string.
            Defaults to None and will be otherwise defined internally.
    
    Returns:
        pKZA (str): Parent KZA identifier.
    """

    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    logger.info(f"First line of GENDF file: {first_line}")
    Z, element, A = first_line.split('-')[:3]
    A = A.split(' ')[0]
    if 'm' in A:
        m_index = A.find('m')
        A = A[:m_index]
    M = str(str(M).count('M')) or '0'
    pKZA = Z.zfill(2) + A.zfill(3) + M
    return pKZA

@contextlib.contextmanager
def redirect_ENDFtk_output():
    """
    Force ENDFtk-specific output to the logger instead of the terminal.
        ENDFtk prompts a number of warnings, which would not otherwise redirect
        to the logger without a more strong-armed method, and this function only
        needs to be called when explicitly making use of the ENDFtk module.

    Arguments:
        None

    Returns:
        None    
    """

    # Redirect stdout and stderr to logger
    logger_stdout = LoggerWriter(logger.info)
    logger_stderr = LoggerWriter(logger.error)

    with open(os.devnull, 'w') as fnull:
        old_stdout = os.dup(1)
        old_stderr = os.dup(2)
        # Suppress terminal stdout readout
        os.dup2(fnull.fileno(), 1)

        sys.stdout = logger_stdout
        sys.stderr = logger_stderr
        try:
            yield
        finally:
            os.dup2(old_stdout, 1)
            os.dup2(old_stderr, 2)
            os.close(old_stdout)
            os.close(old_stderr)
            # Reset stdout and stderr to default
            sys.stdout = sys.__stdout__
            sys.stderr = sys.__stderr__

def endf_specs(path, filetype):
    """
    Extract the material ID and MT numbers from an ENDF or GENDF file.

    Arguments:
        path (str): File path to the selected ENDF/GENDF file.
        filetype (str): Either ENDF or GENDF (case insensitive)
    
    Returns:
        matb (int): Unique material ID extracted from the file.
        MTs (list): List of reaction types (MT's) present in the file.
        file (ENDFtk.tree.File or None): ENDFtk file object containing the contents
            for a specific material's cross-section data.
            Only returns the file for GENDF filetypes.
    """

    with redirect_ENDFtk_output():
        # Read in ENDF tape using ENDFtk
        tape = ENDFtk.tree.Tape.from_file(path)

        # Determine the material ID
        mat_ids = tape.material_numbers
        matb = mat_ids[0]

        # Set MF for cross sections
        xs_MF = 3

        # Extract out the file
        file = tape.material(matb).file(xs_MF)

        # Extract the MT numbers that are present in the file
        MTs = [MT.MT for MT in file.sections.to_list()]

    filetype = filetype.lower()
    return_values = {
        'endf': (matb, MTs),
        'gendf': (matb, MTs, file)
    }
    return return_values.get(filetype)

def extract_cross_sections(file, MT):
    """
    Parse through the contents of a GENDF file section to extract the cross-section
        data for a specific reaction type (MT).
    
    Arguments:
        file (ENDFtk.tree.File): ENDFtk file object containing a specific material's
            cross-section data.
        MT (int): Numerical identifier for the reaction type corresponding to the
            file's sectional organization.
    
    Returns:
        sigma_list (list): All of the cross-sections for a given reaction type
            and material, listed as floating point numbers. If the run fails,
            the function will just return an empty list.
    """

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

def emitted_particle_count(particle, emitted_particle_string):
    """
    Count emitted particles from a reaction given a target particle
        and the particles produced in a neutron activation reaction.

    Arguments:
        particle (str): Name of the target particle produced in the reaction.
            Options include n, p, alpha, d, t, and 3He, corresponding to
            neutrons, protons, alpha particles, deuterons, tritons, and helium-3 nuclides.
        emitted_particle_string (str): Particle product(s) of the neutron activation,
            of the format 'p' for a single proton for example or '2n' for two neutrons etc.

    Returns:
        number_str (int or None): Count of the target particle present in the product.
            For particles not present, returns None rather than 0.
    """

    particle_index = emitted_particle_string.find(particle)
    number_str = ''
    for i in range(particle_index - 1, -1, -1):
        if emitted_particle_string[i].isdigit():
            number_str = emitted_particle_string[i] + number_str
        else:
            break

    if number_str:
        number_str = int(number_str)
    elif particle in emitted_particle_string:
        number_str = 1
    else:
        number_str = None
    
    return number_str

# Check for isomers
def isomer_check(emitted_particle_string):
    """
    Check the isomeric status of a neutron-activated nucleus.
        By the formatting conventions of ENDF reaction types,
        if the string of a reaction product ends with a digit,
        that signifies the excitation state of the nucleus, so 
        this function looks for and stores these values.

    Arguments:
        emitted_particle_string (str): Particle product(s) of the neutron activation.
    
    Returns:
        isomeric_state (int): Nuclear excitation level of the activated nucleus.
            For a nucleus in the ground state, isomeric_state = 0.
    """

    last_digits_str = ''
    for char in reversed(emitted_particle_string):
        if char.isdigit():
            last_digits_str = char + last_digits_str
        else:
            break
    isomeric_value = int(last_digits_str) if last_digits_str else 0
    return isomeric_value

# Nuclear transmutation function
def nuclear_decay(A, nucleus_protons, emission_tuples):
    """
    Reconfigure nucleus for nuclear decay during neutron activation
        by adding in a single neutron and then subtracting the total number
        of neutrons and protons (if any) emitted during the reaction from 
        the nucleus.
    
    Arguments:
        A (int): Mass number for target isotope.
        nucleus_protons (int): Atomic number for the target isotope,
            namely the number of protons in the nucleus.
        emission_tuples (list): List of all emitted particles for a given reaction,
            in the form of tuples with the particle count as the first value
            and the particle symbol as the second value. For example, a reaction that
            emits one neutron and one proton will have 
            emission_tuples = [(1, 'n'), (1, 'p')].
        
    Returns:
        nucleus_neutrons (int): Updated count of neutrons in the residual nucleus.
        nucleus_protons (int): Updated count of protons in the residual nucleus.
    """

    nucleus_neutrons = A - nucleus_protons + 1
    for num_particle, particle in emission_tuples:
        if particle == 'n':
            nucleus_neutrons -= num_particle
        if particle == 'p':
            nucleus_protons -= num_particle
        if particle == 'd':
            nucleus_neutrons -= num_particle
            nucleus_protons -= num_particle
        if particle == 't':
            nucleus_neutrons -= 2 * num_particle
            nucleus_protons -= num_particle
        if particle == '3He':
            nucleus_neutrons -= num_particle
            nucleus_protons -= 2 * num_particle
        if particle == 'α':
            nucleus_neutrons -= 2 * num_particle
            nucleus_protons -= 2 * num_particle

    return nucleus_neutrons, nucleus_protons

# Calculate reaction
def reaction_calculator(MT, mt_dict, pKZA):
    """
    Calculate the products of a neutron activation reaction given
        the parent nuclide's KZA and the selected reaction type (MT).
        This calculation determines both the residual nucleus, as described by the
        daughter KZA value (dKZA) and the emitted particle(s).

    Arguments:
        MT (int): Unique identifier for the reaction type corresponding to a specific
            reaction tabulated in the mt_table dictionary.
        mt_dict (dict): Reference dictionary containing reaction information
            for each MT number pre-defined in the ENDF manual.
            (https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf)
        pKZA (int or str): Parent KZA identifier of the format ZZAAAM,
            where ZZ is the isotope's atomic number, AAA is the mass number, 
            and M is the isomeric state (0 if non-isomeric).
    
    Returns:
        dKZA (str): KZA of the residual (daughter) nucleus.
        emitted_particles (str): Name of the particles emitted from the reaction,
            given as a single string. If multiple particles are emitted from the reaction,
            then the emitted_particles would be written in the form "np", corresponding
            to the emission of a neutorn and a proton.
    """

    try:
        # Extract the parent nuclide properties from the pKZA
        nucleus_protons = int(str(pKZA)[:2])
        A = int(str(pKZA)[2:5])

        # Identify the particles emitted from the given reaction
        reaction = mt_dict[str(MT)]
        emitted_particles = reaction.split(',')[1][:-1]
        
        # Tally the counts of each emitted particle from the reaction
        particle_types = ['n', 'd', 'α', 'p', 't', '3He', 'gamma']
        emission_tuples = [
            (
                emitted_particle_count(particle, emitted_particles),
                particle
            )
            for particle in particle_types
            if emitted_particle_count(particle, emitted_particles)
        ]

        # Reconfigure nucleus to account for changing nucleon counts
        nucleus_neutrons, nucleus_protons = nuclear_decay(A,
                                                          nucleus_protons,
                                                          emission_tuples)
        residual_A = str(nucleus_neutrons + nucleus_protons).zfill(3)
        nucleus_protons = str(nucleus_protons).zfill(2)
        M = isomer_check(emitted_particles)
        if M != 0:
            emitted_particles = emitted_particles[:-len(str(M))]
        dKZA = f"{str(nucleus_protons)}{residual_A}{str(M)}"
        return dKZA, emitted_particles
    
    except Exception as e:
        logger.error(f"Error in reaction calculation for MT {MT}: {e}")
        return None, None
    
def iterate_MTs(MTs, file_obj, mt_dict, pKZA):
    # Initialize lists
    cross_sections_by_MT = []
    emitted_particles_list = []
    dKZAs = []
    groups = []

    # Extract data for each MT
    for MT in MTs:
        try:
            sigma_list = extract_cross_sections(file_obj, MT)
            if not sigma_list:
                continue
            dKZA, emitted_particles = reaction_calculator(MT, mt_dict, pKZA)
            if dKZA is None:
                continue
            cross_sections_by_MT.append(sigma_list)
            dKZAs.append(dKZA)
            emitted_particles_list.append(emitted_particles)
            groups.append(len(sigma_list))
        except Exception as e:
            logger.error(f"Error processing MT {MT}: {e}")
            continue
    
    # Store data in a Pandas DataFrame
    gendf_data = pd.DataFrame({
        'Parent KZA': [pKZA] * len(dKZAs),
        'Daughter KZA': dKZAs,
        'Emitted Particles': emitted_particles_list,
        'Non-Zero Groups' : groups,
        'Cross Sections': cross_sections_by_MT
    })

    return gendf_data
