import argparse
import sched_post_processor as spp
import numpy as np
import matplotlib.pyplot as plt

# This script plots an irradiation history consisting of exactly 1 schedule with exactly 1 entry, with a corresponding
# pulse history that contains exactly 1 level.

def search_for_match(top_dict, search_str):
    '''
    Search a top dictionary with nested sub-dictionaries and lists
    and search for matches with a provided string.
    '''
    matches = []

    for key, value in top_dict.items():

        if key == search_str:
            matches.append(value)

        elif isinstance(value, dict):
            results = search_for_match(value, search_str)
            matches.extend(results)

        elif isinstance(value, list):
            for item in value:
                if isinstance(item, dict):
                    new_results = search_for_match(item, search_str)
                    matches.extend(new_results)

    return matches

def plot_sched_hist(num_pulses, pulse_length, pulse_dwell_time):
    dummy_amp = 1 # dummy flux magnitude/pulse amplitude
    tot_time = [0]
    rep_flux_mag = [0]
    time_position = 0

    for pulse_idx in range(num_pulses):
        # Rising edge of the pulse
        tot_time += [time_position, time_position]
        rep_flux_mag += [0, dummy_amp]

        # Falling edge of the pulse
        time_position += pulse_length

        tot_time += [time_position, time_position]
        rep_flux_mag += [dummy_amp, 0]

        # Delay between pulses
        time_position += pulse_dwell_time

    plt.step(tot_time, rep_flux_mag, where="post")
    plt.xlabel("Time (s)")
    plt.ylabel("Dummy flux amplitude")
    plt.ylim(-0.1, 1.2)
    plt.title("Representation of simple schedule with simple pulse history")
    plt.tight_layout()
    plt.grid(True)
    plt.show()
    plt.savefig('simple_sched_ph.png')


def parse_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f",
                        type=str,
                        required=True,
                        default="/filespace/a/asrajendra/research/test_dir/simple_test_out",
                        help="path to file containing ALARA output")
    arg = parser.parse_args()
    return arg.f

def main():
    output_path = parse_arg()
    lines = spp.read_out(output_path)

    pulse_dict = spp.read_pulse_histories(lines)
    sch_dict = spp.make_nested_dict(lines)

    pulse_length = search_for_match(sch_dict, 'pe_dur')[0]
    corr_ph_name = search_for_match(sch_dict, 'corr_ph_name')[0]
    ph_details = search_for_match(pulse_dict, corr_ph_name)[0]
    num_pulses = eval(ph_details['num_pulses_all_levels'])[0]
    pulse_dwell_time = eval(ph_details['delay_seconds_all_levels'])[0]

    plot_sched_hist(num_pulses, pulse_length, pulse_dwell_time)


if __name__ == "__main__":
    main()