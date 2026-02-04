import numpy as np
import argparse

def read_out(output_path):
    with open(output_path, "r") as output_file:
        lines = output_file.readlines()
    line_idx = 0
    while not lines[line_idx].startswith("top_schedule 'top_sched':"):
        line_idx += 1  
    return lines[line_idx:]

def read_pulse_histories(out_lines):    
    '''
    Creates a dictionary with the name of the pulse history as the key, and a dictionary containing the number of pulses
    in all pulsing levels & the delay (in seconds) of all pulsing levels as the value. All pulse histories are stored
    regardless of usage in any schedule item.
    '''
    pulse_dict = {}
    for line_idx, line in enumerate(out_lines):
        if line.strip().startswith("pulse_history"):
            pulse_dict[line.split()[1].strip("':")] = {"num_pulses_all_levels": out_lines[line_idx+2].split(":")[1].strip(), 
                                                   "delay_seconds_all_levels": out_lines[line_idx+3].split(":")[1].strip()
                                                      }                                                                                         
    return pulse_dict

def make_sch_sub_dict(line):
    sch_sub_dict = {
    "sched_ph_name" : line.strip().split()[3],
    "sched_delay_dur" : line.strip().split()[5],
    "sched_delay_unit" : line.strip().split()[6]
                   }
    return sch_sub_dict

def make_pe_sub_dict(line):
    pe_sub_dict = {
    "pe_dur": line.split()[1], 
    "pe_dur_unit": line.split()[2], 
    "corr_ph_name": line.split()[4],
    "pe_delay_dur": line.split()[6],
    "pe_delay_unit": line.split()[7]
                  } 
    return pe_sub_dict

def make_nested_dict(lines):
    '''
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    '''
    sch_dict = {}
    last_upper_indent_level = {0: sch_dict}
    line_idx = 0
    while not lines[line_idx].startswith("pulse_history:"): # next section of output

        child_level = lines[line_idx].count('\t')
        newline_name = lines[line_idx].lstrip('\t').rstrip('\n')

        if newline_name.strip().split()[0] == 'schedule':
            counter = 1
            last_upper_indent_level[child_level][f'schedule {lines[line_idx].strip().split()[1]}'] = make_sch_sub_dict(lines[line_idx])
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][f'schedule {lines[line_idx].strip().split()[1]}']
            line_idx += 1

        elif newline_name.strip().split()[0] == 'pulse_entry:':
            last_upper_indent_level[child_level][f"{lines[line_idx].strip().split()[0]} num_{counter}_in_sched"] = make_pe_sub_dict(lines[line_idx])  
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][f"{lines[line_idx].strip().split()[0]} num_{counter}_in_sched"]
            counter += 1
            line_idx += 1        
        else: # for line with top schedule
            last_upper_indent_level[child_level][newline_name.split()[1].strip("':")] = {}
            last_upper_indent_level[child_level + 1] = last_upper_indent_level[child_level][newline_name.split()[1].strip("':")]           
            line_idx += 1
    return sch_dict

def search_for_match(top_dict, search_str):
    '''
    Takes a nested top dictionary and searches all sub-dictionaries for matches with a provided string.
    '''
    matches = []
    for key, value in top_dict.items():

        if key.startswith(search_str):
            matches.append(value)

        elif isinstance(value, dict):
            match_res = search_for_match(value, search_str)
            matches.extend(match_res)

    return matches

def convert_to_s(sch_dict):
    '''
    Defines multipliers to convert all durations and units in schedule/pulse entries to seconds. Changes the corresponding 
    entries in the dictionaries to reflect this change.
    '''
    unit_multiples = {
    'c' : 60 * 60 * 24 * 365 * 100,
    'y' : 60 * 60 * 24 * 365,
    'w' : 60 * 60 * 24 * 7,
    'd' : 60 * 60 * 24,
    'h' : 60 * 60,
    'm' : 60,
    's' : 1
                     }
    search_fields = ["schedule", "pulse_entry"]
    for search_field in search_fields:
        matches = search_for_match(sch_dict, search_field)
        for match in matches:
            if 'sched_delay_dur' in match: # if the entry is a schedule entry
                match['sched_delay_dur'] = float(match['sched_delay_dur']) * unit_multiples[match['sched_delay_unit']]
            elif 'pe_dur' in match: # if the entry is a pulse entry
                match['pe_dur'] = float(match['pe_dur']) * unit_multiples[match['pe_dur_unit']]
                match['pe_delay_dur'] = float(match['pe_delay_dur']) * unit_multiples[matches[0]['pe_delay_unit']]

            match['sched_delay_unit'] = match['pe_dur_unit'] = match['pe_delay_unit'] = 's' 
    return sch_dict

def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", required=True, type=str, help="path to file containing ALARA output")
    arg = parser.parse_args()  
    return arg.f              

def main():
    output_path = parse_arg()
    lines = read_out(output_path)

    pulse_dict = read_pulse_histories(lines)
    sch_dict = make_nested_dict(lines)

    sch_dict = convert_to_s(sch_dict)

if __name__ == '__main__':
    main()
