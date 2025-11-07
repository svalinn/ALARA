import numpy as np
import argparse
import yaml

# This script produces the ALARA input file 20y_all_nuc.inp in this directory, with a different element library and some formatting
# differences that may need to be addressed manually

def read_nuclib(nuclib, template_alara_inp):
    nuclib_lines = open(nuclib, 'r').readlines()  
    return nuclib_lines

def make_volume_block(nuclib_lines, volume):
    vol_list = []
    load_list = []
    mix_list = []
    for line in nuclib_lines:
        line = line.strip().split()
        nuc = line[0]
        if ':' in nuc:
            vol_list.append(f'\t {volume}\t{nuc}\n')
            load_list.append(f'\t{nuc}\t mix_{nuc}\n')
            mix_list.append(f'mixture mix_{nuc}\n\t element {nuc} 1 1.0 \nend \n')

    return vol_list, load_list, mix_list  

def edit_template(vol_list, load_list, mix_list, template_alara_inp):
    # Uses an existing ALARA input file (in this case ../singleElement_20y_inp/alara_inp_fe_20y) and appends all lines
    # starting from the material_lib line to a new input file.
    blocks = ['volume', 'mat_loading', 'mixture']
    line_lists = [vol_list, load_list, mix_list]
    new_inp = open('20y_all_nuc_test.inp', 'w')
    new_inp.write('geometry rectangular \n \n')
    for block_id, block in enumerate(line_lists):
        if block_id != 2:
            new_inp.write(f'{blocks[block_id]}\n')
        for line in line_lists[block_id]:       
            new_inp.write(line)
        if block_id != 2:    
            new_inp.write('end \n')   

    with open(template_alara_inp, 'r') as temp_file:
        inside_block = False
        for temp_line in temp_file:
            stripped_line = temp_line.strip()
            if stripped_line.startswith('material_lib'):
                inside_block = True
                new_inp.write(temp_line)
                continue
            if inside_block == True:
                new_inp.write(temp_line)
                

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

    nuclib = params['nuclib']
    template_alara_inp = params['template_alara_inp']
    volume = params['volume']

    nuclib_lines= read_nuclib(nuclib, template_alara_inp)
    vol_list, load_list, mix_list = make_volume_block(nuclib_lines, volume)

    edit_template(vol_list, load_list, mix_list, template_alara_inp)

if __name__ == '__main__':
    main()    