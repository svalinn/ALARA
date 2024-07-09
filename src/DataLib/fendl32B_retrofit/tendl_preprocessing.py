# Import packages
import csv
import aiohttp
import asyncio
import time
import urllib.error
import urllib.request
import sys
from logging_config import logger, LoggerWriter
import ENDFtk
import contextlib
import os

# Define constant(s)
TENDL_GEN_URL = 'https://tendl.web.psi.ch/tendl_2017/neutron_file/'

def load_csv(csv_path):
    """
    Load in the mt_table.csv file and store it in a dictionary.
    
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

async def fetch(session, url):
    """
    Asynchronously fetch the content from the URL using the provided session.

    Arguments:
        session (aiohttp.client.ClientSession): Aiohttp session to use for making
            the request.
        url (str): The URL from which to fetch content.
    
    Returns:
        str or None: The content of the URL as a string if the request is successful,
            signified by status code 200. Returns None if the request is unsuccessful
            or an error occurs.
    """

    try:
        async with session.get(url) as response:
            if response.status == 200:
                return await response.text()
            else:
                return None
    except aiohttp.ClientError:
        return None

async def fetch_with_sem(semaphore, session, url):
    """
    Use an asyncio semaphore to call the fetch() function for a specific URL.

    Arguments:
        semaphore (asyncio.locks.Semaphore): A semaphore to limit the number of
            concurrent requests.
        session (aiohttp.client.ClientSession): Aiohttp session to use for making
            the request.
        url (str): The URL from which to fetch content.
    
    Returns:
        str or None: The content of the URL as a string if the request is successful,
            signified by status code 200. Returns None if the request is unsuccessful
            or an error occurs.
    """

    async with semaphore:
        return await fetch(session, url)

async def identify_tendl_isotopes(element, concurrency_limit = 50):
    """
    Use asyncio and aiohttp to iterate over all possible mass numbers for a given
        element to identify all of its isotopes and isomers in the TENDL database.
    
    Arguments:
        element (str): Chemical symbol for element of interest (i.e. Ti).
        concurrency_limit (int, optional): The maximum number of concurrent
            processes or sessions that can be handled at once.
    """
    
    A_vals = []
    semaphore = asyncio.Semaphore(concurrency_limit)

    async with aiohttp.ClientSession() as session:
        tasks = []
        # Iterate over all mass numbers possible in the TENDL database
        for A in range(1, 292):
            # Iterate over each number twice to check for isomers
            for i in range(2):
                A_str = str(A).zfill(3)
                if i == 1:
                    A_str += 'm'

                isotope_component = f'{element}/{element}{A_str}/lib/endf/n-{element}{A_str}.'
                tendl_url = TENDL_GEN_URL + isotope_component + 'tendl'
                pendf_url = TENDL_GEN_URL + isotope_component + 'pendf'

                tendl_task = fetch_with_sem(semaphore, session, tendl_url)
                pendf_task = fetch_with_sem(semaphore, session, pendf_url)

                tasks.append((tendl_task, pendf_task, tendl_url, pendf_url, A_str))

        results = await asyncio.gather(*[asyncio.gather(tendl_task, pendf_task) for tendl_task, pendf_task, _, _, _ in tasks])

        for (tendl_result, pendf_result), (tendl_task, pendf_task, tendl_url, pendf_url, A_str) in zip(results, tasks):
            if tendl_result and pendf_result:
                A_vals.append(A_str)

    return A_vals

def urllib_download(download_url, filetype):
    """
    Use the urllib Request and Error packages to download the contents of
        a webpage, if it exists
    
    Arguments:
        download_url (str): Link to the webpage to download the desired file.
        filetype (str): Either "ENDF" or "PENDF" filetypes to be downloaded
            (case insensitive).
    """

    try:
        with urllib.request.urlopen(download_url) as f:
            temp_file = f.read().decode('utf-8')
    except urllib.error.URLError as e:
        if e.code == 404:
            temp_file = None
            raise FileNotFoundError(f'{filetype.upper()} file does not exist at {download_url}')
    return temp_file

def download_tendl(element, A, filetype, save_path = None):
    """
    Download ENDF/PENDF files from the TENDL 2017 database for specific isotopes.

    Arguments:
        element (str): Chemical symbol for element of interest (i.e. Ti).
        A (str or int): Mass number for selected isotope (i.e. 48).
            If the target is an isomer, "m" after the mass number (i.e. 48m),
            so A must be input as a string.
        filetype (str): Either "ENDF" or "PENDF" filetypes to be downloaded
            (case insensitive).
        save_path (str, optional): User-defined file path for the downloaded file.
            Defaults to None and will be otherwise defined internally.
    
    Returns:
        save_path (str): File path for the file downloaded file.
    """

    # Ensure that A is properly formatted
    A = A.zfill(4) if 'm' in A else str(A).zfill(3)

    # Create a dictionary to generalize formatting for both ENDF and PENDF files
    file_handling = {'endf' : {'ext': 'tendl', 'tape_num': 20},
                     'pendf' : {'ext': 'pendf', 'tape_num': 21}}
    
    # Construct the filetype and isotope specific URL
    isotope_component = f'{element}/{element}{A}/lib/endf/n-{element}{A}.'
    ext = file_handling[filetype.lower()]['ext']
    download_url = TENDL_GEN_URL + isotope_component + ext
    logger.info(f'{filetype.upper()} URL: {download_url}')

    # Define a save path for the file if there is not one already specified
    if save_path is None:
        save_path = f'tape{file_handling[filetype.lower()]["tape_num"]}'

    # Conditionally downloa
    temp_file = urllib_download(download_url, filetype)

    with open(save_path, 'w') as f:
        f.write(temp_file)

    return save_path

def extract_gendf_pkza(gendf_path, M=None):
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

def extract_endf_specs(path, filetype):
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