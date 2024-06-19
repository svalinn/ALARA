# Import packages
import ENDFtk
import gendf_tools as GENDFtk
import pandas as pd
import sys
import subprocess
from logging_config import logger

# Load MT table
# Data for MT table collected from 
# https://www.oecd-nea.org/dbdata/data/manual-endf/endf102_MT.pdf
mt_table = GENDFtk.read_csv('mt_table.csv')

def main():
    
    # Parse command line arguments
    args = GENDFtk.fendl_args()

    # Set conditionals for local file input 
    if args.method == 'I':
        gendf_path = args.local_path
        pKZA = GENDFtk.gendf_pkza_extract(gendf_path)
    
    # Set conditionals for file download
    elif args.method == 'D':
        element = args.element
        A = args.A
        # Check isomeric state
        if 'm' not in A:
            gendf_path, pKZA = GENDFtk.gendf_download(element, A)
        else:
            # Use NJOY GROUPR to convert the isomer's TENDL 2017 data to a GENDF file
            sys.path.append('./GROUPR')
            import groupr_tools as GRPRtk

            # Download ENDF and PENDF files for the isomer
            endf_path = GRPRtk.tendl_download(element, A, 'endf')
            pendf_path = GRPRtk.tendl_download(element, A, 'pendf')

            # Extract necessary MT and MAT data from the ENDF file
            matb, MTs = GRPRtk.endf_specs(endf_path)
            
            # Write out the GROUPR input file
            card_deck = GRPRtk.groupr_input_file_format(matb, MTs, element, A, mt_table)
            GRPRtk.groupr_input_file_writer(card_deck, MTs)

            # Run NJOY with GROUPR to create a GENDF file for the isomer
            gendf_path = GRPRtk.run_njoy(card_deck, element, A)

            # Save pKZA value
            pKZA = GENDFtk.gendf_pkza_extract(gendf_path, M = 1)

            # Clean up repository from unnecessary intermediate files from GROUPR run
            groupr_files = ['groupr.inp', 'groupr.out', 'tape20', 'tape21', 'tape31']
            for file in groupr_files:
                subprocess.run(['rm', file])

    return gendf_path, pKZA

# Execute main() function based on arguments
if __name__ == '__main__':
    gendf_path, pKZA = main()

logger.info(f"GENDF file path: {gendf_path}")
logger.info(f"Parent KZA (pKZA): {pKZA}")

# Read in data with ENDFtk
tape = ENDFtk.tree.Tape.from_file(gendf_path)
mat_ids = tape.material_numbers
mat = mat_ids[0]
xs_MF = 3
file = tape.material(mat).file(xs_MF)

# Extract the MT numbers that are present in the file
MTs = [MT.MT for MT in file.sections.to_list()]

# Initialize lists
cross_sections_by_MT = []
emitted_particles_list = []
dKZAs = []

# Extract data for each MT
for MT in MTs:
    try:
        sigma_list = GENDFtk.extract_cross_sections(file, MT)
        if not sigma_list:
            continue
        dKZA, emitted_particles = GENDFtk.reaction_calculator(MT, mt_table, pKZA)
        if dKZA is None:
            continue
        cross_sections_by_MT.append(sigma_list)
        dKZAs.append(dKZA)
        emitted_particles_list.append(emitted_particles)
    except Exception as e:
        logger.error(f"Error processing MT {MT}: {e}")
        continue

# Store data in a Pandas DataFrame
gendf_data = pd.DataFrame({
    'Parent KZA': [pKZA] * len(dKZAs),
    'Daughter KZA': dKZAs,
    'Emitted Particles': emitted_particles_list,
    'Cross Sections': cross_sections_by_MT
})

# Save to CSV
gendf_data.to_csv('gendf_data.csv', index=False)
logger.info("Saved gendf_data.csv")
logger.info(gendf_data.head())