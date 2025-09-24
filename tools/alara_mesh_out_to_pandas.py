import pyne.alara
import pyne.mesh
from pyne.material import Material
from pyne.material_library import MaterialLibrary
import openmc
import h5py
import pandas as pd
import numpy as np

# This script is designed to work with an ALARA flux that is discretized over a MOAB mesh object.
# Currently, this script takes 1 single material & element over the mesh, and 1 single irradation time in the 'schedule' block.

def open_files():
    inp_lines = open('alara_inp', 'r').readlines()    
    out_lines = open('alara_out', 'r').readlines() #ALARA output file from sdout
    mat_lib_lines = open('alara_matlib', 'r').readlines()
    flux_lines = open('alara_fluxin', 'r').readlines()
    mesh_file = "Mesh.h5"
    return inp_lines, out_lines, mat_lib_lines, flux_lines, mesh_file

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

def read_inp_schedule(inp_lines):
    for inp_index, inp_line in enumerate(inp_lines):
        if inp_line.strip().startswith("schedule"):
            schedule_line = inp_lines[inp_index+1].split()
            number = int(schedule_line[0])
            unit = schedule_line[1]
    return number, unit

def read_inp_mats(inp_lines, mat_lib_lines):
    for inp_index, inp_line in enumerate(inp_lines):
        if inp_line.strip().startswith("mixture"):
            mixture_line = inp_lines[inp_index+1].split()
            material = mixture_line[1]  
            break    
    for mat_lib_index, mat_lib_line in enumerate(mat_lib_lines):
        if mat_lib_line.strip().startswith(material):
            mat_line = mat_lib_lines[mat_lib_index + 1].split()
            element = mat_line[0]
            break    
    return element

def store_flux_lines(flux_lines):
    energy_bins = openmc.mgxs.GROUP_STRUCTURES['VITAMIN-J-175'] 
    bin_widths = []
    for bin_index in range(len(energy_bins) - 1):
        bin_width = energy_bins[bin_index + 1] - energy_bins[bin_index]
        bin_widths.append(bin_width)

    all_entries = []
    for line in flux_lines:
        if line.strip() == "":
            continue
        all_entries.extend(line.split())

    all_entries = np.array(all_entries, dtype=float)

    return bin_widths, all_entries

def normalize_flux_spectrum(all_entries, bin_widths, pyne_mesh):
    flux_array = all_entries.reshape(len(bin_widths), len(pyne_mesh.mats), order="F")
    total_flux = np.sum(all_entries)
    for mesh_idx in range(len(pyne_mesh.mats)):
        flux_array[:,mesh_idx] = (flux_array[:, mesh_idx] / bin_widths)  * (1 / total_flux)
    return flux_array    

def write_to_pd(pyne_mesh, number, unit, element, flux_array):
    #writes parent element, irradiation time, normalized flux, daughter nuclides, and number densities to pandas df
    children = []
    num_dens = []
    for mat in pyne_mesh.mats.values():
        children.append(list(mat.comp.keys()))
        num_dens.append(list(mat.comp.values()))
    df = {
        'parent':element,
        't_irr':str(number)+unit,
        'flux_norm':flux_array,
        'children': children,
        'num_dens':num_dens
    }
    #needs to include some time-varying flux magnitude

def main():
    inp_lines, out_lines, mat_lib_lines, flux_lines, mesh_file = open_files()
    pyne_mesh = make_mesh_num_density(out_lines, mesh_file)
    write_num_dens_hdf5(pyne_mesh)
    number, unit = read_inp_schedule(inp_lines)
    element = read_inp_mats(inp_lines, mat_lib_lines)
    bin_widths, all_entries = store_flux_lines(flux_lines)
    flux_array = normalize_flux_spectrum(all_entries, bin_widths, pyne_mesh)
    write_to_pd(pyne_mesh, number, unit, element, flux_array)

if __name__ == "__main__":
    main()
