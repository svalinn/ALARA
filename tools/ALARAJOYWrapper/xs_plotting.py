import matplotlib.pyplot as plt
import numpy as np
import argparse
from openmc.mgxs import GROUP_STRUCTURES
import njoy_tools as njt
import tendl_processing as tp
from collections import defaultdict, abc
from pathlib import Path
from pathlib._local import PosixPath 
import reaction_data as rxd

def check_all_tag(param):
    """
    For a given plotting parameter (mass number or reaction), identify if the
        stored value is `'all'`, as read in from the input `.yaml` file.

    Arguments:
        param (str or list): Object containing all values for either the mass
            number or reaction parameter. If multiple values are present, then
            `param` will necessarily be a list, otherwise it can be a string.

    Returns:
        is_all_tag (bool): Boolean descriptor of whether the given parameter
            corresponds to `'all'` being selected.
    """

    return (
        len(param) == 1
        and isinstance(param[0], str)
        and param[0].lower() == 'all'
    )

def flagged_num_to_int(num):
    """
    Convert numerical values that may be in string form containing additional
        tags (i.e. '26m' or '4*') left over from groupwise processing to their
        base integer form.

    num (int or str): Numerical value, which may be in string form with
        additional tags such as 'm' or '*'.

    num_int (int): Numerical value stripped of any non-numerical characters in
        integer form.
    """

    return int(''.join(char for char in str(num) if char.isdigit()))

def extract_continuous_data(tendl_path, MT):
    """
    For a given nuclide and reaction, extract its continuous-energy cross-
        sections and corresponding energies from its original TENDL file.

    Arguments:
        tendl_path (pathlib._local.PosixPath): Path to the nuclide's original
            TENDL file.
        MT (int): Reaction identifying number.

    Returns:
        tendl_xs (list): Continuous-energy cross-sections for a given
            nuclide-reaction combination from the original TENDL file. If the
            provided reaction type does not exist in the TENDL file (such as
            gas production totals, which are calculated by the data
            preprocessor), then tendl_xs will be empty.
        tendl_energies (list): Corresponding energies for the cross-sections
            for a given nuclide-reaction combination from the original TENDL
            file. If the provided reaction type does not exist in the TENDL
            file), then tendl_energies will be empty.
    """
    
    tendl_xs = []
    tendl_energies = []

    file, _ = tp.create_endf_file_obj(tendl_path, 3)
    MT = flagged_num_to_int(MT)
    if MT in [MT.MT for MT in file.sections]:
        section = file.section(MT).parse()
        tendl_xs = list(section.cross_sections)
        tendl_energies = list(section.energies)

    return tendl_xs, tendl_energies

def all_rxns_to_plotting_dict(all_rxns, tendl_dir):
    """
    Method to be called specifically within preprocess_fendl3.py to adapt the
        all_rxns dictionary structure to the element -> nuclide -> reaction
        hiearchical structure used by xs_plotting.py to produce and save
        cross-section plots.

    Arguments:
        all_rxns (collections.defaultdict): Hierarchical dictionary keyed by
            parent nuclides to store all reaction data, with structured as:
            {parent:
                {daughter:
                    {MT:
                        {
                            'emitted': (str of emitted particles)
                            'xsections': (array of groupwise XS)
                        }
                    }
                }    
            }
        tendl_path (pathlib._local.PosixPath): Path to the directory in which
            the TENDL data for the processing run is stored.

    Returns:
        plotting_dict (collections.defaultdict): Hiearchical dictionary keyed
            by element with a sub-dictionary of mass-number as the keys, and
            a set of reaction types (MTs) as the values. Additionally, at the
            highest level of keys, a special key "TENDL" has a value for the
            path to tendl_dir.
    """

    plotting_dict = defaultdict(lambda: defaultdict(set))
    plotting_dict['TENDL'] = tendl_dir

    for parent in all_rxns:
        element, A = tp.interpret_KZA(parent)
        for rxn in all_rxns[parent].values():
            MT = list(rxn.keys())[0]      
            plotting_dict[element][A].add(MT)

    return plotting_dict

def del_if_empty(directory):
    """
    Delete a directory if it does not contain any files within.

    Arguments:
        directory (pathlib._local.PosixPath): Path to a directory.

    Returns:
        None
    """

    if not any(directory.iterdir()):
        directory.rmdir()    

