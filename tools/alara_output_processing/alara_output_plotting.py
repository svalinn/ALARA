import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib import lines
from warnings import warn
import alara_output_processing as aop

# ------- Utility and Helper Functions -------

def preprocess_data(
    adf,
    run_lbl,
    variable,
    nuclides = None,
    time_unit='s',
    sort_by_time='',
    pre_irrad=False,
    head=None,
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
        pre_irrad (bool, optional): Option to include pre-irradiation values.
            (Defaults to False)
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
        'run_lbl'  : run_lbl,
        'variable' : adf.VARIABLE_ENUM[variable]
    }
    if not pre_irrad:
        filter_dict['time'] = ['>', -1]

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

    piv = filtered.pivot(index='nuclide', columns='time', values='value')

    if sort_by_time:
        sort_by_time = aop.extract_time_vals([sort_by_time])[0]
        piv = piv.sort_values(sort_by_time, ascending=False)

    if head:
        piv = piv.head(head)

    return filtered, piv

def build_color_map(cmap_name, all_nucs=[], pivs=None):
    '''
    Given a list of pivot DataFrames (one per run) or a 1D array-like data
        structure of nuclide string names, build a stable color mape keyed by
        nuclide name.
    
    Arguments:
        cmap_name (str): Matplotlib Colormap for the plots.
        all_nucs (array-like, optional): Collection of all nuclide
            names to be included in the color map.
            (Defaults to [])
        pivs (list of pandas.DataFrames or None, optional): List of pivot
            tables indexed by nuclide with values for each cooling time.
            (Defaults to None)
    '''
    
    cmap = plt.cm.get_cmap(cmap_name)
    
    # Check for "empty" all_nucs compatable with any 1D array-like objs
    all_nucs = set(all_nucs)
    if len(all_nucs) == 0:
        for piv in pivs:
            all_nucs.update(piv.index)

    if len(all_nucs) == 0:
        raise ValueError('Must input either all_nucs or pivs.')

    color_map = {
        lbl: cmap(i % cmap.N) for i, lbl in enumerate(sorted(all_nucs))
    }

    if 'Other' in set(all_nucs):
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

    if 'total' in isotope.lower() or isotope == 'Other':
        return isotope
    
    else:
        element, A = isotope.split('-')
        element = element.capitalize()
        return f'$^{{{A}}}${element}'

def construct_legend(ax, data_comp=False, legend_ax=None):
    '''
    Create a custom pyplot legend that exists outside of the grid itself and
        can group like-isotopes together from compared data sets for clarity.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        data_comp (bool, optional): Boolean setting for comparison between two
            data sets.
            (Defaults to False)
        legend_ax (matplotlib.axes._axes.Axes or None, optional): Optional
            argument to construct the legend on a separate Matplotlib Axes
            object than the plot itself.
    
    Returns:
        legend (matplotlib.legend.Legend): Matplotlib Legend object containing
            the constructed legend.
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

    target_ax = legend_ax if legend_ax else ax

    legend = target_ax.legend(
        grouped_handles,
        grouped_labels,
        loc='center' if legend_ax else 'center left',
        bbox_to_anchor=None if legend_ax else (1.025, 0.5),
        borderaxespad=0.,
        fontsize='small',
        handlelength=1.5,
        handletextpad=0.5,
    )

    return legend

def plot_or_scatter(plot_type, ax, x, y, label, color, style):
    '''
    Using a like-set of arguments, create either a standard plot or scatter
        plot for an ALARADFrame to be called within plot_single_response.

    Arguments:
        plot_type (str): Designation of what kind of plot to produce. Accepted
            values are either "plot" or "scatter".
        ax (matplotlib.axes._axes.Axes): Matplotlib axis object of the plot
            being constructed.
        x (array-like): x-axis data.
        y (array-like): y-axis data.
        label (str): Series label.
        style (str): Matplotlib lineStyle or lineMarker symbol unique to each
            run.
    
    Returns:
        None
    '''

    if plot_type == 'plot':
        ax.plot(x, y, label=label, color=color, linestyle=style)
    elif plot_type == 'scatter':
        ax.scatter(x, y, label=label, color=color, marker=style)
    else:
        raise ValueError(
            'Invalid plot type.' \
            'Must choose either "plot" or "scatter" for plot_type.'
        )

def pie_chart_aggregation(
        adf, run_lbl, variable, threshold, time_unit, pre_irrad=False
):
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
        pre_irrad (bool, optional): Option to include pre-irradiation values.
            (Defaults to False)

    Returns:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
    '''

    filtered, _ = preprocess_data(
        adf=adf,
        run_lbl=run_lbl,
        variable=variable,
        time_unit=time_unit,
        pre_irrad=pre_irrad
    )
    rel = filtered.calculate_relative_vals()
    agg = aop.aggregate_small_percentages(rel, threshold)

    return agg

