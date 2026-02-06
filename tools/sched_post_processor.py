import numpy as np
import argparse


def read_out(output_path):
    with open(output_path, "r") as output_file:
        lines = output_file.readlines()
    line_idx = 0
    while not lines[line_idx].startswith("top_schedule 'top_sched':"):
        line_idx += 1
    return lines[line_idx:]


def read_pulse_histories(lines):
    """
    Creates a dictionary with the name of the pulse history as the key, and a dictionary containing the number of pulses
    in all pulsing levels & the delay (in seconds) of all pulsing levels as the value. All pulse histories are stored
    regardless of usage in any schedule item.
    """
    pulse_dict = {}
    line_idx = 0
    while line_idx < len(lines):
        line = lines[line_idx].strip()
        if line.startswith("pulse_history"):
            num_pulse_line = lines[line_idx + 2].strip()
            delay_line = lines[line_idx + 3].strip()

            pulse_hist_name = line.split()[1].strip("':")
            num_pulses = num_pulse_line.split(":")[1]
            delays = delay_line.split(":")[1]

            pulse_dict[pulse_hist_name] = {
                "num_pulses_all_levels": num_pulses,
                "delay_seconds_all_levels": delays,
            }
            line_idx += 4
        else:
            line_idx += 1
    return pulse_dict


def make_sch_sub_dict(sch_line, unit_multipliers):
    sch_sub_dict = {
        "sched_ph_name": sch_line[3],
        "sched_delay_dur": float(sch_line[5]) * unit_multipliers[sch_line[6]],
        "sched_delay_unit": "s",
    }
    return sch_sub_dict


def make_pe_sub_dict(pe_line, unit_multipliers):
    pe_sub_dict = {
        "pe_dur": float(pe_line[1]) * unit_multipliers[pe_line[2]],
        "pe_dur_unit": "s",
        "corr_ph_name": pe_line[4],
        "pe_delay_dur": float(pe_line[6]) * unit_multipliers[pe_line[7]],
        "pe_delay_unit": "s",
    }
    return pe_sub_dict


def make_nested_dict(lines):
    """
    Constructs a hierarchy of dictionaries with a separate level for each additional tab found at the beginning of each line
    in the section of the output with schedule details.
    A sub-dictionary is created for each additional indented level.
    """
    unit_multipliers = {
        "c": 60 * 60 * 24 * 365 * 100,
        "y": 60 * 60 * 24 * 365,
        "w": 60 * 60 * 24 * 7,
        "d": 60 * 60 * 24,
        "h": 60 * 60,
        "m": 60,
        "s": 1,
    }
    sch_dict = {}
    sched_tree = {0: sch_dict}
    line_idx = 0
    counter_dict = {}
    # next section of output
    while not lines[line_idx].startswith("pulse_history:"):
        child_level = lines[line_idx].count("\t")
        child_level_line = lines[line_idx].strip().split()

        if child_level_line[0] == "schedule":
            sched_tree[child_level][f"schedule {child_level_line[1]}"] = (
                make_sch_sub_dict(child_level_line, unit_multipliers))
            sched_tree[
                child_level +
                1] = sched_tree[child_level][f"schedule {child_level_line[1]}"]
            line_idx += 1
            new_child_level = lines[line_idx].count("\t")
            counter = counter_dict[new_child_level] = 1

        elif child_level_line[0] == "pulse_entry:":
            counter = counter_dict[child_level]
            sched_tree[child_level][
                f"pulse_entry num_{counter}_in_sched"] = make_pe_sub_dict(
                    child_level_line, unit_multipliers)
            sched_tree[child_level + 1] = sched_tree[child_level][
                f"pulse_entry num_{counter}_in_sched"]
            counter_dict[child_level] = counter + 1
            line_idx += 1
        else:  # for line with top schedule
            sched_tree[child_level][child_level_line[1].strip("':")] = {}
            sched_tree[child_level + 1] = sched_tree[child_level][
                child_level_line[1].strip("':")]
            line_idx += 1
            new_child_level = lines[line_idx].count("\t")
            counter_dict[new_child_level] = 1
    return sch_dict


def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f",
                        required=True,
                        type=str,
                        help="path to file containing ALARA output")
    arg = parser.parse_args()
    return arg.f

def main():
    output_path = parse_arg()
    lines = read_out(output_path)

    pulse_dict = read_pulse_histories(lines)
    sch_dict = make_nested_dict(lines)

if __name__ == "__main__":
    main()
