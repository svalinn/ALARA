# Import packages
import ENDFtk
from pathlib import Path

def get_isotope(stem):
    """
    Extract the element name and mass number from a given filename.
    Arguments:
        stem (str): Stem of a an ENDF (TENDL) and/or PENDF file, formatted
            as f'{element}{mass_number}.ext'
    Returns:
        element (str): Chemical symbol of the isotope whose data is contained
            in the file.
        A (str): Mass number of the isotope, potentially including the letter
            "m" at the end if the isotope is in a metastable state.
    """

    for i, char in enumerate(stem):
        if char.isdigit():
            break

    element = stem[:i]
    A = stem[i:]       

    return element, A

def search_for_files(dir = '.'):
    """
    Search through a directory for all pairs of ENDF (TENDL) and PENDF files
        that have matching stems. If so, save the paths and the isotopic
        information to a dictionary.
    
    Arguments:
        directory (str, optional): Path to the directory in which to search
            for ENDF and PENDF files.
            Defaults to the present working directory (".").
    Returns:
        file_info (dict): Dictionary containing the chemical symbol, mass
            number, and paths to the ENDF and PENDF files for a given isotope.
            The dictionary is formatted as such:
            {f'{element}{mass_number}' :
                                    {'Element'} : Isotope's chemical symbol,
                                    {'Mass Number'} : Isotope's mass number,
                                    {'File Paths'} : (endf_path, pendf_path)
            }
    """

    dir = Path(dir)

    file_info = {}
    for suffix in ['tendl', 'endf']:
        for file in dir.glob(f'*.{suffix}'):
            if file.with_suffix('.pendf').is_file():
                element, A = get_isotope(file.stem)
                file_info[f'{element}{A}'] = {
                    'Element'       :                             element,
                    'Mass Number'   :                                   A,
                    'File Paths'    :   (file, file.with_suffix('.pendf'))
                }

    return file_info

def extract_endf_specs(path):
    """
    Extract the material ID and MT numbers from an ENDF file.
    Arguments:
        path (str): File path to the selected ENDF file.
    
    Returns:
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

    return (matb, MTs, file)

def extract_gendf_pkza(gendf_path):
    """
    Read in and parse the contents of a GENDF file to construct the parent
        KZA. KZA values are defined as ZZAAAM, where ZZ is the isotope's
        atomic number, AAA is the mass number, and M is the isomeric state
        (0 if non-isomeric).
    
    Arguments:
        gendf_path (str): File path to the GENDF file being analyzed.
        dir (str): String identifying the directory from which the function is
            being called.
    
    Returns:
        pKZA (int): Parent KZA identifier.
    """

    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    Z, element, A = first_line.split('-')[:3]

    Z = int(Z)
    isomer_tag = 'm' if 'm' in A.lower() else 'n'
    M = 2 if 'n' in A.lower() else 1 if 'm' in A.lower() else 0
    A = int(A.lower().split(' ')[0].split(isomer_tag)[0])
    pKZA = (Z * 1000 + A) * 10 + M
    return pKZA

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

    return sigma_list[::-1]

def iterate_MTs(MTs, file_obj, mt_dict, pKZA):
    """
    Iterate through all of the MTs present in a given GENDF file to extract
        the necessary data to be able to run ALARA.
    
    Arguments:
        MTs (list of int): List of reaction types present in the GENDF file.
        file_obj (ENDFtk.tree.File): ENDFtk file object containing the
            contents for a specific material's cross-section data.
        mt_dict (dict): Dictionary formatted data structure for mt_table.csv
        pKZA (int): Parent KZA identifier.
    Returns:
        gendf_data (list of dict): List of dictionaries containing parent KZA
            daughter KZA values, emitted particles, counts of the number of
            non-zero groups in the Vitamin-J groupwise structure, and the
            cross-section values for those groups, organized by MT number.
    """

    gendf_data = []
    for MT in MTs:
        sigma_list = extract_cross_sections(file_obj, MT)
        gendf_data.append(
            {
                'Parent KZA'            :                                pKZA,
                'Daughter KZA'          :        pKZA + mt_dict[MT]['delKZA'],
                'Emitted Particles'     :    mt_dict[MT]['Emitted Particles'],
                'Non-Zero Groups'       :                     len(sigma_list),
                'Cross Sections'        :                          sigma_list
            }
        )

    return gendf_data