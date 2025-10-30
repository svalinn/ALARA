import pyne.alara
import pyne.mesh
from pyne.material import Material
from pyne.material_library import MaterialLibrary
import openmc
import h5py
import pandas as pd
import numpy as np
import argparse
from io import StringIO
import re
from datetime import timedelta

# This script takes an ALARA output file (and corresponding input, material library, and flux files) and writes the parent element,
# irradiation time, normalized flux, flux magnitude, daughter nuclides, and number densities to a pandas df

# This script is designed to work withs an ALARA flux that is discretized over a MOAB mesh object.
# Currently, this script takes 1 single material & element over each mesh element, and 1 single irradation time in the 'schedule' block.

def open_files():
    inp_lines = open('alara_inp_1w_test', 'r').readlines()    
    out_lines = open('1w_out_test', 'r').readlines() #ALARA output file from sdout
    flux_lines = open('../alara_fluxin', 'r').readlines()
    mesh_file = "../../Mesh.h5"
    return inp_lines, out_lines, flux_lines, mesh_file

def make_mesh_num_density(out_lines, mesh_file):
    pyne_mesh = pyne.mesh.Mesh(mesh=mesh_file)
    time = "shutdown" # don't need decay times as of now
    pyne.alara.num_density_to_mesh(out_lines, time, pyne_mesh)
    return pyne_mesh

def write_num_dens_hdf5(pyne_mesh):
    matlib = pyne.material_library.MaterialLibrary()
    for mat_id in pyne_mesh.mats.keys():
        matlib[mat_id] = pyne_mesh.mats[mat_id]
    matlib.write_hdf5('pyne_matlib.h5', h5_overwrite=True)

def find_total_tirr(out_lines):
    #assume 1 pulse for now & 0 delay between pulses
    tirr_num_unit = {}
    time_dict = {'s': 'seconds', 'm': 'minutes', 'h': 'hours', 'd': 'days', 'w': 'weeks', 'y': 'years', 'c': 'centuries'}
    for out_line in out_lines:
        if out_line.strip().startswith("pulse:"):
            #key = number, value = unit
            tirr_num_unit[out_line.strip().split()[1]] = out_line.strip().split()[2] #active pulse time
            #tirr_num_unit[out_line.strip().split()[4]] = out_line.strip().split()[5] #delay time  

    # Find the sum of all numbers with the same units
    total_time_dict = {}
    for u in time_dict.keys():
        sum_num = sum([float(number) for number, unit in tirr_num_unit.items() if unit == u and u != 'y' and u != 'c'])
        # Leap years not handled here
        sum_num_yd = 365*sum([float(number) for number, unit in tirr_num_unit.items() if unit == u and u == 'y']) #years to days
        sum_num_cd = 36500*sum([float(number) for number, unit in tirr_num_unit.items() if unit == u and u == 'c']) #centuries to days

        if u == 'y':
            total_time_dict['d'] = total_time_dict['d'] + sum_num_yd

        elif u == 'c':
            total_time_dict['d'] = total_time_dict['d'] + sum_num_cd

        else:
            total_time_dict[u] = sum_num # (total_time_dict['d'] is updated first due to how time_dict is set up)
            
    tot_time = timedelta(seconds=total_time_dict['s'], 
                         minutes=total_time_dict['m'], 
                         hours=total_time_dict['h'],
                         days=total_time_dict['d'],
                         weeks=total_time_dict['w'])
    number = tot_time.total_seconds()
    unit = list(total_time_dict.keys())[0]
    return number, unit
    
def read_mats(out_lines):
    #Track the chemical element in each mesh voxel based on the name of the corresponding zone (e.g. Zone #1: H)
    elements = []
    for out_line in out_lines:
        if out_line.strip().startswith("Zone #"):
            elements.append(out_line.strip().split()[2])
    return elements        

def store_flux_lines(flux_lines):
    energy_bins = openmc.mgxs.GROUP_STRUCTURES['VITAMIN-J-175'] 
    bin_widths = []
    for bin_index in range(len(energy_bins) - 1):
        bin_width = energy_bins[bin_index + 1] - energy_bins[bin_index]
        bin_widths.append(bin_width)

    all_entries = []
    for flux_line in flux_lines:
        if flux_line.strip(): #if the current line is not blank
            all_entries.extend(flux_line.split())
    all_entries = np.array(all_entries, dtype=float)
    return bin_widths, all_entries

def normalize_flux_spectrum(all_entries, bin_widths, pyne_mesh):
    flux_array = all_entries.reshape(len(pyne_mesh.mats), len(bin_widths))
    total_flux = np.sum(all_entries)
    for mesh_idx in range(len(pyne_mesh.mats)):
        flux_array[mesh_idx,:] = (flux_array[mesh_idx,:] / bin_widths)  * (1 / total_flux)
    return flux_array 

def find_avg_flux(all_entries, pyne_mesh, bin_widths, number):
    # find the average flux magnitude (over the irradiation time), for each mesh element
    # currently the same as the original flux as the magnitude of the flux does not change with time

    seconds = number # total pulse time
    fluence_per_mesh_element = (np.sum(all_entries.reshape(len(pyne_mesh.mats), len(bin_widths)), axis=1)) * seconds
    avg_flux_per_mesh_element = fluence_per_mesh_element / seconds
    return avg_flux_per_mesh_element

