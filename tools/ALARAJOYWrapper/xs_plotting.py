import re
import argparse
import numpy as np
import tendl_processing as tp
import njoy_tools as njt
import reaction_data as rxd
import matplotlib.pyplot as plt
from pathlib import Path
from endf_parserpy import EndfParserPy

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
        gas_type = rxd.GAS_DF.loc[rxd.GAS_DF['kza']==int(dKZA), 'gas'].iat[0]
        emitted = emitted.upper() + gas_type

    return emitted

def vectorize_tab1(endf_dict={}, MT=0, pathway_data={}):
    """
    Interpret and reformat ENDF6 TAB1 data into a 1-D array for a specific
        reaction. This can be done for any ENDF MF (file), with different
        input requirements for MF3 versus MF9. TAB1 formatting is based on
        the ENDF-6 manual
        (https://www.nndc.bnl.gov/endfdocs/ENDF-102-2023.pdf). NJOY's
        endf.terpa() TAB1 interpolation function necessitates the array
        formatting of TAB1 data that vectorize_tab1() outputs, which is
        necessary to produce matching energies and energy-dependent values
        between MF3 and MF9 for a given reaction to produce excitation
        pathway-specific cross-sections from the multiplication of MF3
        cumulative cross-sections with MF9 multiplicites.

        MF3 ('Reaction Cross Sections'):
            The TAB1 formatting for MF3 is as follows:
                [QM, QI, 0, LR, NR, NP/ E_int/ sigma(E)]
            Descriptions of each of these values can be found in section 3.2
            ('Formats') of the ENDF6 manual.
            
            To vectorize this TAB1 data, an EndfParserPy-formatted nested
            dictionary containing a whole TENDL file's parsed nuclear data
            must be provided as the `endf_dict` argument and the specific
            reaction type as the `MT` argument.

        MF 9 ('Multiplicities for Production of Radioactive Nuclides'):

            The TAB1 formatting for MF9/10 is as follows:
                [QM, QI, IZAP, LFS, NR, NP/ E_int / Y(E)]
            Descriptions of each of these values can be found in section 9.2
            ('Formats') of the ENDF6 manual.

            To vectorize this TAB1 data, a subdictionary of an EndfParserPy-
            formatted nested dictionary must be supplied for a specific
            reaction, parent-daughter pathway. Because MF9 contains specific
            daughter excitation pathways for a given reaction, these
            'subsections' are contained in their own dictionaries in the
            EndfParserPy dictionary structure below the `MT` key. Such a
            subdictionary must be provided as the `pathway_data` argument.

    Arguments:
        endf_dict (dict, optional): Nested EndfParserPy-formatted dictionary
            containing all parsed nuclear data from a TENDL file. Only
            necessary for MF3.
            (Defaults to {})
        MT (int, optional): Unique reaction identifier. Only necessary for
            MF3.
            (Defaults to 0)
        pathway_data (dict, optional): Dictionary containing the TAB1 data for
            a specific MF9, MT reaction pathway to a specified daughter
            nuclide. Only necessary for MF9.
            (Defaults to {})

    Returns:
        tab1_array (numpy.ndarray): 1-D array of the reaction's TAB1 data from
            the implied MF handling scheme.
    """

    if (not endf_dict and MT == 0) and not pathway_data:
        raise TypeError(
            'Must supply either supply endf_dict and MT arguments together ' \
            'or pathway_data individually.'
        )

    # MF 9
    if pathway_data:
        nbt     = np.asarray(pathway_data['NBT'])
        ints    = np.asarray(pathway_data['INT'])
        ninterp = len(nbt)
        npoints = len(pathway_data['E'])
        header  = np.array([
            pathway_data['QM'],
            pathway_data['QI'],
            pathway_data['IZAP'],
            pathway_data['LFS'],
            ninterp,
            npoints
        ])
        energy_arr           = pathway_data['E']
        energy_dependent_var = pathway_data['Y']

    # MF 3
    else:
        section  = tp.get_section_dict(endf_dict, 3, MT)
        xs_table = section.get('xstable')
        
        nbt      = np.asarray(xs_table['NBT'])
        ints     = np.asarray(xs_table['INT'])
        ninterp  = len(nbt)
        npoints  = len(xs_table['E'])
        header   = np.array([
            section['QM'],
            section['QI'],
            0,
            section['LR'],
            ninterp,
            npoints
        ])
        energy_arr           = xs_table['E']
        energy_dependent_var = xs_table['xs']

    interpolation_scheme = np.empty(2 * ninterp)
    interpolation_scheme[::2]  = nbt
    interpolation_scheme[1::2] = ints

    tabular_data = np.empty(2 * npoints)
    tabular_data[::2]  = energy_arr
    tabular_data[1::2] = energy_dependent_var

    return np.concatenate([header, interpolation_scheme, tabular_data])

