import matplotlib.pyplot as plt
from matplotlib import lines
import alara_output_processing as aop
from numpy import array

def preprocess_data(
    adf,
    run_lbl,
    variable,
    nuclides = None,
    time_unit='s',
    sort_by_time='',
    head=None
):
    '''
    Prepare an ALARADFrame containing data from multiple runs and potentially
        multiple responses for plotting by filtering rows to a specified run,
        variable, and potentially nuclides. After filtering, a pivot table is
        created indexed by nuclide with values for each cooling time.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFrame containing 
            response data from one or more ALARA runs.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        nuclides (str, list, or None, optional): Optional parameter for
            nuclide selection. For a single nuclide, input should be a string
            of the form "element-A". For multiple individual nuclides, they
            should be input as a list. To filter all nuclides of a given
            element, provide the chemical symbol of the desired element,
            either as a stand-alone string or in a list with other elements or
            nuclides. To include the "total" row produced by ALARA, write
            "total". If no nuclides or elements provided, the filtering will
            pass through all nuclides matching the run_lbl and variable
            parameters.
            (Defaults to None)
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')
        sort_by_time (str, optional): Option to sort the ALARADFrame by the
            data in a particular time column.
            (Defaults to '')
        head (int or None, optional): Option by which to truncate the
            ALARADFrame to a particular number of rows.
            (Defaults to None)
        
    Returns:
        times (list of floats): Chronological list of converted cooling times.
        filtered (alara_output_processing.ALARADFrame): Modified copy of input
            adf containing only rows that match all conditions in run_lbl,
            variable, and (if present) nuclides.
        piv (pandas.DataFrame): Pivot table indexed by nuclides with values
            for each cooling time.
    '''

    filter_dict = {
        'run_lbl' : run_lbl, 'variable' : adf.VARIABLE_ENUM[variable]
    }
    if nuclides:
        filter_dict['nuclide'] = nuclides
    
    filtered = adf.filter_rows(filter_dict)

    preset_time_unit = filtered['time_unit'].unique()[0]
    if time_unit != preset_time_unit:
        filtered['time'] = filtered['time'].transform(
            lambda v: aop.convert_times(
                array([v]), from_unit=preset_time_unit, to_unit=time_unit
            )[0]
        )
        filtered['time_unit'] = [time_unit] * len(filtered)
        
    times = sorted(filtered['time'].unique().tolist())

    piv = filtered.pivot(
        index='nuclide',
        columns='time',
        values='value'
    )

    if sort_by_time:
        sort_by_time = aop.extract_time_vals([sort_by_time])[0]
        piv = piv.sort_values(sort_by_time, ascending=False)

    if head:
        piv = piv.head(head)

    return times, filtered, piv

def build_color_map(list_of_pivs, cmap_name='Dark2'):
    '''
    Given a list of pivot DataFrames (one per run), build a stable color map
        keyed by nuclide name.

    Arguments:
        list_of_pivs (list of pandas.DataFrames): List of pivot tables indexed
            by nuclide with values for each cooling time.
        cmap_name (str, optional): Option to set the Matplotlib Colormap for
            the plots. Reference guide for Matplotlib Colormaps can be found
            at matplotlib.org/stable/gallery/color/colormap_reference.html
            (Defaults to "Dark2")

    Returns:
        color_map (dict): Preset color map for each nuclide in the pivot table
            indices.
    '''

    all_nucs = set()
    for piv in list_of_pivs:
        all_nucs.update(piv.index)
  
    cmap = plt.cm.get_cmap(cmap_name)

    return {lbl: cmap(i % cmap.N) for i, lbl in enumerate(sorted(all_nucs))}