def write_to_pd(pyne_mesh, number, unit, elements, flux_array, avg_flux_per_mesh_element):
    #writes parent element, irradiation time, normalized flux, daughter nuclides, and number densities to pandas df
    children = []
    num_dens = []
    for mat in pyne_mesh.mats.values():
        children.append(list(mat.comp.keys()))
        num_dens.append(list(mat.comp.values()))
    dict = {
        'parents':elements,
        't_irr': list(((str(number)+unit).split()))*len(pyne_mesh.mats), # the irradiation schedule applies to all mesh elements
        'flux_norm':flux_array.tolist(),
        'avg_flux_mag':avg_flux_per_mesh_element,
        'children': children,
        'num_dens':num_dens
    }
    df = pd.DataFrame(dict)
    
    #needs to include some time-varying flux magnitude

#-------------------------------------------------------------------------------------------------    

def normalize_header(header_line=str) -> str:
    """
    Normalize table header lines by replacing spaces between numeric values
        and their units with underscores, allowing for pandas to interpret
        multi-word column names without splitting them into separate columns.
    
    Arguments:
        header_line (str): Raw text of header line read in from ALARA output
            file.
    
    Returns:
        normalized_header_line (str): Header line with spaces in number-unit 
            pairs replaced by underscroes (e.g., "1 d" -> "1_d").
    """
    
    return re.sub(r'(\d+)\s+([a-zA-Z]+)', r'\1_\2', header_line)

def sanitize_filename(name: str) -> str:
    """
    Convert a table name into a a valid string that can be used in a filepath
        by replacing invalid or special characters with undescores.

    Arguments:
        name (str): Raw descriptive name of the table, as stored in the
            key of the dictionary for that particular table.
    
    Returns:
        sanitized_name (str): Modified dictionary key name without invalid
            filepath characters.
    """
    
    return re.sub(r'[<>:"/\\|?*\[\]\(\)\s]+', '_', name)

def parse_table_data(current_table_lines, results,
               current_parameter, current_interval):
    """
    Parse a block of table lines with StringIO into a Pandas DataFrame
        and store in the results dictionary.
    
    Arguments:
        current_table_lines (list of str): Lines of the current table,
            each stored as a separate string.
        results (dict): Dictionary that stores all parsed tables,
            keyed by parameter and block name.
        current_parameter (str): Specific quantitative value represented
            in the table (e.g. specific activity, number density, etc.)
        current_interval (str): Interval iterated upon in ALARA run.

    Returns:
        None
    """

    df = pd.read_csv(
        StringIO('\n'.join(current_table_lines)),
        delim_whitespace=True
    )

    df.columns = [c.replace("_", " ") for c in df.columns]
    key = f'{current_parameter} - {current_interval}'
    results[key] = df

def process_alara_output(filename):
    """
    Reads an ALARA output file, identifies all data tables contained within,
        and stores each as a Pandas DataFrame in a dictionary.

    Arguments:
        filename (str): Path to the ALARA output file.

    Returns:
        results (dict): Dictionary that stores all parsed tables,
            keyed by parameter and block name.
    """

    results = {}
    with open(filename, "r") as f:
        lines = f.readlines()

    current_parameter = None
    current_interval = None
    inside_table = False
    current_table_lines = []

    for line in lines:
        stripped = line.strip()

        # Identify table parameters by '***' signature
        if stripped.startswith('***') and stripped.endswith('***'):
            current_parameter = stripped.strip('* ').strip()
            continue

        # Identify intervals
        if stripped.startswith(
            'Interval #'
            ) or stripped.startswith('Totals for all intervals'):
            current_interval = stripped.rstrip(':')
            continue

        # Table headers begin with 'isotope' -- start of tabular data
        if stripped.startswith('isotope'):
            inside_table = True
            current_table_lines = [normalize_header(stripped)]
            continue

        # Skip buffer line inserted by ALARA ('=====...')
        if inside_table and stripped.startswith('='):
            continue

        # Read actual tabular data
        if inside_table and (
            not stripped or stripped.startswith(
                '***'
            ) or stripped.startswith('Interval')
        ):
            if len(
                current_table_lines
            ) > 1 and current_parameter and current_interval:
                parse_table_data(current_table_lines, results,
                           current_parameter, current_interval
                )
            
            inside_table = False
            current_table_lines = []
            continue

        if inside_table:
            current_table_lines.append(stripped)

    # Read final table
    if inside_table and len(
        current_table_lines
    ) > 1 and current_parameter and current_interval:
        parse_table_data(current_table_lines, results,
                   current_parameter, current_interval
        )

    return results

#---------------------------------------------------------------------------------------------------------------------
#args() and main()

def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="cmd")
    subparsers.add_parser("mesh_based") #Choose if ALARA input & output are based on some mesh (structured/unstructured)

    meshless = subparsers.add_parser("meshless")
    meshless.add_argument("--filepath", '-f', required=True, nargs=1)

    args = parser.parse_args()
    return args

def main():

    args = parse_args()

    if args.cmd == "mesh_based":
        inp_lines, out_lines, flux_lines, mesh_file = open_files()
        elements = read_mats(out_lines)
        number, unit = find_total_tirr(out_lines)
        find_total_tirr(out_lines)
        pyne_mesh = make_mesh_num_density(out_lines, mesh_file)
        write_num_dens_hdf5(pyne_mesh)
        bin_widths, all_entries = store_flux_lines(flux_lines)
        flux_array = normalize_flux_spectrum(all_entries, bin_widths, pyne_mesh)
        avg_flux_per_mesh_element = find_avg_flux(all_entries, pyne_mesh, bin_widths, number, unit)
        write_to_pd(pyne_mesh, number, unit, elements, flux_array, avg_flux_per_mesh_element)    

    elif args.cmd == "meshless":
        filepath = args.meshless    

        alara_tables = process_alara_output(args().filepath[0])

        for key, df in alara_tables.items():
            filename = sanitize_filename(key) + '.csv'
            df.to_csv(filename, index=False)

    else:
        print("Please choose one of the following subparser commands: mesh_based, meshless")       

if __name__ == '__main__':
    main()