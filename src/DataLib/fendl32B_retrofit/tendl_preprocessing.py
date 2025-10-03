# Import packages
import aiohttp
from bs4 import BeautifulSoup
import urllib.error
import urllib.request
from logging_config import logger
import ENDFtk


# Define constant(s)
TENDL_GEN_URL = 'https://tendl.web.psi.ch/tendl_2017'

async def fetch_and_parse_html(session, url):
    """
    Asynchronously fetch the content from the URL using the provided session.

    Arguments:
        session (aiohttp.client.ClientSession): Aiohttp session to use for
            making the request.
        url (str): The URL from which to fetch content.
    
    Returns:
        str or None: The content of the URL as a string if the request is
            successful, signified by status code 200. Returns None if the
            request is unsuccessful or an error occurs.
    """

    async with session.get(url) as response:
        if response.status == 200:
            html = await response.text()
            soup = BeautifulSoup(html, 'html.parser')
            links = soup.find_all('a')
            return [link.get('href') for link in links if link.get('href')]
        return []

async def identify_tendl_isotopes(element):
    """
    Use asyncio and aiohttp to iterate over all possible mass numbers for a
        given element to identify all of its isotopes and isomers in the TENDL
        database.
    
    Arguments:
        element (str): Chemical symbol for element of interest (i.e. Ti).
        concurrency_limit (int, optional): The maximum number of concurrent
            processes or sessions that can be handled at once.

    Returns:
        A_vals (list of str): List of all of the mass numbers of isotopes that
            have data stored in both ENDF (TENDL) and PENDF files in the TENDL
            2017 nuclear database.
    """

    A_vals = []
    async with aiohttp.ClientSession() as session:
        tasks = []
        for A in range(1, 292):
            for i in range(2):
                A_str = str(A).zfill(3)
                if i == 1:
                    A_str += 'm'

                navigation_page_url = (
                    f'{TENDL_GEN_URL}/neutron_html/{element}/'
                    f'Neutron{element}{str(A).zfill(2)}.html'
                )
                isotope_component = (
                    f'/neutron_file/{element}/{element}{A_str}/'
                    f'lib/endf/n-{element}{A_str}.'
                )
                tendl_url = TENDL_GEN_URL + isotope_component + 'tendl'
                pendf_url = TENDL_GEN_URL + isotope_component + 'pendf'

                tasks.append((navigation_page_url, tendl_url,
                              pendf_url, A_str))

        # Execute tasks and gather results
        for navigation_page_url, tendl_url, pendf_url, A_str in tasks:
            nav_urls = await fetch_and_parse_html(session,
                                                  navigation_page_url)

            if tendl_url in nav_urls and pendf_url in nav_urls:
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
            raise FileNotFoundError(
                f'{filetype.upper()} file does not exist at {download_url}'
            )
    
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
        save_path (str, optional): User-defined file path for the
            downloaded file.

            Defaults to None and will be otherwise defined internally.
    
    Returns:
        save_path (str): File path for the file downloaded file.
    """

    # Ensure that A is properly formatted
    A = A.zfill(4) if 'm' in A else str(A).zfill(3)

    file_handling = {'endf' : {'ext': 'tendl', 'tape_num': 20},
                     'pendf' : {'ext': 'pendf', 'tape_num': 21}}
    
    # Construct the filetype and isotope specific URL
    isotope_component = (
        f'/neutron_file/{element}/{element}{A}/lib/endf/n-{element}{A}.'
    )
    ext = file_handling[filetype.lower()]['ext']
    download_url = TENDL_GEN_URL + isotope_component + ext
    logger.info(f'{filetype.upper()} URL: {download_url}')

    if save_path is None:
        save_path = f'tape{file_handling[filetype.lower()]["tape_num"]}'

    temp_file = urllib_download(download_url, filetype)

    with open(save_path, 'w') as f:
        f.write(temp_file)

    return save_path

def extract_gendf_pkza(gendf_path):
    """
    Read in and parse the contents of a GENDF file to construct the parent
        KZA. KZA values are defined as ZZAAAM, where ZZ is the isotope's
        atomic number, AAA is the mass number, and M is the isomeric state
        (0 if non-isomeric).
    
    Arguments:
        gendf_path (str): File path to the GENDF file being analyzed.
        M (str, optional): Identifier of isomer, signified by the letter "M"
            at the end of the mass number string.

            Defaults to None and will be otherwise defined internally.
    
    Returns:
        pKZA (int): Parent KZA identifier.
        A (int): Mass number of the parent nucleus.
        element (str): Chemical name of the parent.
    """

    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    Z, element, A = first_line.split('-')[:3]
    
    Z = int(Z)
    M = 1 if 'm' in A.lower() else 0
    A = int(A.lower().split(' ')[0].split('m')[0])
    pKZA = (Z * 1000 + A) * 10 + M
    return pKZA, A, element

def extract_endf_specs(path, filetype):
    """
    Extract the material ID and MT numbers from an ENDF or GENDF file.

    Arguments:
        path (str): File path to the selected ENDF/GENDF file.
        filetype (str): Either ENDF or GENDF (case insensitive)
    
    Returns:
        endf_specs (list): List containing the following values from the file:
        matb (int): Unique material ID extracted from the file.
        MTs (list): List of reaction types (MT's) present in the file.
        file (ENDFtk.tree.File or None): ENDFtk file object containing the
            contents for a specific material's cross-section data.
            Only returns the file for GENDF filetypes.
    """

    tape = ENDFtk.tree.Tape.from_file(path)
    matb = tape.material_numbers[0]
    # Set MF for cross sections
    xs_MF = 3
    file = tape.material(matb).file(xs_MF)
    # Extract the MT numbers that are present in the file
    MTs = [MT.MT for MT in file.sections.to_list()]

    endf_specs = [matb, MTs]
    if filetype.lower() == 'gendf':
        endf_specs.append(file)
    
    return (endf_specs)

def extract_cross_sections(file, MT):
    """
    Parse through the contents of a GENDF file section to extract the
        cross-section data for a specific reaction type (MT).
    
    Arguments:
        file (ENDFtk.tree.File): ENDFtk file object containing a specific
            material's cross-section data.
        MT (int): Numerical identifier for the reaction type corresponding to
            the file's sectional organization.
    
    Returns:
        sigma_list (list): All of the cross-sections for a given reaction type
            and material, listed as floating point numbers. If the run fails,
            the function will just return an empty list.
    """

    section = file.section(MT).content

    # Only every 2nd line starting at the 3rd line has cross-section data.
    lines = section.split('\n')[2:-2:2]

    # Extract the 3rd token and convert to more conventional string
    # representation of a float
    sigma_list = [
        float(line.split(' ')[2].replace('+','E+').replace('-','E-'))
        for line in lines
    ]

    return sigma_list