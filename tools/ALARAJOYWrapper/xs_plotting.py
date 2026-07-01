import re
import numpy as np
import tendl_processing as tp
import matplotlib.pyplot as plt
from pathlib import Path
from reaction_data import GAS_DF
from njoy_tools import load_external_group_struct

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

    re_match = re.match(r'^\d+', str(num))
    if not re_match:
        raise ValueError(
            f'Invalid flagged number {num}. Must be formatted with numeric ' \
            'characters before non-numeric characters.'
        )
    
    return int(re_match.group())

def extract_continuous_data(tendl_path, MT):
    """
    For a given nuclide and reaction, extract its continuous-energy cross-
        sections and corresponding energies from its original TENDL file.

    Arguments:
        tendl_path (pathlib._local.PosixPath): Path to the nuclide's original
            TENDL file.
        MT (int): Reaction identifying number.

    Returns:
        continuous_dict (dict, optional): Dictionary containing an individual
            nuclide's continous TENDL cross-sections and energies for a given
            reaction. Formatted as:
                {'xs' : continuous_xs, 'energies' : continous_energies}

            If the provided reaction type does not exist in the TENDL file
            (such as gas production totals, which are calculated by the data
            preprocessor), then the dictionary values will be stored as empty
            lists.
    """

    tendl_xs = []
    tendl_energies = []

    file, _ = tp.create_endf_file_obj(tendl_path, 3)
    MT = flagged_num_to_int(MT)
    if MT in [MT.MT for MT in file.sections]:
        section = file.section(MT).parse()
        tendl_xs = list(section.cross_sections)
        tendl_energies = list(section.energies)

    return {
        'xs'            :           tendl_xs,
        'energies'      :     tendl_energies
    }

def set_plot_save_path(
    element, A, emitted, tendl_dir, group_names, img_ext='png'
):
    """
    For a given reaction's cross-section plot produced by
        plot_single_nuc_rxn_xs(), ensure/create a directory storage structure
        of TENDL-LIBRARY -> ELEMENT -> NUCLIDE to write the path at which the
        plot can be saved by its reaction type within the nuclide-level sub-
        directory.

    Arguments:
        element (str): Symbol of the element to which the nuclide being
            plotted belongs.
        A (str or int): Mass number for selected isonuclide.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.
        emitted (str): Particle(s) emitted from a nuclear reaction.
        tendl_dir (pathlib._local.PosixPath or str): Path to the directory in
            which the original TENDL data from which the cross-section data is
            extracted or derived.
        group_names (array-like or str): Iterable of all group structures with
            data included in the plot. If only a single group structure has
            data being plotted, then this parameter can be a string of the
            singular group name.
        img_ext (str, optional): Option to set the image filetype for the plot
            to be saved, limited to Matplotlib filetypes: png, ps, pdf, svg.
            (Defaults to 'png')
    
    Returns:
        save_path (pathlib._local.PosixPath): Filepath for a given reaction
            plot of the format:
            
                TENDL_LIB_PLOTS/ELEMENT/NUCLIDE/RXN_WITH_GROUPS.img_ext

            For example, the Fe-56 (n,p) reaction plotting VITAMIN-J-175 group
                data based on the continuous TENDL-2017 data could have the
                filepath:

                CWD/tendl2017_plots/Fe/Fe56/Fe56_(n,p)_VITAMIN-J-175.png
    """

    if isinstance(group_names, str):
        group_names = [group_names]

    nuc = f'{element}{A}'
    nuc_dir = Path(f'{tendl_dir}_plots') / element / nuc
    nuc_dir.mkdir(parents=True, exist_ok=True)

    return nuc_dir / f'{nuc}_(n,{emitted})_{"_".join(group_names)}.{img_ext}'

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

    return {
        'xs'            :           tendl_xs,
        'energies'      :     tendl_energies
    }

def ensure_emission_specificity(emitted, dKZA):
    """
    Check an emitted particle string for a gas total production reaction
        signature ('x'). Given that each type of these reactions (MTs 203-207)
        is only saved in the groupwise preprocessing collection nested-
        dictionaries according in the general form, this function cross-checks
        with reaction_data.GAS_DF with the daughter nuclide's KZA identifier
        to clarify the specific gas total reaction in question, if applicable.

    Arguments:
        emitted (str): Particle(s) emitted from a nuclear reaction.
        dKZA (int or str): Unique KZA identifier for the residual nuclide of a
            given nuclear reaction.

    Returns:
        emitted (str): Updated emitted particle string clarifying the specific
            type of gas production total reaction, if applicable. Otherwise
            emitted is unchanged from the input.
    """

    if emitted == 'x':
        gas_type = GAS_DF.loc[GAS_DF['kza'] == int(dKZA), 'gas'].iat[0]
        emitted = emitted.upper() + gas_type

    return emitted