def pie_grid_relative_tables(
        table_ax, agg, times, time_unit, color_map, totals, var_unit
):
    '''
    Given a Matplotlib Figure object, create a subplot Matplotlib table object
        to act as a color-coordinated legend for a grid of pie charts with
        the percentage contributions of each nuclide represented in any of the
        pie charts at each cooling time (as well as a row for aggregated
        "Other" nuclides that fall below a given proportional threshold).

    Arguments:
        table_ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the
            table being constructed.
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
        times (list of floats): Chronological list of cooling times.
        time_unit (str): Units for cooling times.
        color_map (dict): Pre-constructed color map for each nuclide to match
            table rows with pie chart wedges.
        totals (list): List of total values at each cooling time.
        var_unit (str): Unit of the variable being processed.

    Returns:
        None
    '''

    table_ax.axis('off')

    piv = agg.pivot(index='nuclide', columns='time', values='value') * 100
    piv = piv.loc[
        piv.mean(axis=1).sort_values(ascending=False).index
    ]
    totals_row = pd.Series(totals, index=piv.columns)
    total_name = f'Total [{var_unit}]'
    piv = pd.concat(
        [totals_row.to_frame(name=total_name).T, piv],
        axis=0
    )

    formatted_rows = []
    for idx, row in piv.iterrows():
        is_total = (idx == total_name)
        formatted_rows.append([
            (f'{x:.2e}' if is_total else f'{x:.1f}%') if pd.notna(x)
            else ('0.0' if is_total else '0.0%')
            for x in row
        ])
    piv = pd.DataFrame(formatted_rows, index=piv.index, columns=piv.columns)
    
    table = table_ax.table(
        cellText=piv.values,
        rowLabels=[reformat_isotope(nuc) for nuc in piv.index],
        colLabels=[
            f'{format_t(t)} {time_unit}'
            for t in piv.columns
        ],
        loc='center',
        cellLoc='center'
    )

    table.auto_set_font_size(False)
    table.set_fontsize(14)
    table.scale(1.0, 2.0)

    # Format table for readability
    for row in range(len(piv.index)):
        table[(row+1, -1)].get_text().set_fontweight('bold')
    for col in range(len(piv.columns)):
        for row in range(2):
            table[(row, col)].get_text().set_fontweight('bold')

    for i, nuc in enumerate(piv.index):
        if nuc == total_name:
            continue
        rgba = color_map[nuc]
        for j in range(len(times)+1):
            table[(i+1, j-1)].set_facecolor(rgba)

def format_t(t):
    '''
    Format a time value as either "Pre-Irradiation" or numerically in
        scientific notation to 2 decimal places.

    Arguments:
        t (float or str): Cooling time.

    Returns:
        t_trunc (str): Reformatted cooling time in truncated scientific
            notation.
    '''

    return t if (t == 'Pre-Irradiation' or t < 0) else f'{t:.2e}'