def split_label(label):
    '''
    Split the string of a series' label to extract the isotope being plotted
        and (conditionally) the run label attached to it. The run parameter
        will be present parenthetically if two runs are being plotted
        comparitavely.
    Arguments:
        label (str): Series label generated from 
            ax.get_legend_handles_labels().
    
    Returns:
        isotope (str): Isotope being plotted.
        run_lbl (str): Run label of plotted isotope. Will be empty string
            for single run plotting.
    '''

    # Label formatted as f"{element}-{A} ({run_lbl})" for comparative plots
    if '(' in label:
        parts = label.split('(')
        isotope = parts[0].strip()
        run_lbl = f'({parts[1].strip(')')})'
    else:
        isotope = label.strip()
        run_lbl = ''
    return isotope, run_lbl

def reformat_isotope(isotope):
    '''
    Restructure the string describing an isotope to capitalize its first
        letter and place the atomic number in a superscript for cleaner
        presentation in legend. Skips "isotope" entries of "total" or "Other".
    Arguments:
        isotope (str): Identifier of the isotope of the form element-A.
    
    Returns:
        isotope (str): Reformatted identifier of the isotope of the form
            ᴬelement.
    '''

    if isotope == 'total' or isotope == 'Other':
        return isotope
    else:
        element, A = isotope.split('-')
        element = element.capitalize()
        return f'$^{{{A}}}${element}'

def construct_legend(ax, data_comp=False):
    '''
    Create a custom pyplot legend that exists outside of the grid itself and
        can group like-isotopes together from compared data sets for clarity.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib axis object of the plot
            being constructed.
        data_comp (bool, optional): Boolean setting for comparison between two
            data sets.
            (Defaults to False)
    
    Returns:
        None
    '''

    handles, labels = ax.get_legend_handles_labels()
    labels_sorted_with_handles = sorted(
        zip(labels, handles),
        key=lambda x: (
            split_label(x[0]) if data_comp else (split_label(x[0])[0], x[0])
        )
    )

    grouped_handles = []
    grouped_labels = []
    prev_isotope = None

    for lbl, h in labels_sorted_with_handles:
        isotope, datalib = split_label(lbl)
        isotope = reformat_isotope(isotope)

        if prev_isotope is not None and isotope != prev_isotope:
            grouped_handles.append(plt.Line2D([], [], linestyle=''))
            grouped_labels.append('――――――')
        grouped_handles.append(h)
        grouped_labels.append(f'{isotope} {datalib}')
        prev_isotope = isotope

    ax.legend(
        grouped_handles,
        grouped_labels,
        loc='center left',
        bbox_to_anchor=(1.025, 0.5),
        borderaxespad=0.,
        fontsize='small',
        handlelength=1.5,
        handletextpad=0.5,
    )

