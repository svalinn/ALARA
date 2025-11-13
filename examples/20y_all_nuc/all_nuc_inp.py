import argparse
import yaml
import string

import sys
sys.path.insert(1, '..')

from template_object_maker import make_template_obj

# This script produces the ALARA input file 20y_all_nuc.inp in this directory, with a different element library and some formatting
# differences that may need to be addressed manually

def read_nuclib(nuclib):
    nuclib_lines = open(nuclib, 'r').readlines()  
    return nuclib_lines

def make_volume_block(nuclib_lines, volume):
    vol_lines = ""
    load_lines = ""
    mix_lines = ""
    for line in nuclib_lines:
        line = line.strip().split()
        nuc = line[0]
        if ':' in nuc:
            vol_lines += f'\t {volume}\t{nuc}\n'
            load_lines += f'\t{nuc}\t mix_{nuc}\n'
            mix_lines += f'mixture mix_{nuc}\n\t element {nuc} 1 1.0 \nend \n'
    return vol_lines, load_lines, mix_lines  

def edit_template(template_object, params, vol_lines, load_lines, mix_lines, new_inp_name):
    new_lines = template_object.substitute(
        volume_placeholder = vol_lines, 
        mat_loading_placeholder = load_lines, 
        mixtures_placeholder = mix_lines,
        material_lib = params['material_lib'],
        nuclib = params['nuclib'],
        alaralib = params['alaralib'],
        flux_file = params['flux_file'],
        dump_filepath = params['dump_filepath'],
        norm = params['norm'],
        schedule_time = params['schedule_time'],
        num_pulses = params['num_pulses'],
        schedule_units = params['schedule_units'],
        trunc_tol = params['trunc_tol']
        )    
    with open(new_inp_name, 'w') as new_inp:
        new_inp.write(new_lines)

#Define all inputs and execute functions:
#---------------------------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--yaml', default = "all_nuc_inp.yaml", help="Path (str) to YAML containing inputs")
    args = parser.parse_args()
    return args

def read_yaml(args):
    with open(args.yaml, 'r') as yaml_file:
        params = yaml.safe_load(yaml_file)
    return params

def main():
    args = parse_args()
    params = read_yaml(args)
    volume = params['volume']

    nuclib = params['nuclib']
    new_inp_name = params['new_inp_name']

    nuclib_lines = read_nuclib(nuclib)
    vol_lines, load_lines, mix_lines = make_volume_block(nuclib_lines, volume)

    template_object = make_template_obj()
    edit_template(template_object, params, vol_lines, load_lines, mix_lines, new_inp_name)

if __name__ == '__main__':
    main()