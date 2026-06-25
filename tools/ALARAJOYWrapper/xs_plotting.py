def plot_single_nuc_rxn_xs(ax, single_rxn_dict, element, A, emitted):
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
        single_rxn_dict (dict): Nested dictionary keyed at the highest level
            by either the key "Continuous" for continuous TENDL data or the
            name of the group structure according to which an array of cross-
            sections were processed. The form of this data structure is as
            follows:

                {
                    'continuous'   : {
                        'xs'       : continuous_xs,
                        'energies' : continuous_energies
                    },
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
        element (str): Symbol of the element to which the nuclide being
            plotted belongs.
        A (str or int): Mass number for selected isonuclide.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
    """

    title = (
        f'Energy-Dependent Neutron Cross-Sections for ' \
        f'$^{{{A}}}${element}(n,{emitted}):\n'
    )

    for data_set, arrays in single_rxn_dict.items():
        if data_set == 'continuous':
            ax.plot(arrays['energies'], arrays['xs'], label='TENDL')
            title += 'TENDL (Continuous), '
        else:
            ax.stairs(arrays['xs'][::-1], arrays['energies'], label=data_set)

    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('Energy [eV]')
    ax.set_ylabel('Cross-Section [b]')
    ax.set_title(
        title
        + ', '.join([g for g in single_rxn_dict if g != 'continuous'])
        + ' (Groupwise)'
    )
    ax.grid()
    ax.legend()

    return ax