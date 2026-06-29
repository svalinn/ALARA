import tendl_processing as tp

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

def plot_single_nuc_rxn_xs(
    ax, element, A, emitted, continuous_dict={}, groupwise_dict={}
):
    """
    Create a plot for a singular nuclide/reaction's cross-sections vs. energy.
        Can be used to plot continuous TENDL data (not processed by ALARAJOY),
        alongside an arbitrary number of groupwise cross-sections according to
        the group structure in which they were converted. Groupwise and
        continuous data can be plotted individually if only one type is
        provided.

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

    return ax