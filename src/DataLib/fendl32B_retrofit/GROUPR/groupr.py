import groupr_tools as grpt
import pandas as pd

# Call TENDL download function by user CLI input
element = input('Select element: ')
A = input('Select mass number: A = ')
endf_path = grpt.tendl_download(element, A, 'endf')
pendf_path = grpt.tendl_download(element, A, 'pendf')
print(f'ENDF file can be found at ./{endf_path}')
print(f'PENDF file can be found at ./{pendf_path}')

# Extract necessary MT and MAT data from the ENDF file
matb, MTs = grpt.endf_specs(endf_path)

# Write out the GROUPR input file
mt_table = pd.read_csv('./mt_table.csv')
card_deck = grpt.groupr_input(matb, MTs, element, A, mt_table)

# Run NJOY
grpt.run_njoy(endf_path, pendf_path, card_deck, element, A)