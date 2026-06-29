import matplotlib.pyplot as plt
from pathlib import Path

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

def plot_single_nuc_rxn_xs(
    ax, element, A, emitted, tendl_dir,
    continuous_dict={}, groupwise_dict={}, img_ext='png'
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

    plt.savefig(set_plot_save_path(
        element, A, emitted, tendl_dir, groupwise_dict.keys(), img_ext
    ))

    return ax