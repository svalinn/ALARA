import re
import argparse
import numpy as np
import tendl_processing as tp
import njoy_tools as njt
import reaction_data as rxd
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.patches import StepPatch
from pathlib import Path
from openmc.data import Reaction, endf

def flagged_num_to_int(num):
    """
    Convert numerical values that may be in string form containing additional
        tags (i.e. '26m' or '4*') left over from groupwise processing to their
        base integer form. Retain the isomeric flag separately if it is
        formatted with '*'.

    Arguments:
        num (int or str): Numerical value, which may be in string form with
            additional tags such as 'm' or '*'.

    Returns:
        num_int (int): Numerical value stripped of any non-numerical
            characters in integer form.
        isomer_flag (str): Sequence of '*' values corresponding to the
            isomeric flag contained in the original value. Empty string if no
            instance of '*' contained in the original value.
    """

    num = str(num)
    re_match = re.match(r'^-?\d+', num)
    if not re_match:
        raise ValueError(
            f'Invalid flagged number {num}. Must be formatted with numeric ' \
            'characters before non-numeric characters.'
        )

    return int(re_match.group()), num.count('*')

def ensure_emission_specificity(dsv_data):
    """
    For all emitted particles in a column of a groupwise DSV loaded into a
        Pandas DataFrame, check the particle string for a gas total production
        reaction signature ('x'). Given that each type of these reactions (MTs
        203-207) is only saved in the groupwise preprocessing collection
        nested-dictionaries in the general form, this function cross-checks
        against reaction_data.GAS_DF with the daughter nuclide's KZA
        identifier to clarify the specific gas total reaction in question, if
        applicable.

    Arguments:
        dsv_data (pandas.core.frame.DataFrame): DataFrame containing all
            reaction row-level data from a `tendl_to_alarajoy` produced DSV.

    Returns:
        emitted_series (pandas.core.series.Series): Updated emitted column.
            Row values where the emitted particle string was not equal to 'x'
            are left unchanged. Gas total rows become 'X' + gas_type (e.g.,
            'Xp').
    """

    gas_map = rxd.GAS_DF.drop_duplicates('kza').set_index('kza')['gas']
    emitted = dsv_data['emitted'].copy()
    mask = (emitted == 'x')

    if mask.any():
        kzas = dsv_data.loc[mask, 'dKZA']
        gas_types = kzas.map(gas_map)

        missing = gas_types.isna()
        if missing.any():
            raise KeyError(
                f'No gas type found for KZAs: {kzas[missing].unique()}.'
            )

        emitted.loc[mask] = 'X' + gas_types

    return emitted

def parse_dsv_firstline(dsv):
    """
    Process the header of an ALARAJOY DSV file to a list of metadata values.

    Arguments:
        dsv (str or pathlib._local.PosixPath): Path to a provided DSV file.

    Returns:
        header (list): List of each metadata value in the DSV header.
    """

    with open(dsv, 'r') as f:
        header = f.readline().split()

    header[0] = int(header[0])

    return header

def load_dsvs_to_dfs(dsv_paths):
    """
    Given a list of DSV files, load in each file as a Pandas DataFrame and
        store each in a dictionary keyed by the DSV filepath.

    Arguments:
        dsv_paths (list): List of paths to all DSV files being loaded.

    Returns:
        dsv_dict (dict): Dictionary keyed by each DSV filepath containing a
            Pandas DataFrame for each parsed file as its value.
    """

    reaction_cols = {
        'pKZA'    : int,
        'dKZA'    : int,
        'MT'      : str,
        'emitted' : str
    }

    dsv_dict = dict()
    ref_nGroups = None
    ref_dsv = None
    for dsv in dsv_paths:
        nGroups = parse_dsv_firstline(dsv)[0]

        if ref_nGroups is None:
            ref_nGroups, ref_dsv = nGroups, dsv

        elif nGroups != ref_nGroups:
            raise ValueError(
                'Number of groups/group structures in provided DSV data ' \
                f'sets must be the same as each other.\n {ref_dsv!r} has ' \
                f'{ref_nGroups} groups, but {dsv!r} has {nGroups}.'
            )

        dsv_data = pd.read_csv(
            dsv, sep=r'\s+', skiprows=1, header=None, low_memory=False
        )
        xs_cols = {f'group_{i}' : float for i in range(1, (nGroups + 1))}
        col_dict = reaction_cols | xs_cols
        dsv_data.columns = list(col_dict)
        dsv_data = dsv_data[dsv_data['pKZA'] != -1]
        dsv_data = dsv_data.astype(col_dict)
        dsv_data['emitted'] = ensure_emission_specificity(dsv_data)

        dsv_dict[dsv] = dsv_data

    return dsv_dict