def extract_groupwise_data_from_DSV(dsv_list, KZA, MT):
    """
    Given a list of DSV files containing ALARAJOY-processed groupwise cross-
        sections, read through through each file to find the line
        corresponding to the nuclide, reaction pair provided as (KZA, MT).
        Compile these data to a dictionary of the form:
                {
                    'group_name_1' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                    ...
                    'group_name_n' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                }

    Arguments:
        dsv_list (list): List of paths to ALARAJOY-processed DSV files
            containing groupwise cross-sections converted from continuous
            energy TENDL files.
        KZA (int): Unique nuclide identifier of the form ZZZAAAM.
        MT (int or str): Unique reaction identifier.

    Returns:
        groupwise_dict (dict): Nested dictionary keyed at the highest level by
            the name of the group structure according to which an array of
            cross-sections were processed.
        emitted (str): Particle(s) emitted from the nuclear reaction
            corresponding to the MT number provided.
    """

    groupwise_dict = {}
    emitted = ''
    for dsv in dsv_list:
        with open(dsv, 'r') as f:
            dsv_lines = f.readlines()

        group_name = dsv_lines[0].split()[-1]
        _, energy_bounds = load_external_group_struct(group_name)

        for line in dsv_lines[1:-1]:
            rxn = line.split()
            dsv_pKZA, dsv_dKZA, dsv_MT, emitted = rxn[:4]
            emitted = ensure_emission_specificity(emitted, dsv_dKZA)

            if KZA == dsv_pKZA and MT == flagged_num_to_int(dsv_MT):
                groupwise_dict[group_name] = {
                    'xs'          :    np.array(rxn[4:]).astype(float),
                    'energies'    :    energy_bounds
                }
                break

    return groupwise_dict, ensure_emission_specificity(emitted)

def plot_single_nuc_rxn_xs(
    ax, element, A, emitted, tendl_dir,
    continuous_dict={}, groupwise_dict={}, saving=True, img_ext='png'
):
    """
    Create and save a plot for a singular nuclide/reaction's cross-sections
        vs. energy. Can be used to plot continuous TENDL data (not processed
        by ALARAJOY), alongside an arbitrary number of groupwise cross-
        sections according to the group structure in which they were
        converted. Groupwise and continuous data can be plotted individually
        if only one type is provided.

    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        element (str): Symbol of the element to which the nuclide being
            plotted belongs.
        A (str or int): Mass number for selected isonuclide.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.
        emitted (str): Particle(s) emitted from a nuclear reaction.
        continuous_dict (dict, optional): Dictionary containing an individual
            nuclide's continous TENDL cross-sections and energies for a given
            reaction. Formatted as:
                {'xs' : continuous_xs, 'energies' : continous_energies}

            (Defaults to {})
        groupwise_dict (dict, optional): Nested dictionary keyed at the
            highest level by the name of the group structure according to
            which an array of cross-sections were processed. The form of this
            data structure is as follows:
                {
                    'group_name_1' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                    ...
                    'group_name_n' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                }

            (Defaults to {})
        saving (bool, optional): Option to save the plot to an image file
            within a hierarchical directory structure of TENDL-LIBRARY ->
            ELEMENT -> NUCLIDE.
            (Defaults to True)
        img_ext (str, optional): Option to set the image filetype for the plot
            to be saved, limited to Matplotlib filetypes: png, ps, pdf, svg.
            (Defaults to 'png')

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
    """

    title = (
        f'Energy-Dependent Neutron Cross-Sections for ' \
        f'$^{{{A}}}${element}(n,{emitted}):\n'
    )

    # Conditionally plot continous data
    if continuous_dict:
        ax.plot(
            continuous_dict['energies'], continuous_dict['xs'], label='TENDL'
        )

    # Conditionally plot each group structure's data provided
    if groupwise_dict:
        for data_set, arrays in groupwise_dict.items():
            ax.stairs(arrays['xs'][::-1], arrays['energies'], label=data_set)
        
        title += ', '.join([g for g in groupwise_dict]) + ' (Groupwise)'

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('Energy [eV]')
    ax.set_ylabel('Cross-Section [b]')
    ax.set_title(title)
    ax.grid()
    ax.legend()

    save_path = ''
    if saving:
        save_path = set_plot_save_path(
            element, A, emitted, tendl_dir, groupwise_dict.keys(), img_ext
        )
        plt.savefig(save_path)

    return ax, save_path