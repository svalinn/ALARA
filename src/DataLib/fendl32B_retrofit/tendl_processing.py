# Import packages
import ENDFtk

def extract_endf_specs(path):
    """
    Extract the material ID and MT numbers from an ENDF file.

    Arguments:
        path (str): File path to the selected ENDF file.
    
    Returns:
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
    
    return matb, MTs

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
    """

    with open(gendf_path, 'r') as f:
        first_line = f.readline()
    Z, element, A = first_line.split('-')[:3]
    
    Z = int(Z)
    M = 1 if 'm' in A.lower() else 0
    A = int(A.lower().split(' ')[0].split('m')[0])
    pKZA = (Z * 1000 + A) * 10 + M
    return pKZA
