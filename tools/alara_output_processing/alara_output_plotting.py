import math
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import lines
import alara_output_processing as aop

# ------- Utility and Helper Functions -------

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
                np.array([v]), from_unit=preset_time_unit, to_unit=time_unit
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

def build_color_map_from_pivs(list_of_pivs, cmap_name='Dark2'):
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

def build_color_map_from_nuc_list(all_nucs, cmap_name='Dark2'):
    '''
    Given a list of nuclide name strings, build a stable color map keyed by
        nuclide name.

    Arguments:
        all_nucs (list of str): List of all nuclide names to be included in
            the color map.
        cmap_name (str, optional): Option to set the Matplotlib Colormap for
            the plots. Reference guide for Matplotlib Colormaps can be found
            at matplotlib.org/stable/gallery/color/colormap_reference.html
            (Defaults to "Dark2")        
    '''

    cmap = plt.cm.get_cmap(cmap_name)
    N = len(all_nucs)
    colors = [cmap(i / max(N-1, 1)) for i in range(N)]
    color_map = {lbl: color for lbl, color in zip(all_nucs, colors)}

    # Set "Other" color as light gray in RGBA formatting
    color_map['Other'] = (0.8, 0.8, 0.8, 1.0)

    return color_map

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

def pie_chart_aggregation(adf, run_lbl, variable, threshold, time_unit):
    '''
    Prepare an aggregated ALARADFrame for single or multiple pie chart
        plotting with a user-defined proportional cutoff threshold.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFrame containing 
            response data from one or more ALARA runs.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        threshold (float, optional): Proportional threshold for small-value
            aggregation.
            (Defaults to 0.05)
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')

    Returns:
        times (list of floats): Chronological list of cooling times.
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
    '''

    times, filtered, _ = preprocess_data(
        adf=adf,
        run_lbl=run_lbl,
        variable=variable,
        time_unit=time_unit,
    )
    rel = filtered.calculate_relative_vals()
    agg = aop.aggregate_small_percentages(rel, threshold)

    return times, agg

def populate_table_rows(agg, all_nucs, times):
    '''
    Create a list of table rows for the legend-table in multi_time_pie_grid().
        Each row is populated with the percentage contribution of its
        respective nuclide (or aggregated "Other" pseudo-nuclide) at each
        cooling time.

    Arguments:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
        all_nucs (list of str): List of all nuclide names included in the
            table.
        times (list of floats): Chronological list of cooling times.

    Returns:
        table_data (list of lists): Nested list data structure containing a
            sub-list for each row in the table, corresponding to the total
            number of nuclides represented within the table (and in all of the
            pie charts in the grid). Each row-list contains a floating point
            number representing the percentage contribution of that nuclide at
            each cooling time. 
    '''

    table_data = []
    for nuc in all_nucs:
        row = []
        for t in times:
            ts = agg[(agg['nuclide'] == nuc) & (agg['time'] == t)]
            pct = float(ts['value'].iloc[0])*100 if not ts.empty else 0.0
            row.append(f'{pct:.1f}%')
        table_data.append(row)

    return table_data

def sort_by_index(lst, indices):
    '''
    Reorganize a list by a new list of indices.

    Arguments:
        lst (list): List to be reordered.
        indices (list of int): Indices for reordering.

    Returns:
        reordered_lst (lst): Modified copy of lst, reordered to the new
            indices. 
    '''

    return [lst[i] for i in indices]

def pie_grid_relative_tables(
    fig, agg, all_nucs, times, nrows, ncols, time_unit, color_map
):
    '''
    Given a Matplotlib Figure object, create a subplot Matplotlib table object
        to act as a color-coordinated legend for a grid of pie charts with
        the percentage contributions of each nuclide represented in any of the
        pie charts at each cooling time (as well as a row for aggregated
        "Other" nuclides taht fall below a given proportional threshold).

    Arguments:
        fig (matplotlib.figure.Figure): Matplotlib Figure object wherein the
            grid of pie charts will be constructed.
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
        all_nucs (list of str): List of all nuclide names included in any of
            the pie charts in the grid.
        times (list of floats): Chronological list of cooling times.
        nrows (int): Number of rows in the grid of pie charts.
        ncols (int): Number of columns in the grid of pie charts.
        time_unit (str): Units for cooling times.
        color_map (dict): Pre-constructed color map for each nuclide to match
            table rows with pie chart wedges.

    Returns:
        gs (matplotlib.gridspec.GridSpec): Matplotlib Gridspec object to
            organize the combined set of subplots containing the resultant
            table as well as the individual pie charts, to be added
            subsequent to this function's calling.
    '''

    # Create table gridspec
    left = 0.05
    top = 0.95
    gs = fig.add_gridspec(
        nrows + 1, ncols,
        height_ratios=([1.0] + [1]*nrows),
        hspace=0.4,
        wspace=0.3,
        left=left, right=(1-left), top=top, bottom=(1-top)
    )

    # Build and populate table
    table_ax = fig.add_subplot(gs[0,:])
    table_ax.axis('off')
    table_data = populate_table_rows(agg, all_nucs, times)

    col_labels = [f'{t} {time_unit}' for t in times]
    row_labels = [reformat_isotope(nuc) for nuc in all_nucs]

    table_data_float = np.array([
        [float(cell.strip('%')) for cell in row] for row in table_data
    ])
    row_means = table_data_float.mean(axis=1)
    sorted_indices = np.argsort(-row_means)
    table_data = sort_by_index(table_data, sorted_indices)
    row_labels = sort_by_index(row_labels, sorted_indices)

    table = table_ax.table(
        cellText=table_data,
        rowLabels=row_labels,
        colLabels=col_labels,
        loc='center',
        cellLoc='center'
    )
    table.auto_set_font_size(False)
    table.set_fontsize(11)
    table.scale(1.0, 2.0)

    # Format table for readability
    for i in range(len(row_labels)):
        table[(i+1, -1)].get_text().set_fontweight('bold')
    for j in range(len(col_labels)):
        table[(0, j)].get_text().set_fontweight('bold')

    for i, nuc in enumerate([all_nucs[idx] for idx in sorted_indices]):
        rgba = color_map[nuc]
        
        # Color in index
        table[(i+1, -1)].set_facecolor(rgba)
        
        # Color all other columns
        for j in range(len(times)):
            table[(i+1, j)].set_facecolor(rgba)

    return gs

