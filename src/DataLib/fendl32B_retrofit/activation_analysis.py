# Import packages
from logging_config import logger, LoggerWriter
import pandas as pd
from tendl_preprocessing import extract_cross_sections

def iterate_MTs(MTs, file_obj, mt_dict, pKZA):
    # Initialize lists
    cross_sections_by_MT = []
    emitted_particles_list = []
    dKZAs = []
    groups = []

    # Extract data for each MT
    for MT in MTs:
        sigma_list = extract_cross_sections(file_obj, MT)
        if not sigma_list:
            continue
        dKZA = pKZA - mt_dict[str(MT)]['delKZA']
        emitted_particles = mt_dict[str(MT)]['Emitted Particles']
        if dKZA is None:
            continue
        cross_sections_by_MT.append(sigma_list)
        dKZAs.append(dKZA)
        emitted_particles_list.append(emitted_particles)
        groups.append(len(sigma_list))

    
    # Store data in a Pandas DataFrame
    gendf_data = pd.DataFrame({
        'Parent KZA': [pKZA] * len(dKZAs),
        'Daughter KZA': dKZAs,
        'Emitted Particles': emitted_particles_list,
        'Non-Zero Groups' : groups,
        'Cross Sections': cross_sections_by_MT
    })

    return gendf_data