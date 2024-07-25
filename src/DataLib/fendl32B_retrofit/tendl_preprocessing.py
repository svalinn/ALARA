# Import packages
import ENDFtk

def extract_endf_specs(path):
    """
    Extract the material ID and MT numbers from an ENDF file.

    Arguments:
        path (str): File path to the selected ENDF file.
    
    Returns:
        endf_specs (list): List containing the following values from the file:
            matb (int): Unique material ID extracted from the file.
            MTs (list): List of reaction types (MT's) present in the file.
    """

    tape = ENDFtk.tree.Tape.from_file(path)
    matb = tape.material_numbers[0]
    # Set MF for cross sections
    xs_MF = 3
    file = tape.material(matb).file(xs_MF)
    # Extract the MT numbers that are present in the file
    MTs = [MT.MT for MT in file.sections.to_list()]

    endf_specs = [matb, MTs]
    
    return endf_specs