def pie_labels_by_threshold(time_slice, threshold):
    '''
    Filter pie chart labels to group small contributions with the "Other"
        label for readability.
    
    Arguments:
        
    '''

    values = time_slice['value'].to_numpy()
    total = time_slice['value'].sum()
    fractions = values / total if total > 0 else np.zeros_like(values)

    labels = []
    if np.any(fractions == 1.0):
        for nuc, frac in zip(time_slice['nuclide'], fractions):
            labels.append(reformat_isotope(nuc) if frac == 1.0 else '')
        return labels
    
    for nuc, frac in zip(time_slice['nuclide'], fractions):
        if frac >= threshold or nuc == 'Other':
            labels.append(reformat_isotope(nuc))
        else:
            labels.append('')
    
    return labels


# ----- Plotting Functions ------

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

    color_map = build_color_map_from_pivs([data[3] for data in data_list])
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

    times, agg = pie_chart_aggregation(
        adf=adf,
        run_lbl=run_lbl,
        variable=variable,
        time_unit=time_unit,
        threshold=threshold
    )
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

def multi_time_pie_grid(
    adf,
    run_lbl,
    variable,
    threshold=0.05,
    time_unit='s',
    ncols=2,
    cmap_name='Dark2'
):
    '''
    Create a grid of pie charts for a single ALARA response variable from a
        single run across all cooling times contained in the ALARADFrame input
        by the user. Included in the figure produced is a color-coordinated 
        table of all nuclides represented in any of the pie charts at any
        time, as well as a grouping of "Other" nuclides that fall below the
        proportional aggregation threshold, which defaults to a 5%
        contribution for nuclides at any given time, but can be modified to
        user specifications.

    Arguments:
        adf (alara_output_processing.ALARADFrame): ALARADFrame containing 
            response data from one or more ALARA runs.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        threshold (float, optional): Proportional threshold for small-value
            aggregation.
            (Defaults to 0.05)
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')
        ncols (int, optional): Option to set the number of pie charts per row
            in the grid.
            (Defaults to 2)
        cmap_name (str, optional): Option to set the Matplotlib Colormap for
            the plots. Reference guide for Matplotlib Colormaps can be found
            at matplotlib.org/stable/gallery/color/colormap_reference.html
            (Defaults to "Dark2")

    Returns:
        None        
    '''

    times, agg = pie_chart_aggregation(
        adf=adf,
        run_lbl=run_lbl,
        variable=variable,
        time_unit=time_unit,
        threshold=threshold
    )
    all_nucs = sorted(set(agg['nuclide']))
    color_map = build_color_map_from_nuc_list(all_nucs, cmap_name=cmap_name)

    N = len(times)
    nrows = math.ceil(N / ncols)
    fig = plt.figure(figsize=(5 * ncols + 3, 5 * (nrows + 0.7)))
    
    gs = pie_grid_relative_tables(
        fig=fig,
        agg=agg,
        all_nucs=all_nucs,
        times=times,
        nrows=nrows,
        ncols=ncols,
        time_unit=time_unit,
        color_map=color_map
    )

    # Create pie charts in a chronologically ordered grid with the table above
    ax_grid = []
    for idx, t in enumerate(times):
        r = idx // ncols + 1
        c = idx % ncols
        ax = fig.add_subplot(gs[r, c])
        time_slice = agg[agg['time'] == t]

        ax.pie(
            time_slice['value'],
            labels=pie_labels_by_threshold(time_slice, threshold),
            colors=[color_map[nuc] for nuc in time_slice['nuclide']],
            wedgeprops={'edgecolor': 'black', 'linewidth': 1},
            labeldistance=1.1,
            textprops={'fontsize': 12} 
        )
        ax.set_title(
            f'Time = {t} {time_unit}', fontstyle='italic', fontsize=16
        )
        ax.axhline(
            y=1.25,
            xmin=0, xmax=1,
            color='gray',
            linewidth=0.8
        )
        ax.set_aspect('equal')
        ax.axis('off')

        ax_grid.append(ax)

    # Hide unused axes
    for idx in range(N, nrows * ncols):
        r = idx // ncols + 1
        c = idx % ncols
        fig.add_subplot(gs[r, c]).axis('off')

    # Draw grid between pie charts
    left, right = 0.05, 0.95
    bottom, top = 0.025, 0.975 - 1.0/(nrows + 1)
    for i in range(1, ncols):
        x = left + i * (right-left)/ncols
        fig.add_artist(plt.Line2D(
            [x, x], [bottom, top], color='black', linewidth=1
        ))
    for i in range(1, nrows):
        y = bottom + i * (top-bottom)/nrows
        fig.add_artist(plt.Line2D(
            [left, right], [y, y], color='black', linewidth=1
        ))

    suptitle = (
        f'{run_lbl}: Nuclide Contribution Breakdown for {variable}\n' \
         'Across all Cooling Times'
    )
    fig.suptitle(
        suptitle,
        fontsize=22,
        y=0.995,
        fontweight='bold'
    )

    plt.show()