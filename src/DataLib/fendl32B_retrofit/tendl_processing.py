# Import packages
import ENDFtk
import pandas as pd

# Initialize Data Frame in which to store all extracted data
cumulative_data = pd.DataFrame({
    'Parent KZA'            :       [],
    'Daughter KZA'          :       [],
    'Emitted Particles'     :       [],
    'Non-Zero Groups'       :       [],
    'Cross Sections'        :       []
    })

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
        gendf_data (pandas.core.frame.DataFrame): Pandas DataFrame containing
            parent KZA values, daughter KZA values, emitted particles,
            counts of the number of non-zero groups in the Vitamin-J groupwise
            structure, and the cross-section values for those groups.
    """

    cross_sections_by_MT    =    []
    emitted_particles_list  =    []
    dKZAs                   =    []
    groups                  =    []

    for MT in MTs:
        sigma_list = extract_cross_sections(file_obj, MT)
        dKZA = pKZA + mt_dict[MT]['delKZA']
        emitted_particles = mt_dict[MT]['Emitted Particles']
        cross_sections_by_MT.append(sigma_list)
        
        dKZAs.append(dKZA)
        emitted_particles_list.append(emitted_particles)
        groups.append(len(sigma_list))

    gendf_data = pd.DataFrame({
        'Parent KZA'            :       [pKZA] * len(dKZAs),
        'Daughter KZA'          :       dKZAs,
        'Emitted Particles'     :       emitted_particles_list,
        'Non-Zero Groups'       :       groups,
        'Cross Sections'        :       cross_sections_by_MT
    })

    return gendf_data