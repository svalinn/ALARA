# Import packages
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

    isomer_id = ''

    upper_case_letters = [char for char in stem if char.isupper()]
    lower_case_letters = [char for char in stem if char.islower()]
    numbers = [str(char) for char in stem if char.isdigit()]

    if len(lower_case_letters) == 0:
        lower_case_letters = ''
    elif len(lower_case_letters) > 1:
        isomer_id = lower_case_letters[-1]
        lower_case_letters = lower_case_letters[:-1]
    
    element = f'{upper_case_letters[0]}{lower_case_letters[0]}'
    A = ''.join(numbers) + isomer_id

    return element, A

def search_for_files(directory = '.'):
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

    directory = Path(directory)

    file_info = {}
    for file in directory.iterdir():
        if file.is_file():
            if file.suffix == '.endf' or file.suffix == '.tendl':
                endf_file = file
                pendf_file = Path(f'{endf_file.stem}.pendf')
                if pendf_file.is_file():
                    element, A = get_isotope(file.stem)
                    file_info[f'{element}{A}'] = {
                        'Element'       :                 element,
                        'Mass Number'   :                       A,
                        'File Paths'    : (endf_file, pendf_file)
                        }
                    
    return file_info