def plot_single_response(
    adf,
    run_lbls,
    variable,
    nuclides = None,
    time_unit='s',
    sort_by_time='shutdown',
    head=None,
    total=False,
    yscale='log',
    relative=False
):
    '''
    Create a simple x-y plot of a given variable tracked in an ALARA output
        table (as stored in an ALARADFrame) against a log timescale. Options
        for plotting a single run, as well as two runs against each other.
        Plot will contain unique lines for the isotopes represented in the
        data, with options to show only certain elements and/or only the
        largest contributors at a given cooling time. Additionally, the
        cumulative total values across all isotopes can be plotted separately
        from individual isotopic data using the combination of the parameters:
        total=True, head=1.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFrame containing 
            response data from one or more ALARA runs.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        nuclides (str, list, or None, optional): Optional parameter for
            nuclide selection. For a single nuclide, input should be a string
            of the form "element-A". For multiple individual nuclides, they
            should be input as a list. To filter all nuclides of a given
            element, provide the chemical symbol of the desired element,
            either as a stand-alone string or in a list with other elements or
            nuclides. To include the "total" row produced by ALARA, write
            "total". If no nuclides or elements provided, the filtering will
            pass through all nuclides matching the run_lbl and variable
            parameters.
            (Defaults to None)
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')
        sort_by_time (str, optional): Option to sort the ALARADFrame by the
            data in a particular time column.
            (Defaults to 'shutdown')
        head (int or None, optional): Option by which to truncate the
            ALARADFrame to a particular number of rows.
            (Defaults to None)           
        total (bool, optional): Option to include the cumulative total
            contribution from all isotopes towards the select variable in the
            plot. If total=True, the total array will be treated equivalently
            to any of the other isotopes and will be plotted alongside them.
            If total=True and head=1, only the total will be plotted.
            (Defaults to False) 
        yscale (str, optional): Option to set the y-axis scale.
            (Defaults to 'log')
        relative (bool, optional): Option to plot relative values with respect
            to totals at each cooling time.
            (Defaults to False)

    Returns:
        None
    '''

    data_comp = False
    _, ax = plt.subplots(figsize=(10,6))

    if isinstance(run_lbls, list):
        data_comp=True
    else:
        run_lbls = [run_lbls]

    data_list = []
    line_styles = list(lines.lineStyles.keys())[:len(run_lbls)]
    for run_lbl, linestyle in zip(run_lbls, line_styles):
        times, filtered, piv = preprocess_data(
            adf=adf,
            run_lbl=run_lbl,
            variable=variable,
            nuclides=nuclides,
            time_unit=time_unit,
            sort_by_time=sort_by_time,
            head=head,
        )
        data_list.append((run_lbl, times, filtered, piv, linestyle))

    color_map = build_color_map([data[3] for data in data_list])
    for run_lbl, times, filtered, piv, linestyle in data_list:
        for nuc in piv.index:
            if nuc == 'total' and not total:
                continue

            label_suffix = f' ({run_lbl})' if data_comp else ''

            ax.plot(
                times,
                piv.loc[nuc].tolist(),
                label=(nuc + label_suffix),
                color=color_map[nuc],
                linestyle=linestyle
            )

    title_suffix = f'{variable} vs Cooling Time '

    if relative:
        title_suffix += 'Relative to Total at Each Cooling Time '
        yscale = 'linear'

    if head:
        title_suffix += (
            f'\n(ALARADFrame Head Sorted by Values at {sort_by_time})'
        )

    title_prefix = (
        f'{", ".join(run_lbls)} Comparison:\n' if data_comp
        else f'{run_lbls[0]}: '
    )

    ax.set_title(title_prefix + title_suffix)
    ax.set_ylabel(
        f'Proportion of Total {variable}' if relative
        else f'{variable} [{filtered['var_unit'].unique()[0]}]'
    )
    ax.set_xlabel(f'Time ({time_unit})')
    ax.set_xscale('log')
    ax.set_yscale(yscale)

    construct_legend(ax, data_comp)

    ax.grid(True)
    plt.tight_layout(rect=[0, 0, 0.85, 1])
    plt.show()

def single_time_pie_chart(
        adf,
        run_lbl,
        variable,
        threshold,
        time_idx,
        time_unit='s'
    ):
    '''
    Create a pie chart depicting the breakdown of nuclides contributing to a
        given response at a given cooling time.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFrame containing 
            response data from one or more ALARA runs.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        threshold (float): Proportional threshold for small-value aggregation.
        time_idx (int): Cooling time interval number (i.e. 0 for shutdown).
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')
    
    Returns:
        None
    '''

    times, filtered, _ = preprocess_data(
        adf=adf, run_lbl=run_lbl, variable=variable, time_unit=time_unit
    )
    rel = filtered.calculate_relative_vals()
    agg = aop.aggregate_small_percentages(rel, threshold)
    time_slice = agg[agg['time'] == times[time_idx]]
    labels = [reformat_isotope(nuc) for nuc in time_slice['nuclide']]

    wedges, _ = plt.pie(
        time_slice['value'],
        labels=labels,
        wedgeprops={'edgecolor' : 'black', 'linewidth' : 1}
    )

    legend_labels = [
        f'{nuc}: {frac * 100 :.1f}%'
        for nuc, frac in zip(labels, time_slice['value'])
    ]

    plt.title(
        f'{run_lbl}: Aggregated Proportional Contitributions ' \
        f'to {variable} at {times[time_idx]} {time_unit}'
    )

    plt.legend(
        wedges,
        legend_labels,
        title='Nuclides',
        loc='center left',
        bbox_to_anchor=(-0.375, 0.75)
    )
    
    plt.show()