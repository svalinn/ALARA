import numpy as np
import argparse
import yaml

def read_nuclib(nuclib, template_alara_inp):
    nuclib_lines = open(nuclib, 'r').readlines()  
    return nuclib_lines

def make_volume_block(nuclib_lines, volume):
    nuc_list = []
    vol_list = []
    for line_idx, line in enumerate(nuclib_lines):
        line = line.strip().split()
        if ':' in line[0]:
            nuc_list.append(line[0])
            vol_list.append(f'\t {volume}\t{line[0]}\n')
    return nuc_list, vol_list  

def make_mat_mix(nuc_list):
    load_list = []
    mix_list = []
    for nuc in nuc_list:
        load_list.append(f'\t{nuc}\t mix_{nuc}\n')
        mix_list.append(f'mixture mix_{nuc}\n\t element {nuc} 1 1.0 \nend \n')
    return load_list, mix_list    

def edit_template(vol_list, load_list, mix_list, template_alara_inp):
    temp_lines = []
    blocks = ['volume', 'mat_loading', 'mixture']
    line_lists = [vol_list, load_list, mix_list]
    new_inp = open('20y_all_nuc.inp', 'w')
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
    nuc_list, vol_list = make_volume_block(nuclib_lines, volume)
    load_list, mix_list = make_mat_mix(nuc_list)


    edit_template(vol_list, load_list, mix_list, template_alara_inp)



if __name__ == '__main__':
    main()    