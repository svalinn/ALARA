import numpy as np
import matplotlib.pyplot as plt

def read_out(output_path):
    with open(output_path, "r") as output_file:
        out_lines = output_file.readlines()
    return out_lines    

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

def read_top_schedule(out_lines):
    '''
    Assumes that the output contains exactly 1 schedule. Creates a dictionary with the order of the entries in the schedule
    as the key, and a dictionary containing the duration of the pulse entries, duration units, name of the corresponding
    pulse history, duration of the entry's delay, and units of the delay as the value.  
    '''
    top_sch_dict = {}
    counter = 1
    for line in out_lines:
        if line.strip().startswith("pulse_entry"):
            top_sch_dict[f"top_sched_entry_{counter}"] = {"entry_dur": line.split()[1], 
                                            "entry_dur_units": line.split()[2], 
                                            "corr_ph_name": line.split()[4],
                                            "delay_dur": line.split()[6],
                                            "delay_dur_units": line.split()[7]}
            counter += 1                                 
    return top_sch_dict

def convert_sch_seconds(top_sch_dict):
    '''
    Defines multipliers to convert all durations and units in schedule entries to seconds. Changes the corresponding 
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
    for value in top_sch_dict.values():      
        value["entry_dur"] = float(value["entry_dur"]) * unit_multiples[value["entry_dur_units"]]    
        value["delay_dur"] = float(value["delay_dur"]) * unit_multiples[value["delay_dur_units"]]   
        value["entry_dur_units"] = value["delay_dur_units"] = 's'
    return top_sch_dict     

def store_irr_times(pulse_dict, top_sch_dict):
    '''
    The cumulative total irradiation times (including delays) are stored in a numpy array.
    output:
        plotting_array: numpy array of shape (2 x n), where the 1st row is the cumulative times,
                        and the 2nd row represents dummy pulse heights (1 at the end of an active period,
                        and 0 at the end of a delay period).
    '''
    irr_times = []
    dummy_heights = []
    # time counter:
    t = 0
    for top_sch_dict_val_id, top_sch_dict_val in enumerate(top_sch_dict.values()):
        # check if it's the last entry in the schedule
        is_last_sched_entry = top_sch_dict_val_id == len(top_sch_dict.values()) - 1
        corr_ph_name = top_sch_dict_val["corr_ph_name"]

        # iterable of # of pulses and delay times for all levels in a pulse history
        num_pulses_all_levels = eval(pulse_dict[corr_ph_name]["num_pulses_all_levels"])
        delay_seconds_all_levels = eval(pulse_dict[corr_ph_name]["delay_seconds_all_levels"])

        for num_pulse_lvl, num_pulse in enumerate(num_pulses_all_levels):
            # check if it's the last level in the pulse history
            is_last_ph_level = num_pulse_lvl == len(num_pulses_all_levels) - 1
            for pulse in range(num_pulse):
                t += top_sch_dict_val["entry_dur"] # add active duration to counter
                irr_times.append(t) # add cumulative time to irr_times
                dummy_heights.append(1) # add 1 to dummy_heights to represent active irradiation time

                # pulse delays only applied to the first N-1 pulses in a pulse level**
                if pulse < num_pulse - 1:
                    t += delay_seconds_all_levels[num_pulse_lvl] # add pulse delay duration to counter
                    irr_times.append(t)
                    dummy_heights.append(0) # add 0 to dummy_heights to represent delay time 

                # schedule item delays only applied right before the start of the next entry in the schedule block
                # schedule item delays ignored if it's the last entry in the schedule block
                elif is_last_ph_level and not is_last_sched_entry:
                    t += top_sch_dict_val["delay_dur"] # add schedule delay duration to counter
                    irr_times.append(t)
                    dummy_heights.append(0)

    plotting_array = np.array((irr_times, dummy_heights),dtype=float)
    return plotting_array            

def plot_irr_hist(plotting_array): 
    ''' 
    Cumulative irradiation times are normalized by the total accumulated time. Results plotted in a bar-chart like figure.
    '''
    irr_times_norm = plotting_array[0,:] / plotting_array[0, -1:]
    dummy_heights = plotting_array[1,:]

    plt.figure(figsize=(12, 5))
    for height_idx in range(len(dummy_heights)):
        left = 0 if height_idx== 0 else irr_times_norm[height_idx-1]
        right = irr_times_norm[height_idx] 
        if dummy_heights[height_idx] == 1:
            # blank space to the right of the time period with dummy_height = 1
            # blue fill to the right of the time period with dummy_height = 0
            plt.fill_between([left, right], min(dummy_heights), max(dummy_heights), step='pre', color='blue', alpha=0.5)

    plt.xlim(0, 1)
    plt.ylim(0.0, 1.05)
    plt.xlabel("Normalized Total Irradiation Times (s)")
    plt.ylabel("Representative Pulse Height")
    plt.title("Visualization of Irradiation Histories")
    plt.tight_layout()
    plt.savefig('irr_hist.png')
    plt.show()

def main():
    output_path = 'alara_out'

    out_lines = read_out(output_path)
    top_sch_dict = read_top_schedule(out_lines)
    top_sch_dict = convert_sch_seconds(top_sch_dict)
    pulse_dict = read_pulse_histories(out_lines)
    plotting_array = store_irr_times(pulse_dict, top_sch_dict)
    plot_irr_hist(plotting_array)

if __name__ == '__main__':
    main()