def extract_continuous_data(endf_obj, MT):
    """
    For a given nuclide and reaction, extract its continuous-energy cross-
        sections and corresponding energies from its original TENDL file.

    Arguments:
        endf_obj (openmc.data.endf.Evaluation): OpenMC parsed-ENDF object.
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

    energy_list = []
    xs_list = []
    MT, isomeric_state = flagged_num_to_int(MT)
    rxn_obj = Reaction.from_endf(endf_obj, MT)

    # For excitation reactions, calculate specific pathway reactions by
    # multiplying reaction multiplicities by MF3 cumulative cross-sections
    # interpolated by the multiplicities' energy array if MF9 data is
    # present. Otherwise, extract pathway-specific cross-sections directly
    # from MF10
    if isomeric_state > 0:
        pathways, matched_MF = tp.collect_excitation_pathways(endf_obj, MT)
        if pathways and isomeric_state in pathways:
            tab1 = pathways[isomeric_state][1]
            energies = tab1.x
            energy_list.extend(energies)

            # Calculate interpolated proportional cross-section from MF3
            # cumulative cross-sections and MF9 multiplicities
            if matched_MF == 9:
                xs_list.extend(tab1.y * rxn_obj.xs['0K'](energies))

            # MF10 cross-sections can be extracted directly without need for
            # interpolation
            else:
                xs_list.extend(tab1.y)

    else:
        mf3_xs_table = rxn_obj.xs.get('0K')
        if mf3_xs_table:
            energy_list.extend(mf3_xs_table.x)
            xs_list.extend(mf3_xs_table.y)

    return {
        'energies' : np.asarray(energy_list, dtype=float),
        'xs'       : np.asarray(xs_list    , dtype=float)
    }

def extract_groupwise_data_from_DSV(dsv_dict, KZA, MT):
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
            the name of the group structure/weight function combination,
            according to which an array of cross-sections were processed.
        emitted (str): Particle(s) emitted from the nuclear reaction
            corresponding to the MT number provided.
    """

    emitted = ''
    groupwise_dict = {}
    for dsv, dsv_data in dsv_dict.items():
        (
            nGroups,
            tendl_version,
            group_name,
            weight_function,
            processing_code,
            decay_lib
        ) = parse_dsv_firstline(dsv)
        _, energy_bounds = njt.load_external_group_struct(group_name)

        data_id = group_name + ' ('
        if processing_code != 'NJOY':
            data_id += f'{processing_code}, '
        data_id += f'{weight_function} weight function)'

        rxn = dsv_data.loc[
            (dsv_data['pKZA'] == KZA) & (dsv_data['MT'] == str(MT))
        ]
        if not rxn.empty:
            emitted = rxn['emitted'].iat[0]
            groupwise_dict[data_id] = {
                'xs'         :    rxn.filter(like='group').to_numpy()[0],
                'energies'   :    energy_bounds
            }

    return groupwise_dict, emitted

def plot_neutron_energy_axvline(ax, energy_vline_arg, parser=None):
    """
    Given a pre-exisisting Matplotlib Axes object, include a vertical line at
        a specified energy to highlight the cross-section data in that region.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        energy_vline_arg (list of str): List specifying the neutron energy to
            denote with a vertical line. If only a single value is provided,
            then eV units will be assumed. Otherwise, any SI unit prefix can
            be supplied as the second value in the list, to be converted
            internally to eV (i.e. ['14.1', 'MeV'] will be converted to
            1.41e+07 eV).
        parser (argparser.ArgumentParser, optional): Option to include the
            argparser ArgumentParser object through which to channel
            improperly supplied `energy_vline_arg` values. Needed if being
            called within `main()`.
            (Defaults to None)

    Returns:
        energy_units (str): Scaled units of eV, extracted from the second
            value in energy_vline_arg.
    """

    vline_energy = float(energy_vline_arg[0])
    if len(energy_vline_arg) == 2:
        energy_units = energy_vline_arg[1]
    elif len(energy_vline_arg) > 2:
        error_message = (
            'Too many values for energy_vline. ' \
            'Can provide up to two values: energy, energy_units'
        )
        if parser:
            parser.error(error_message)
        else:
            raise TypeError(error_message)

    ax.axvline(
        vline_energy,
        color='r', 
        alpha=0.4, 
        label=f'{vline_energy:.2e} {energy_units}',
        linestyle='--'
    )

    return energy_units