def plot_cross_sections(plotting_dict):
    """
    Plot the cross-sections for a collection of nuclides, reactions, and
        groupwise DSVs comparatively to the continuous-energy TENDL cross-
        sections. Groupwise cross-sections are plotted using the plt.stairs()
        function, bounded by the particular energy group structure in which
        the cross-section values weree produced. Plots are saved in a
        hiearchical directory strcture by the following form:
            TENDL_DATA_SOURCE
                |-> ELEMENT
                    |-> NUCLIDE
                        |-> REACTION_1
                        |-> ...
                        |-> REACTION_N

    Arguments:
        plotting_dict (collections.defaultdict or dict): Hiearchical
            dictionary keyed by element with a sub-dictionary of mass-number
            as the keys, and a set of reaction types (MTs) as the values.
            Additionally, at the highest level of keys, there are two special
            subdictionaries of the form:
                {
                    'TENDL' : /path/to/tendl_data/ ,
                    'DSV'   : list of all groupwise DSV data files
                }

        Returns:
            None
    """

    plt.rcParams.update({'figure.max_open_warning': 0})

    data_sets = plotting_dict.get('DSV', 'cumulative_gendf_data.dsv')
    if not isinstance(data_sets, list):
        data_sets = [data_sets]
    
    tendl_dir = plotting_dict.get('TENDL', Path('tendl2017'))
    if not isinstance(tendl_dir, PosixPath):
        tendl_dir = Path(tendl_dir)

    top_dir = Path(f'{tendl_dir}_plots')
    top_dir.mkdir(exist_ok=True)

    elements = (
        set(njt.elements) if 'all' in [key.lower() for key in plotting_dict]
        else set(plotting_dict) & set(njt.elements)
    )

    for element in elements:
        element_dir = top_dir / element
        element_dir.mkdir(exist_ok=True)

        element_dict = (
            plotting_dict[element]
            if element in plotting_dict
            else plotting_dict['all']
        )
        mass_nums = list(element_dict)

        if check_all_tag(mass_nums):
            # Create range of all known mass numbers for element-agnostic
            # iterable, up to second isomeric state (highest allowed in TENDL)
            mass_range = range(1, 295)
            mass_nums = list(mass_range)
            for iso in tp.ISOMERIC_STATES[:2]:
                mass_nums.extend([f'{a}{iso}' for a in mass_range])

        for A in mass_nums:
            bottom_dir = element_dir / f'{element}{A}'
            bottom_dir.mkdir(exist_ok=True)
            M = tp.ISOMERIC_STATES.find(str(A)[-1]) + 1
            kza = str((
                njt.elements[element] * 1000 + flagged_num_to_int(A)
            ) * 10 + M)

            MTs = (
                element_dict[A]
                if A in element_dict
                else element_dict['all']
            )

            if not isinstance(MTs, abc.Iterable) or isinstance(MTs, str):
                MTs = [MTs]
    
            if check_all_tag(MTs):
                MTs = rxd.process_mt_data(rxd.load_mt_table(
                    njt.set_directory() / 'mt_table.csv'
                )).keys()

            for MT in [flagged_num_to_int(MT) for MT in MTs]:
                fig, ax = plt.subplots(figsize=(10,6))
                stair_plot = None
                group_names = []

                for dsv in data_sets:
                    with open(dsv, 'r') as f:
                        dsv_lines = f.readlines()

                    group_name = dsv_lines[0].split()[-1]
                    group_names.append(group_name)

                    # If group structure is not contained in
                    # openmc.mgxs.GROUP_STRUCTURES, a reference file must
                    # exist in the CWD with a matching name to the group_name
                    # containing the group bounds explicitly
                    energy_bounds = GROUP_STRUCTURES.get(group_name, [])
                    if len(energy_bounds) == 0:
                        energy_bounds = sorted(np.loadtxt(group_name))

                    xs = np.array([])
                    for line in dsv_lines[1:-1]:
                        rxn = line.split()
                        pKZA, dKZA, dsv_MT, emitted = rxn[:4]
                        if emitted == 'x':
                            gas_type = rxd.GAS_DF.loc[
                                rxd.GAS_DF['kza'] == int(dKZA), 'gas'
                            ].iat[0]
                            emitted = emitted.upper() + gas_type

                        if kza == pKZA and MT == flagged_num_to_int(dsv_MT):
                            xs = np.array(rxn[4:]).astype(float)
                            break

                    if xs.size > 0:
                        stair_plot = ax.stairs(
                            xs[::-1], energy_bounds, label=group_name
                        )
                
                if stair_plot:
                    title = (
                        f'Energy-Dependent Neutron Cross-Sections for ' \
                        f'$^{{{A}}}${element}(n,{emitted}):\n'
                    )
                    
                    tendl_path = tendl_dir / f'{element}{A}.tendl'
                    if tendl_path.is_file():
                        tendl_xs, tendl_energies = extract_continuous_data(
                            tendl_path, MT
                        )
                    if tendl_xs and tendl_energies:
                        ax.plot(tendl_energies, tendl_xs, label='TENDL')
                        title += 'TENDL (Continuous), '

                    title += ', '.join(group_names) + ' (Groupwise)'

                    ax.set_xscale('log')
                    ax.set_yscale('log')
                    ax.set_xlabel('Energy [eV]')
                    ax.set_ylabel('Cross-Section [b]')
                    ax.set_title(title)
                    ax.grid()
                    ax.legend()
                    plt.savefig(
                        bottom_dir / (
                             f'{element}{A}_(n,{emitted})_' \
                             f'{'_'.join(group_names)}.png'
                        )
                    )
                    plt.close()

            # Remove nuclide-level directories that did not produce any plots
            del_if_empty(bottom_dir)

        # Remove element-level directories that did not produce any plots
        del_if_empty(element_dir)

    print(
        f'Cross-section plots saved to {top_dir}/, ' \
        'organized by element, nuclide.'
    )

def main():
    # Only load in yaml module when executing xs_plotting.py as a script,
    # to allow for it to be used within preprocess_fendl3.py without
    # additional necessary dependencies
    from yaml import safe_load

    parser = argparse.ArgumentParser()
    parser.add_argument('--yaml', '-y')
    args = parser.parse_args()

    with open(args.yaml, 'r') as f:
        parameter_dict = safe_load(f)

    plot_cross_sections(parameter_dict)

if __name__ == '__main__':
    main()