def pie_labels_by_threshold(time_slice, threshold):
    '''
    Filter pie chart labels to group small contributions with the "Other"
        label for readability.
    
    Arguments:
        time_slice (alara_output_processing.ALARADFrame): Filtered ALARADFrame
            containing data for a single cooling time.
        threshold (float): Proportional threshold for small-value aggregation.

    Returns:
        large_nucs (list of str): List of nuclides with a large enough
            contribution to be labeled on the pie chart, without overcrowding.
    '''

    ts = time_slice.copy()
    total = ts['value'].sum()
    if total > 0:
        ts['value'] /= total

    ts['nuclide'] = ts.apply(
        lambda row: reformat_isotope(row['nuclide'])
        if (
            row['value'] >= threshold
            or (row['nuclide'] == 'Other' and row['value'] > 0.0)
        ) else '', axis=1
    )

    return ts['nuclide'].tolist()

def add_pie(ax, time_slice, color_map, threshold):
    '''
    Create a pie chart for ALARA output data at a given cooling time to be
        called within single_time_pie_chart() or multi_time_pie_grid().

    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib axis object of the plot
            being constructed.
        time_slice (alara_output_processing.ALARADFrame): Filtered ALARADFrame
            containing data for a single cooling time.
        color_map (dict): Pre-constructed color map for each nuclide to match
            the wedges with the legend.
        threshold (float): Proportional threshold for small-value aggregation.

    Returns:
        wedges (list of matplotlib.patches.Wedge): List of Matplotlib Wedge
            objects corresponding to each wedge in the pie chart.
    '''
    
    if time_slice['value'].sum() == 0:
        warn(
            'All values for the selected variable at the selected time are ' \
            '0.\n Unable to produce a pie chart for the given time slice.'
        )
        return None

    wedges, _ = ax.pie(
        time_slice['value'],
        labels=pie_labels_by_threshold(time_slice, threshold),
        colors = [color_map[nuc] for nuc in time_slice['nuclide']],
        wedgeprops={'edgecolor' : 'black', 'linewidth' : 1},
        labeldistance=1.1,
        textprops={'fontsize':12}
    )

    return wedges

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
    ymin=None,
    relative=False,
    cmap_name='Dark2',
    plot_type='plot',
    separate_legend=False,
    control_run=''
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
        ymin (int, float, or None, optional): Option to set a bottom limit for
            the y-axis.
            (Defaults to None)
        relative (bool, optional): Option to plot relative values with respect
            to totals at each cooling time.
            (Defaults to False)
        cmap_name (str, optional): Option to set the Matplotlib Colormap for
            the plots. Reference guide for Matplotlib Colormaps can be found
            at matplotlib.org/stable/gallery/color/colormap_reference.html
            (Defaults to "Dark2")
        separate_legend (bool, optional): Option to return the legend as a
            separate Matplotlib Figure object. Can be useful for plots with
            many nuclide series.
            (Defaults to False)
        control_run (str, optional): Option to set a control run to against
            which to calculate time-series ratios for all other runs. If used,
            must case-sensitively match one of the labels in the list run_lbl.
            (Defaults to '')

    Returns:
        fig (matplotlib.figure.Figure): Closed Matplotlib Figure object
            containing the constructed plot.
        legend_fig (matplotlib.figure.Figure or None): Conditionally separated
            Matplotlib Figure object containing only the plot's legend. None
            if separate_legend argument is False.
    '''

    ratio_plotting = True if control_run else False
    data_comp = False
    fig, ax = plt.subplots(figsize=(10,6))

    if isinstance(run_lbls, list):
        data_comp=True
    else:
        run_lbls = [run_lbls]

    data_list = []
    styles = list(
        lines.lineStyles.keys() if plot_type == 'plot'
        else lines.lineMarkers.keys()
    )[:len(run_lbls)]

    for run_lbl, style in zip(run_lbls, styles):
        filtered, piv = preprocess_data(
            adf=adf,
            run_lbl=run_lbl,
            variable=variable,
            nuclides=nuclides,
            time_unit=time_unit,
            sort_by_time=sort_by_time,
            head=head,
        )

        if run_lbl == control_run and ratio_plotting:
            control_piv = piv
        else:
            data_list.append((run_lbl, filtered, piv, style))

    color_map = build_color_map(
        cmap_name=cmap_name,
        pivs=[data[2] for data in data_list]
    )
    for run_lbl, filtered, piv, style in data_list:
        for nuc in piv.index:
            if nuc == 'total' and not total:
                continue

            try:
                # Vectorized division to calculate time-series ratio against
                # the control run. If zeros exist in the control run, a zero-
                # division RuntimeWarning will be raised, but does not cause
                # plotting issues as NaNs will just not be plotted. If a ratio
                # series starts/stops abruptly, this zero-division is the
                # cause and not necessarily an error, as various nuclides may
                # be present across all cooling times.
                y = (
                    piv.loc[nuc].to_numpy() / control_piv.loc[nuc].to_numpy()
                    if ratio_plotting else piv.loc[nuc].tolist()
                )
            except KeyError:
                print(f'KeyError: Missing {nuc} from {run_lbl}')
                continue

            label_suffix = f' ({run_lbl})' if data_comp else ''
            plot_or_scatter(
                ax=ax,
                plot_type=plot_type,
                x=piv.columns,
                y=y,
                label=(nuc + label_suffix),
                color=color_map[nuc],
                style=style
            )

    title_suffix = (
        f'Ratio of {variable} against {control_run}' if ratio_plotting
        else f'{variable}'
    ) + ' vs Cooling Time'

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
        f'Ratio of {variable} against {control_run}' if ratio_plotting
        else (f'Proportion of Total {variable}' if relative
        else f'{variable} [{filtered['var_unit'].unique()[0]}]')
    )
    ax.set_xlabel(f'Time ({time_unit})')
    ax.set_xscale('log')
    ax.set_yscale(yscale)
    if ymin:
        ax.set_ylim(bottom=ymin)

    legend_ax = None
    if separate_legend:
        n_items = len(color_map)
        _, legend_ax = plt.subplots(figsize=(4, max(4, 0.3 * n_items)))
        legend_ax.axis('off')

    legend_fig = construct_legend(ax, data_comp, legend_ax)

    ax.grid(True)
    plt.tight_layout(rect=[0, 0, 0.85, 1])

    return fig, legend_fig

def single_time_pie_chart(
    agg,
    run_lbl,
    variable,
    threshold,
    time_idx,
    time_unit='s',
    cmap_name='Dark2',
    pre_irrad=False
):
    '''
    Create a pie chart depicting the breakdown of nuclides contributing to a
        given response at a given cooling time.

    Arguments:
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time,containing all
            aggregated data below the thresholds.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        threshold (float): Proportional threshold for small-value aggregation.
        time_idx (int): Cooling time interval number (i.e. 0 for shutdown).
        time_unit (str, optional): Optional paramter to set units for cooling
            times. Accepted values: 's', 'm', 'h', 'd', 'w', 'y', 'c'.
            (Defaults to 's')
        cmap_name (str, optional): Option to set the Matplotlib Colormap for
            the plots. Reference guide for Matplotlib Colormaps can be found
            at matplotlib.org/stable/gallery/color/colormap_reference.html
            (Defaults to "Dark2")
        pre_irrad (bool, optional): Option to include pre-irradiation values.
            (Defaults to False)
    
    Returns:
        fig (matplotlib.figure.Figure): Closed Matplotlib Figure object
            containing the constructed pie chart.
    '''

    times = sorted(agg['time'].unique())
    time_slice = agg[agg['time'] == times[time_idx]]
    color_map = build_color_map(
        cmap_name=cmap_name, all_nucs=time_slice['nuclide']
    )

    fig, ax = plt.subplots(figsize=(6, 6))
    wedges = add_pie(
        ax=ax,
        time_slice=time_slice,
        color_map=color_map,
        threshold=threshold
    )
    if not wedges:
        return None

    legend_items = []
    for wedge, nuc, val in zip(
        wedges, time_slice['nuclide'], time_slice['value']
    ):
        if val >= threshold or nuc == 'Other':
            legend_items.append((val, wedge, nuc))

    legend_items.sort(reverse=True, key=lambda x: x[0])

    legend_wedges = [item[1] for item in legend_items]
    legend_labels = [
        f'{reformat_isotope(item[2])}: {item[0] * 100:.1f}%'
        for item in legend_items
    ]
    ax.legend(
        legend_wedges,
        legend_labels,
        title='Nuclides',
        loc='center left',
        bbox_to_anchor=(-0.375, 0.75)
    )

    header_time = (
        ', Pre-Irradiation Values' if pre_irrad and time_idx == 0
        else f' at {times[time_idx]} {time_unit}'
    )
    ax.set_title(
        f'{run_lbl}:\nAggregated Proportional Contitributions to ' \
        f'{variable}{header_time}',
        loc='right'
    )

    return fig
    
def multi_time_pie_grid(
    agg,
    run_lbl,
    variable,
    var_unit,
    totals,
    threshold=0.05,
    time_unit='s',
    ncols=2,
    cmap_name='Dark2',
    individual_pie_dimension = 5.0, # equal width/height
    horizontal_buffer = 3.0,
    vertical_buffer = 0.7
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
        agg (alara_output_processing.ALARADFrame): Processed ALARADFrame
            (potentially) with new "Other" rows for each time, containing all
            aggregated data below the thresholds.
        run_lbl (str): Distinguisher of the specified ALARA run.
        variable (str): Name of the response variable.
        var_unit (str): Unit of the variable being processed.
        totals (list): List of total values at each cooling time.
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
        individual_pie_dimension (float, optional): Optional parameter to set
            the width/height of each pie chart in grid.
            (Defaults to 5.0)
        horizontal_buffer (float, optional): Optional parameter to set a
            buffer between each pie chart in a row.
            (Defaults to 3.0)
        vertical_buffer (float, optional): Optional parameter to set a buffer
            between each pie chart in a column (as well as the legend/table).
            (Defaults to 0.7)

    Returns:
        fig (matplotlib.figure.Figure): Closed Matplotlib Figure object
            containing the table and grid of pie charts.        
    '''

    color_map = build_color_map(
        cmap_name=cmap_name, all_nucs=agg['nuclide'].unique()
    )

    times = sorted(agg['time'].unique())
    if times[0] < 0:
        times[0] = 'Pre-Irradiation'

    N = len(times)
    nrows = int(np.ceil(N / ncols))
    fig = plt.figure(figsize=(
        individual_pie_dimension * ncols + horizontal_buffer,
        individual_pie_dimension * (nrows + vertical_buffer)
    ))

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
    table_ax = fig.add_subplot(gs[0,:])

    # Populate and format data table
    pie_grid_relative_tables(
        table_ax=table_ax,
        agg=agg,
        times=times,
        time_unit=time_unit,
        color_map=color_map,
        totals=totals,
        var_unit=var_unit
    )

    # Create pie charts in a chronologically ordered grid with the table above
    ax_grid = []
    for idx, t in enumerate(times):
        r = idx // ncols + 1
        c = idx % ncols
        ax = fig.add_subplot(gs[r, c])
        time_slice = agg[agg['time'] == t]

        wedges = add_pie(
            ax=ax,
            time_slice=time_slice, 
            color_map=color_map,
            threshold=threshold
        )
        title = f'Time = {format_t(t)}'
        title += '' if t == 'Pre-Irradiation' else f' {time_unit}'
        if not wedges:
            title += (
                '\n\n(No available pie chart for this time slice\n.' \
                'All nuclide contributions are 0 for the selected variable)'
            )
        ax.set_title(title, fontstyle='italic', fontsize=16)
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

    return fig