def convert_energy_array(energy_array, to_unit='eV'):
    """
    Convert an array of energy values from electron-volts a specified SI
        multiple (i.e. MeV). If no units are provided, array returned as is.

    Arguments:
        energy_array (numpy.ndarray): Array of energy values in eV.
        to_unit (str, optional): SI energy unit by which to convert array.
            Accepted values range from aeV to EeV (1e-18 eV - 1e18 eV).
            (Defaults to 'eV')

    Returns:
        converted_array (numpy.ndarray): Array of energy values converted to
            the specified units.
    """

    if to_unit == 'eV':
        return energy_array

    conversion_dict = {
        'a' : 1e-18,
        'f' : 1e-15,
        'p' : 1e-12,
        'n' : 1e-9,
        'u' : 1e-6, # Accepts u or "μ" for micro-
        'μ' : 1e-6, # Accepts u or "μ" for micro-
        'm' : 1e-3,
        'c' : 1e-2,
        'k' : 1e3,
        'M' : 1e6,
        'G' : 1e9,
        'T' : 1e12,
        'P' : 1e15,
        'E' : 1e18
    }

    return energy_array / conversion_dict[to_unit[0]]

def set_plot_save_path(
    element, A, emitted, tendl_dir, group_names,
    img_ext='png', ratio_plotting=False, reaction_rates=False
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
        ratio_plotting (bool, optional): Option to specify the path for
            plotting the ratio series between different group structures.
            (Defaults to False)
        reaction_rates (bool, optional): Option to specify a path for plotting
            reaction rates per nucleus vs neutron energy rather than cross-
            section.
            (Defaults to False)
    
    Returns:
        save_path (pathlib._local.PosixPath): Filepath for a given reaction
            plot of the format:
            
                TENDL_LIB_PLOTS/ELEMENT/NUCLIDE/RXN_WITH_GROUPS.img_ext

            For example, the Fe-56 (n,p) reaction plotting VITAMIN-J-175 group
                data based on the continuous TENDL-2017 data could have the
                filepath:

                CWD/tendl2017_plots/Fe/Fe56/Fe56_(n,p)_VITAMIN-J-175.png

            Likewise, the same nuclide/reaction pair plotting the cross-
                section ratios between VITAMIN-J-175 and CCFE-709 groupwise
                data could produce this filepath:

                CWD/tendl2017_plots/Fe/Fe56/ratio_plots/Fe56_(n,p)_
                VITAMIN-J-175_CCFE-709_ratios.png
    """

    if isinstance(group_names, str):
        group_names = [group_names]

    group_names = [g.replace(' ', '_').replace('/','') for g in group_names]

    nuc = f'{element}{A}'
    nuc_dir = Path(f'{tendl_dir}_plots') / element / nuc
    stem_suffix = ''
    if reaction_rates:
        stem_suffix += '_reaction_rates'

    if ratio_plotting:
        nuc_dir /= 'ratio_plots'
        stem_suffix += '_ratios'

    nuc_dir.mkdir(parents=True, exist_ok=True)

    return (
        nuc_dir / f'{nuc}_(n,{emitted})_{"_".join(group_names)}{stem_suffix}'
    ).with_suffix(f'.{img_ext}')

def set_plot_parameters(
    ax, title, ratio_plotting=False, reaction_rates=False, energy_units='eV'
):
    """
    Apply standard plotting parameters for either of the two types of plots
        producable by `xs_plotting`: `plot_single_nuc_rxn_xs()` or
        `plot_relative_group_xs()`.

    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        title (str): Plot title.
        ratio_plotting (bool, optional): Option to produce a ratio series plot
            comparing different group structures' cross-sections with
            `plot_relative_group_xs()`.
            (Defaults to False)
        reaction_rates (bool, optional): Option to produce a plot of reaction
            rates per nucleus vs neutron energy for a given neutron flux
            spectrum, calculated from the groupwise cross-sections.
            (Defaults to False)
        energy_units (str, optional): Option to specify the energy units to be
            represented in the xlabel.
            (Defaults to 'eV')

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
    """

    ylabel = 'Cross-Section [b]'
    if reaction_rates:
        ylabel = 'Reaction Rate per Nucleus [1/s]'

    if ratio_plotting:
        ylabel = f'Ratio of {ylabel}'.split(' [')[0]

    if np.log10(np.ptp(ax.get_ylim())) > 1:
        ax.set_yscale('log')
    ax.set_xscale('log')

    ax.set_xlabel(f'Energy [{energy_units}]')
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid()
    ax.legend()

    return ax

def _is_rxn_rate(fluxes):
    """
    Boolean check to see if a reaction-rate plot is to be produced, dependent
        on having an array of flux values with non-zero length.
    
    Arguments:
        fluxes (array-like): Array of groupwise flux values. If an array with
            non-zero length is provided, then return True.

    Returns:
        reaction_rate (bool): Boolean variable to denote whether reaction-rate
            plots are to be produced.
    """

    return len(fluxes) > 0

def _xs_to_rxn_rate(xs, fluxes):
    """
    Formula to calculate reaction rates per nucleus from groupwise cross-
        sections and fluxes. Converts from units of b/cm^2-s to 1/s.

    Arguments:
        xs (numpy.ndarray): 1-D array of groupwise cross-sections in barns.
        fluxes (numpy.ndarray): 1-D array of groupwise neutron flux values in
            1/cm^2-s.
    
    Returns:
        reaction_rates (numpy.ndarray): 1-D array of reaction rates per
            nucleus in 1/s.
    """

    return xs * fluxes * 1e-24

def plot_single_nuc_rxn_data(
    ax, element, A, emitted,
    continuous_dict={}, groupwise_dict={}, fluxes=[], energy_units='eV'
):
    """
    Create a plot for a singular nuclide/reaction's cross-sections vs. energy.
        Can be used to plot continuous TENDL data (not processed by ALARAJOY),
        alongside an arbitrary number of groupwise cross-sections according to
        the group structure in which they were converted. Groupwise and
        continuous data can be plotted individually if only one type is provided.

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
            highest level by the name of the group structure/weight function
            combination, according to which an array of cross-sections were
            processed. The form of this data structure is as follows:
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
        fluxes (array-like, optional): Optional argument to provide the
            groupwise neutron flux values to calculate reaction rates per
            nucleus.
            (Defaults to [])
        energy_units (str, optional): Option to specify the energy units by
            which to represent groupwise energies.
            (Defaults to 'eV')

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
    """

    title = (
        f'Energy-Dependent Neutron Cross-Sections for ' \
        f'$^{{{A}}}${element}(n,{emitted}):\n'
    )
    reaction_rates = _is_rxn_rate(fluxes)
    if reaction_rates:
        title = title.replace(
            'Neutron Cross-Sections', 'Reaction Rates per Nucleus'
        )
        if not isinstance(fluxes, np.ndarray):
            fluxes = np.asarray(fluxes, dtype=float)

    # Conditionally plot continous data
    if continuous_dict and not reaction_rates:
        ax.plot(
            convert_energy_array(continuous_dict['energies'], energy_units),
            continuous_dict['xs'],
            label='TENDL'
        )

    # Conditionally plot each group structure's data provided
    if groupwise_dict:
        for data_set, arrays in groupwise_dict.items():
            values = arrays['xs'][::-1]
            if reaction_rates:
                values = _xs_to_rxn_rate(values, fluxes)

            ax.stairs(
                values, 
                convert_energy_array(arrays['energies'], energy_units),
                label=data_set
            )

        title += ', '.join([g for g in groupwise_dict]) + ' (Groupwise)'

<<<<<<< HEAD
    return set_plot_parameters(
        ax, title, reaction_rates=reaction_rates, energy_units=energy_units
=======
    return set_plot_parameters(ax, title, reaction_rates=reaction_rates)

def convert_to_eV(energy_vline_arg, parser=None):
    """
    Convert an energy value to units of electron-volts, if it is specified to
        be in some SI multiple of eV (i.e. MeV).

    Arguments:
        energy_vline_arg (list of str): List specifying the neutron energy to
            denote with a vertical line. If only a single value is provided,
            then eV units will be assumed. Otherwise, any SI unit prefix can
            be supplied as the second value in the list, to be converted
            internally to eV (i.e. ['14.1', 'MeV'] will be converted to
            1.41e+07 eV).
        parser (argparser.ArgumentParser, optional): Option to include the
            argparser ArgumentParser object through which to channel
            improperly supplied `energy_vline_arg` values. Needed if being
            called within `main()`.
            (Defaults to None)

    Returns:
        converted_energy (float): Energy value in electron-volts      
    """

    conversion_dict = {
        'a' : 1e-18,
        'f' : 1e-15,
        'p' : 1e-12,
        'n' : 1e-9,
        'u' : 1e-6, # Accepts u or "μ" for micro-
        'μ' : 1e-6, # Accepts u or "μ" for micro-
        'm' : 1e-3,
        'c' : 1e-2,
        'k' : 1e3,
        'M' : 1e6,
        'G' : 1e9,
        'T' : 1e12,
        'P' : 1e15,
        'E' : 1e18
    }

    vline_energy = float(energy_vline_arg[0])
    if len(energy_vline_arg) == 2:
        energy_units = energy_vline_arg[1]
    elif len(energy_vline_arg) > 2:
        error_message = (
            'Too many values for energy_vline. ' \
            'Can provide upt to two values: energy, energy_units'
        )
        if parser:
            parser.error(error_message)
        else:
            raise TypeError(error_message)

    if energy_units and energy_units.lower() != 'eV':
        vline_energy *= conversion_dict[energy_units[0]]

    return vline_energy

def plot_neutron_energy_axvline(ax, energy_vline_arg, parser=None):
    """
    Given a pre-exisisting Matplotlib Axes object, include a vertical line at
        a specified energy to highlight the cross-section data in that region.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        energy_vline_arg (list of str): List specifying the neutron energy to
            denote with a vertical line. If only a single value is provided,
            then eV units will be assumed. Otherwise, any SI unit prefix can
            be supplied as the second value in the list, to be converted
            internally to eV (i.e. ['14.1', 'MeV'] will be converted to
            1.41e+07 eV).
        parser (argparser.ArgumentParser, optional): Option to include the
            argparser ArgumentParser object through which to channel
            improperly supplied `energy_vline_arg` values. Needed if being
            called within `main()`.
            (Defaults to None)

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
    """

    vline_energy = convert_to_eV(energy_vline_arg, parser)
    ax.axvline(
        vline_energy,
        color='r', 
        alpha=0.4, 
        label=f'{vline_energy:.2e} eV',
        linestyle='--'
>>>>>>> parent of 4a6b6b8 (Correcting typo)
    )

def collect_all_stair_colors(ax):
    """
    From a Matplotlib Axes object, determine the series colors of all
        matplotlib.patches.StepPatch subplots, namely the `stair` plots for
        groupwise data (as opposed to `plot` plots for continuous data).
        Organize the RGBA color data into a dictionary keyed by each group
        structure, with values of the tuple of 0-1 RGBA values definining the
        color/transparency of each series.

    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of an already-
            created plot with series corresponding to different group
            structures' cross-section data.

    Returns:
        color_dict (dict): Dictionary keyed by each group structure, with
            values of the tuple of 0-1 RGBA values defining the color/
            transparency of each series.
    """

    handles, labels = ax.get_legend_handles_labels()
    color_dict = dict()
    for handle, label in zip(handles, labels):
        if isinstance(handle, StepPatch):
            color_dict[label] = handle.get_edgecolor()

    return color_dict

def compute_groupwise_ratios(groupwise_dict, reference_data):
    """
    Calculate the ratios of groupwise cross-sections for each group in
        `groupwise_dict` that shares a group structure as a single reference
        series. `ValueError` is raised if no other groupwise data matches the
        group structure of the reference data set. This ratio series is
        equivalent for a reaction rates plot as both sets of cross-sections
        are multiplied by the same flux constants.

    Arguments:
        groupwise_dict (dict): Nested dictionary keyed at the highest level by
            the name of the group structure/weight function combination,
            according to which an array of cross-sections were processed.
        reference_data (str): Stem of the reference data DSV path. Will be the
            same as `reference_data` from Arguments if one is supplied,
            otherwise, will be the group name of the first key in
            `groupwise_dict`.

    Returns:
        ratio_dict (dict): Nested dictionary keyed at the highest level by
            the name of the group structure according to which an array of
            cross-sections were processed. Similar to `groupwise_dict`, but
            without a designated key for the reference data set name, as all
            other group names are implicitly refering to that group's cross-
            section (or reaction rate) data divided by the reference data's.
            All data must be of the same group structure.
    """

    ref_xs, ref_energies = groupwise_dict[reference_data].values()
    ratio_dict = {}
    for name, group_data in groupwise_dict.items():
        if name == reference_data:
            continue

        group_xs, group_energies = group_data.values()
        if np.allclose(ref_energies, group_energies, rtol=1e-5):
            values = np.divide(
                group_xs, ref_xs,
                out=np.full_like(ref_xs, np.nan, dtype=float),
                where=(ref_xs != 0)
            )[::-1]
            ratio_dict[name] = {'ratios' : values, 'energies' : ref_energies}

    if not ratio_dict:
        raise ValueError(
            'No groupwise cross-sections provided with the same group ' \
            f'structure as the reference structure in "{reference_data}".'
        )

    return ratio_dict

def plot_relative_energy_dependent_func(       
    ax, element, A, MT, emitted, groupwise_dict, reference_data, color_dict,
    ratio_stats_df, x_limits=(None, None), fluxes=[], energy_units='eV'
):
    """
    Create a plot of the ratio series of groupwise cross-sections or reaction
    rates relative to a reference group structure's energy-dependent values.

    Arguments: 
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        element (str): Symbol of the element to which the nuclide being
            plotted belongs.
        A (str or int): Mass number for selected isonuclide.
            If the target is a metastable isomer, "m" or "n" is written after 
            the mass number, corresponding to the first or second metastable
            states.
        MT (int or str): Unique reaction identifier.
        emitted (str): Particle(s) emitted from a nuclear reaction.
        continuous_dict (dict, optional): Dictionary containing an individual
            nuclide's continous TENDL cross-sections and energies for a given
            reaction. Formatted as:
                {'xs' : continuous_xs, 'energies' : continous_energies}

            (Defaults to {})
        groupwise_dict (dict): Nested dictionary keyed at the highest level by
            the name of the group structure/weight function combination,
            according to which an array of cross-sections were processed. The
            form of this data structure is as follows:
                {
                    'data_set_1' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                    ...
                    'data_set_n' : {
                        'xs'       : groupwise_xs,
                        'energies' : energy_group_bounds
                    },
                }
        reference_data (str): Stem of the reference data DSV path. Will be the
            same as `reference_data` from Arguments if one is supplied,
            otherwise, will be the group name of the first key in
            `groupwise_dict`.
        color_dict (dict): Dictionary keyed by each group structure, with
            values of the tuple of 0-1 RGBA values defining the color/
            transparency of each series.
        ratio_stats_df (pandas.DataFrame): DataFrame containing mean and
            standard deviation statistics for each ratio plot produced.
        xlimits (tuple, optional): Option to specify the x-axis limits for
            the plot.
            (Defaults to (None, None))
        fluxes (array-like, optional): Optional argument to provide the
            groupwise neutron flux values to calculate ratios of reaction
            rates per nucleus.
            (Defaults to [])
        energy_units (str, optional): Option to specify the energy units by
            which to represent groupwise energies.
            (Defaults to 'eV')

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Matplotlib Axes object of the
            plot being constructed.
        ratio_stats_df (pandas.DataFrame): Updated DataFrame containing mean
            and standard deviation statistics for each ratio plot produced.
    """

    if len(groupwise_dict) < 2:
        raise ValueError(
            'At least two groupwise DSV files with equivalent group' \
            'structures are required to compute a relative series.'
        )

    reaction_rates = _is_rxn_rate(fluxes)
    ratio_dict = compute_groupwise_ratios(groupwise_dict, reference_data)

    for name, group_data in ratio_dict.items():
        comparison = f'{name} /\n {reference_data}'
        ratio_mean = np.nanmean(group_data['ratios'])
        ratio_std = np.nanstd(group_data['ratios'])
        ratio_stats_df.loc[len(ratio_stats_df)] = [
            element, A, MT, comparison, ratio_mean, ratio_std
        ]
        label = (
            comparison
            + f'\n$\\mu = {ratio_mean:.3f},'
            + f'\\ \\sigma = {ratio_std:.3e}$\n'
        )
        ax.stairs(
            group_data['ratios'],
            convert_energy_array(group_data['energies'], energy_units),
            baseline=None,
            label=label,
            color=np.mean(
                [color_dict[name], color_dict[reference_data]], axis=0
            )
        )

    ax.set_xlim(x_limits)
    title = ( 
        f'Relative Cross-Sections for $^{{{A}}}${element}(n,{emitted}):\n'
        f'Reference Data = {reference_data}'
    )
    if reaction_rates:
        title = title.replace('Cross-Section', 'Reaction Rates')

    return (
        set_plot_parameters(
            ax,
            title=title,
            ratio_plotting=True,
            reaction_rates=reaction_rates,
            energy_units=energy_units
        ),
        ratio_stats_df
    )

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

def adjust_dict_for_all_tag(d, key):
    """
    If a dictionary read from the xs_plotting input YAML file has specific
        values, return them, but if the 'all' parameter is provided, ignore
        other keys, as they will not be present

    Arguments:
        d (dict): Dictionary at some level of the hierarchical dictionary
            nuc_hierarchy derived from the YAML input file.
        key (str or int): Dictionary key at the provided level.

    Returns:
        val (dict, list, int, or str): Values corresponding to the selected
            dictionary key, be they they actual key read in or the 'all' key. 
    """

    return d[key] if key in d else d['all']

def find_all_mass_nums(tendl_dir, element):
    """
    Given the selection of 'all' mass numbers from an xs_plotting YAML input
        file, create a list of all mass numbers of nuclides of the provided 
        element contained in the reference TENDL data. 

    Arguments:
        tendl_dir (pathlib._local.PosixPath or str): Path to the directory in
            which the original TENDL data from which the cross-section data is
            extracted or derived.
        element (str): Chemical symbol of element for which to find mass
            numbers.

    Returns:
        mass_nums (set): List of all mass numbers of a given element contained
            in the repository of TENDL data. 
    """

    mass_nums = set()
    for path in tendl_dir.glob(f'{element}*.tendl'):
        nuc_match = re.compile(
            rf'{re.escape(element)}(\d+[mn]?)$'
        ).fullmatch(path.stem)
        if nuc_match:
            mass_nums.add(nuc_match.group(1))

    return mass_nums

def find_all_MTs(dfs, pKZA):
    """
    Given a list of preprocessed groupwise DSV files and a parent nuclide
        identified by its KZA, compile all reaction identifiers (MTs) that
        exist for that nuclide in any of the DSVs.

    Arguments:
        dfs (dict_values of pandas.core.frame.DataFrame): Iterable of all
            DataFrames containing ALARAJOY DSV data for the case(s) being
            plotted.
        pKZA (int): ZZZAAAM identifier of the parent nuclide.

    Returns:
        MTs (set): Set of all reaction types for the parent nuclide present in
            any of the DSV files provided.
    """

    return {
        MT for dsv_data in dfs
        for MT in dsv_data.loc[dsv_data['pKZA'] == pKZA, 'MT']
    }

def main():

    # Only load in yaml module when executing xs_plotting.py as a script,
    # to allow for it to be used within tendl_to_alarajoy.py without
    # additional necessary dependencies
    from yaml import safe_load

    plt.rcParams.update({'figure.max_open_warning': 0})
    plot_path = None

    parser = argparse.ArgumentParser()
    parser.add_argument('--yaml', '-y')
    parser.add_argument('--ratio_plotting', '-r', action='store_true')
    parser.add_argument('--reference_data', '-d', required=False)
    parser.add_argument('--flux_file', '-f')
    parser.add_argument('--energy_vline', '-e', nargs='*')
    args = parser.parse_args()

    with open(args.yaml, 'r') as f:
        parameter_dict = safe_load(f)

    #dsv_list = parameter_dict.get('DSV', ['cumulative_gendf_data.dsv'])
    dsv_dict = load_dsvs_to_dfs(
        parameter_dict.get('DSV', ['cumulative_gendf_data.dsv'])
    )
    tendl_dir = Path(parameter_dict.get('TENDL', 'tendl2017'))

    nuc_hierarchy = {
        k: v
        for k, v in parameter_dict.items()
        if k not in ['DSV', 'TENDL']
    }

    # Only search for elements which have files in the reference TENDL
    # directory
    tendl_elements = set(
        re.match(r'[A-Z][a-z]?', path.stem).group()
        for path in tendl_dir.glob('*.tendl')
    )
    elements = (
        tendl_elements if 'all' in [key.lower() for key in nuc_hierarchy]
        else set(nuc_hierarchy) & tendl_elements
    )

    ratio_stats_df = pd.DataFrame(
        columns=['Element', 'A', 'MT', 'Data Comparison', 'Mean', 'STD']
    )

    fluxes = []
    if args.flux_file:
        with open(args.flux_file, 'r') as f:
            for line in f:
                fluxes.extend(line.split())

        fluxes = np.asarray(fluxes, dtype=float)[::-1]

    reaction_rates = _is_rxn_rate(fluxes)
    energy_units = 'eV'
    for element in elements:
        element_dict = adjust_dict_for_all_tag(nuc_hierarchy, element)
        mass_nums = list(element_dict)

        if check_all_tag(mass_nums):
            mass_nums = find_all_mass_nums(tendl_dir, element)

        for A in mass_nums:
            KZA = int((
                njt.elements[element] * 1000 + flagged_num_to_int(A)[0]
            ) * 10 + tp.ISOMERIC_STATES.find(str(A)[-1]) + 1)

            MTs = adjust_dict_for_all_tag(element_dict, A)
            if isinstance(MTs, str):
                MTs = [MTs]

            if check_all_tag(MTs):
                MTs = find_all_MTs(dsv_dict.values(), KZA)

            for MT in MTs:
                fig, ax = plt.subplots(figsize=(10,6))
                
                continuous_dict = extract_continuous_data(
                    endf.Evaluation(tendl_dir / f'{element}{A}.tendl'), MT
                )

                groupwise_dict, emitted = (
                    extract_groupwise_data_from_DSV(dsv_dict, KZA, MT)
                )

                if groupwise_dict:
                    if args.energy_vline:
                        energy_units = plot_neutron_energy_axvline(
                            ax, args.energy_vline, parser=parser
                        )

                    ax = plot_single_nuc_rxn_data(
                        ax, element, A, emitted, continuous_dict,
                        groupwise_dict, energy_units=energy_units
                    )

                    plot_path = set_plot_save_path(
                        element, A, emitted, tendl_dir, groupwise_dict.keys()
                    )
                    fig.savefig(plot_path)
                    plt.close(fig)

                    if reaction_rates:
                        rate_fig, rate_ax = plt.subplots(figsize=(10,6))

                        if args.energy_vline:
                            energy_units = plot_neutron_energy_axvline(
                                rate_ax, args.energy_vline, parser=parser
                            )

                        rate_ax = plot_single_nuc_rxn_data(
                            rate_ax, element, A, emitted,
                            groupwise_dict=groupwise_dict, fluxes=fluxes,
                            energy_units=energy_units
                        )
                        rate_path = set_plot_save_path(
                            element, A, emitted, tendl_dir,
                            groupwise_dict.keys(),
                            reaction_rates=reaction_rates
                        )
                        rate_fig.savefig(rate_path)
                        plt.close(rate_fig)

                    if args.ratio_plotting:
                        reference_data = (
                            args.reference_data
                            if args.reference_data
                            else next(iter(groupwise_dict))
                        )

                        ratio_fig, ratio_ax = plt.subplots(figsize=(10,6))

                        if args.energy_vline:
                            energy_units = plot_neutron_energy_axvline(
                                ratio_ax, args.energy_vline, parser=parser
                            )

                        color_dict = collect_all_stair_colors(ax)
                        _, ratio_stats_df = plot_relative_energy_dependent_func(
                            ratio_ax, element, A, MT, emitted, groupwise_dict,
                            reference_data, color_dict, ratio_stats_df, ax.get_xlim(),
                            fluxes=fluxes, energy_units=energy_units
                        )
                        ratio_plot_path = set_plot_save_path(
                            element, A, emitted, tendl_dir,
                            groupwise_dict.keys(),
                            ratio_plotting=args.ratio_plotting,
                            reaction_rates=reaction_rates
                        )
                        ratio_fig.savefig(ratio_plot_path)
                        plt.close(ratio_fig)

    top_save_dir = plot_path.parents[2]
    print(
        f'Cross-section plots saved to {top_save_dir}/, ' \
        'organized by element, nuclide, reaction.'
    )

    if not ratio_stats_df.empty:
        csv_save_path = top_save_dir / 'ratio_statistics.csv'
        ratio_stats_df.to_csv(csv_save_path)
        print(
            f'Cross-section ratio statistics saved to {csv_save_path}.'
        )

if __name__ == '__main__':
    main()