def extract_continuous_data(endf_dict, MT):
    """
    For a given nuclide and reaction, extract its continuous-energy cross-
        sections and corresponding energies from its original TENDL file.

    Arguments:
        endf_dict (dict): Nested EndfParserPy-formatted dictionary containing
            all parsed nuclear data from a TENDL file.
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

    continuous_dict = dict()
    MT, isomeric_state = flagged_num_to_int(MT)

    if isomeric_state > 0:
        for MF in tp.PATH_SPECIFIC_MFS:
            subsection = tp.get_section_dict(
                endf_dict, MF, MT
            ).get('subsection')
            if subsection and isomeric_state < len(subsection):
                pathway_data = subsection[list(subsection)[isomeric_state]]
                continuous_dict['energies'] = pathway_data['E']

                # Calculate interpolated proportional cross-section from MF3
                # cumulative cross-sections and MF9 multiplicities
                if MF == 9:
                    njoy_endf_wrapper = njt.import_njoy_endf_wrapper()

                    mf3_tab1 = vectorize_tab1(endf_dict=endf_dict, MT=MT)
                    mf3_interpolated_xs = njoy_endf_wrapper.interpolate_tab1(
                        mf3_tab1, pathway_data['E']
                    )

                    mf9_tab1 = vectorize_tab1(pathway_data=pathway_data)
                    mf9_interpolated_multiplicities = (
                        njoy_endf_wrapper.interpolate_tab1(
                            mf9_tab1, pathway_data['E']
                        )
                    )

                    continuous_dict['xs'] = (
                        mf3_interpolated_xs * mf9_interpolated_multiplicities
                    )
                    break
                
                # MF10 cross-sections can be extracted directly without need
                # for interpolation
                else:
                    continuous_dict['xs'] = pathway_data['sigma']

    # For non-excitation reactions, cross-sections can be extracted directly
    # from MF3
    else:
        xs_table = tp.get_section_dict(
            endf_dict, 3, MT
        ).get('xstable', {'E' : [], 'xs' : []})
        continuous_dict['xs'] = xs_table['xs']
        continuous_dict['energies'] = xs_table['E']

    return continuous_dict

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
        _, energy_bounds = njt.load_external_group_struct(group_name)

        for line in dsv_lines[1:-1]:
            rxn = line.split()
            dsv_pKZA, dsv_dKZA, dsv_MT, emitted = rxn[:4]
            emitted = ensure_emission_specificity(emitted, dsv_dKZA)

            if KZA == dsv_pKZA and str(MT) == dsv_MT:
                groupwise_dict[group_name] = {
                    'xs'          :    np.array(rxn[4:]).astype(float),
                    'energies'    :    energy_bounds
                }
                break

    return groupwise_dict, emitted

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
    ax, element, A, emitted, continuous_dict={}, groupwise_dict={}
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

    return ax

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

def find_all_MTs(dsv_list, pKZA):
    """
    Given a list of preprocessed groupwise DSV files and a parent nuclide
        identified by its KZA, compile all reaction identifiers (MTs) that
        exist for that nuclide in any of the DSVs.

    Arguments:
        dsv_list (list): List of filepaths to DSV files containing
            ALARAJOYWrapper-processed groupwise TENDL data.
        pKZA (int): ZZZAAAM identifier of the parent nuclide.

    Returns:
        MTs (set): Set of all reaction types for the parent nuclide present in
            any of the DSV files provided.
    """

    MTs = set()
    for dsv in dsv_list:
        with open(dsv, 'r') as f:
            dsv_lines = f.readlines()

        for line in dsv_lines[1:-1]:
            rxn = line.split()
            if rxn[0] == str(pKZA):
                MTs.add(rxn[2])
    
    return MTs

def main():

    # Only load in yaml module when executing xs_plotting.py as a script,
    # to allow for it to be used within preprocess_fendl3.py without
    # additional necessary dependencies
    from yaml import safe_load

    plt.rcParams.update({'figure.max_open_warning': 0})

    parser = argparse.ArgumentParser()
    parser.add_argument('--yaml', '-y')
    args = parser.parse_args()

    with open(args.yaml, 'r') as f:
        parameter_dict = safe_load(f)

    dsv_list = parameter_dict.get('DSV', ['cumulative_gendf_data.dsv'])
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

    for element in elements:
        element_dict = adjust_dict_for_all_tag(nuc_hierarchy, element)
        mass_nums = list(element_dict)

        if check_all_tag(mass_nums):
            mass_nums = find_all_mass_nums(tendl_dir, element)

        for A in mass_nums:
            endf_dict = EndfParserPy().parsefile(
                tendl_dir / f'{element}{A}.tendl'
            )

            KZA = str((
                njt.elements[element] * 1000 + flagged_num_to_int(A)[0]
            ) * 10 + tp.ISOMERIC_STATES.find(str(A)[-1]) + 1)

            MTs = adjust_dict_for_all_tag(element_dict, A)

            if isinstance(MTs, str):
                MTs = [MTs]
    
            if check_all_tag(MTs):
                MTs = find_all_MTs(dsv_list, KZA)

            for MT in MTs:
                fig, ax = plt.subplots(figsize=(10,6))
                
                continuous_dict = extract_continuous_data(endf_dict, MT)
                groupwise_dict, emitted = extract_groupwise_data_from_DSV(
                    dsv_list, KZA, MT
                )

                if groupwise_dict:
                    plot_single_nuc_rxn_xs(
                        ax, element, A, emitted,
                        continuous_dict, groupwise_dict
                    )
                    plot_path = set_plot_save_path(
                        element, A, emitted, tendl_dir, groupwise_dict.keys()
                    )
                    plt.savefig(plot_path)

    print(
        f'Cross-section plots saved to {plot_path.parents[2]}/, ' \
        'organized by element, nuclide, reaction.'
    )


if __name__ == '__main__':
    main()