import numpy as np
import pandas as pd
import re
import matplotlib.pyplot as plt
import re
from matplotlib import lines
import matplotlib.cm as cm
from warnings import warn
import alara_output_processing as aop
from collections import defaultdict

# ------ Comparative Series Statistics -------

class PlotStats:
    '''
    Class containing attributes and methods used for quantitatively assessing
        time-series plots in ALARAPlot. Can be applied for analysis simulated
        and computational data. Contains computational capabilities for
        statistical metrics organized as such (internal calling names in
        parentheses):

            1) Unweighted Single Series Metrics
                a) Mean (mean)
                b) Median (median)
                c) Standard Deviation (std)
                d) Variance (var)

            2) Weighted Single Series Metrics
                a) Uncertainty Weighted Mean (uncertainty-weighted-mean)

            3) Unweighted Compartive Metrics
                a) Root Mean Square Deviation (rmsd)
                b) Mean Percent Difference (mean-pct-diff)

            4) Weighted Comparative Metrics
                a) Uncertainty Weighted Root Mean Square Deviation
                   (uncertainty-weighted-rmsd)
                b) Uncertainty Weighted Mean Percent Difference
                   (uncertainty-weighted-mean-pct-diff)
    '''

    unweighted_single_series_metrics = {
        'mean': ('_compute_mean', r'\mu'),
        'median': ('_compute_median', 'median'),
        'std': ('_compute_std', r'\sigma'),
        'var': ('_compute_var', 'Var'),
    }
    weighted_single_series_metrics = {
        'uncertainty-weighted-mean': (
            '_compute_weighted_mean', r'Uncertainty\ Weighted\ Mean'
        ),
    }
    single_series_metrics = (
        unweighted_single_series_metrics | weighted_single_series_metrics
    )

    unweighted_comparative_metrics = {
        'rmsd': ('_compute_rmsd', 'RMSD'),
        'mean-percent-diff': ('_compute_mean_pct_diff', r'Mean\ \%\ Diff'),
    }
    weighted_comparative_metrics = {
        'uncertainty-weighted-rmsd': (
            '_compute_weighted_rmsd', r'Uncertainty\ Weighted\ RMSD'
        ),
        'uncertainty-weighted-mean-percent-diff': (
            '_compute_weighted_mean_pct_diff',
            r'Uncertainty\ Weighted\ Mean\ \%\ Diff'
        ),
    }
    comparative_metrics = (
        unweighted_comparative_metrics | weighted_comparative_metrics
    )

    all_metrics = single_series_metrics | comparative_metrics

    def __init__(
        self,
        series,
        stat_type='mean',
        skip_nans=True,
        secondary_series=np.array([]),
        uncertainties=np.array([])
    ):
        self.series = np.asarray(series)
        self.secondary_series = np.asarray(secondary_series)
        self.uncertainties = np.asarray(uncertainties)
        self.stat_type = stat_type.lower()

        self.skip_nans = skip_nans
        if self.skip_nans:
            self._mean = np.nanmean
            self._median = np.nanmedian
            self._std = np.nanstd
            self._var = np.nanvar
            self._average = self._nan_average
        else:
            self._mean = np.mean
            self._median = np.median
            self._std = np.std
            self._var = np.var
            self._average = np.average

        self.weights = None
        if self.uncertainties.size > 0:
            self.weights = self.uncertainties ** -2

        self.metrics = self.single_series_metrics
        if self.secondary_series.size > 0:
            self.metrics = self.all_metrics

        if self.stat_type not in self.metrics:
            raise ValueError(f'Unknown stat_type: {self.stat_type!r}')
        
        method_name, self.tex_name = self.metrics[self.stat_type]
        self._compute = getattr(self, method_name)
        self.stat = None

    def _require_weights(self, value):
        if self.weights is None:
            raise ValueError(
                f'stat_type={self.stat_type!r} requires uncertainties to ' \
                'be provided during PlotStats instantiation.'
            )

        return value

    # ------- Internal Formulae -------
    @staticmethod
    def _nan_average(s, weights):
        return np.nansum(s * weights) / np.nansum(weights)

    def _perc_diff(self):
        return (self.secondary_series - self.series) / self.series * 100

    def _square_err(self):
        return (self.series - self.secondary_series) ** 2

    # ------- Individual Statistics Computations -------
    def _compute_mean(self):
        return self._mean(self.series)

    def _compute_weighted_mean(self):
        return self._require_weights(
            self._average(self.series, weights=self.weights)
        )

    def _compute_median(self):
        return self._median(self.series)

    def _compute_std(self):
        return self._std(self.series)

    def _compute_var(self):
        return self._var(self.series)

    def _compute_rmsd(self):
        return np.sqrt(self._mean(self._square_err()))

    def _compute_weighted_rmsd(self):
        return self._require_weights(
            np.sqrt(self._average(self._square_err(), weights=self.weights))
        )

    def _compute_mean_pct_diff(self):
        return self._mean(self._perc_diff())

    def _compute_weighted_mean_pct_diff(self):
        return self._require_weights(
            self._average(self._perc_diff(), weights=self.weights)
        )

    # ------- Outward Facing Functionality -------
    @staticmethod
    def initialize_row(run, variable):
        return {'run' : run, 'variable' : variable}

    def calculate_statistic(self):
        self.stat = self._compute()
        return self.stat

    @classmethod
    def compute_all_metrics(
        cls,
        series,
        metrics=all_metrics,
        secondary_series=np.array([]),
        uncertainties=np.array([]),
        skip_nans=True
    ):
        '''
        Calculate and store all statistical metrics for a given series.

        Arguments:
            series (numpy.ndarray): Array containing data to be analyzed.
            metrics (list or dict, optional): Collection of statistical
                metrics to calculate. Can accept any combination of metrics
                contained in PlotStats.all_metrics.
                (Defaults to PlotStats.all_metrics)
            secondary_series (numpy.ndarray, optional): Array equivalent in
                size to `series`, used for comparative statistics
                calculations.
                (Defaults to numpy.array([]))
            uncertainties (numpy.ndarray, optional): Array equivalent in size
                to `series` containing experimental uncertainties. Can be used
                for comparative statistics; only necessary for weighted
                statistical metrics.
                (Defaults to numpy.arrray([]))
            skip_nans (bool, optional): Boolean to determine whether NaN
                values encountered in the provided series will be skipped
                (`True`) or raise a `ValueError` (`False`).
                (Defaults to True)

        Returns:
            computed (dict): Dictionary keyed by the names of each evaluated
                statistical metric, valued by the PlotStats object containing
                the associated computed value. 
        '''

        computed = {}
        for stat_type in metrics:
            stats_obj = cls(
                series=series,
                stat_type=stat_type,
                secondary_series=secondary_series,
                uncertainties=uncertainties,
                skip_nans=skip_nans
            )
            stats_obj.calculate_statistic()
            computed[stat_type] = stats_obj

        return computed


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
    half_lives=10
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
        decay_zeroing (bool, optional): Option to overwrite individual nuclide
            responses after the number of half-lives set in the half_lives
            parameter to zero to eliminate round-off error for long cooling
            times (relative to half-life length).
            (Defaults to True)
        half_lives (int, float, or None, optional): Option to specify the
            number of half-lives to pass in cooling time before zeroing
            subsequent decay responses. Used to eliminate round-off error for
            long cooling times (relative to half-life length). To avoid half-
            life zeroing, set half_lives to None.
            (Defaults to 10)
        
    Returns:
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

    if half_lives is not None:
        filtered = filtered.zero_long_decay_responses(half_lives=half_lives)

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

def compile_all_nucs(
    adf, 
    runs,
    variables,
    cmap_name='tab20',
    threshold=0.025,
    sort_by_time=None,
    time_unit='s'
):
    all_nucs = set()
    if not isinstance(variables, list):
        variables = [variables]

    for var in variables:
        aggs = []
        for run in runs:
            aggs.append(pie_chart_aggregation(
                adf=adf,
                run_lbl=run,
                variable=var,
                threshold=threshold,
                time_unit=time_unit,
                half_lives=None
            ))

        _, _, cm, _ = plot_single_response(
            adf=pd.concat(aggs),
            run_lbls=runs,
            variable=var,
            time_unit=time_unit,
            relative=True,
            cmap_name=cmap_name,
            half_lives=None,
            shading=True,
            sort_by_time=sort_by_time
        )
        all_nucs.update(set(cm.keys()))

    return set([n for n in all_nucs if n.lower() != 'other'])

def define_line_styles(run_lbls=[], plot_type='plot'):
    return list(
        lines.lineStyles.keys() if plot_type == 'plot'
        else lines.lineMarkers.keys()
    )[:len(run_lbls)]

def build_color_map(cmap_name, all_nucs=[], pivs=None, mark_thalf=False):
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
        mark_thalf (bool, optional): Option to create a corresponding shade
            for each nuclide's half-life to be plotted vertically alongside
            the time series.
            (Defaults to False)
    '''
    
    cmap = plt.cm.get_cmap(cmap_name)
    
    # Check for "empty" all_nucs compatable with any 1D array-like objs
    all_nucs = set(all_nucs)
    if len(all_nucs) == 0:
        for piv in pivs:
            all_nucs.update(piv.index)

    if len(all_nucs) == 0:
        raise ValueError('Must input either all_nucs or pivs.')

    if mark_thalf:
        color_map = {
            lbl: cmap(0.4 + 0.55 * i / max(len(all_nucs)-1,1))
            for i, lbl in enumerate(sorted(all_nucs))
        }
    else:
        color_map = {
            lbl: cmap(i % cmap.N) for i, lbl in enumerate(sorted(all_nucs))
        }

    if 'Other' in set(all_nucs):
        color_map['Other'] = (0.8, 0.8, 0.8, 1.0)

    if 'total' in set(all_nucs):
        color_map['total'] = "#574949"

    return color_map

def get_var_unit(filtered_adf):
    units = filtered_adf['var_unit'].unique()
    if len(units) > 1:
        raise ValueError(
            'ADF contains data in inconsistent units. Single variable data ' \
            'must all be represented by the same unit form.'
        )

    return units[0]

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
        if '\n' in run_lbl:
            run_lbl = run_lbl.strip(')')
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

    if re.match(r'\$\^{\d+[mn]?}\$[A-Z][a-z]?', isotope):
        return isotope

    time_bounds = ''
    if ':' in isotope:
        isotope, time_bounds = isotope.split(':', 1)
        time_bounds = ':' + time_bounds

    if 'total' in isotope.lower() or isotope == 'Other':
        return isotope

    else:
        element, A = isotope.split('-')
        element = element.capitalize()
        return f'$^{{{A}}}${element}{time_bounds}'

def append_stats_to_label(
    label,
    computed_stats,
    stat_types,
    sig_figs=3,
    lead_newline=True,
    trailing_separator='――――――'
):
    '''
    For plots that have had statistics calculated, include each of these
        values accordingly within the plot's legend.

    Arguments:
        label (str): Pre-existing label text upon which to append statistical
            summaries.
        computed_stats (dict): Dictionary keyed by the names of each evaluated
            statistical metric, valued by the PlotStats object containing
            the associated computed value. 
        stat_types (list of str): List containing the names of each
            statistical metric evaluated. Can be any combination of values
            contained in PlotStats.all_metrics.
        sig_figs (int, optional): Option to set a number of significant
            figures for the represenation of statistics in the legend.
            (Defaults to 3)
        lead_newline (bool, optional): Option to include a newline before the
            appending of the statistics bloc.
            (Defaults to True)
        trailing_separator (str, optional): Option to include a separator at
            the end of each run's statistics.
            (Defaults to "――――――")
    '''
    
    if lead_newline:
        label += '\n'

    for stat_type, stats_obj in computed_stats.items():
        if stat_type not in stat_types:
            continue

        formatted_statistic = f'{stats_obj.stat:.{sig_figs}g}'
        tex_percent = r'\%'
        if tex_percent in stats_obj.tex_name:
            formatted_statistic += tex_percent

        label += rf'${{{stats_obj.tex_name} = {formatted_statistic}}}$' + '\n'

    if trailing_separator is not None:
        label += trailing_separator

    return label

def construct_legend(ax, legend_ax=None):
    '''
    Create a custom pyplot legend that exists outside of the grid itself and
        can group like-isotopes together from compared data sets for clarity.
    
    Arguments:
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        legend_ax (matplotlib.axes._axes.Axes or None, optional): Optional
            argument to construct the legend on a separate Matplotlib Axes
            object than the plot itself.
    
    Returns:
        legend (matplotlib.figure.Figure): Matplotlib Figure object to which
            the constructed legend belongs.
    '''

    handles, labels = ax.get_legend_handles_labels()
    labels_sorted_with_handles = sorted(zip(labels, handles), key=lambda x: (
        (s := split_label(x[0]))[0].lower().startswith('total'), s[0], s[1]
    ))

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

    # Remove final line separator, which could have been applied above or in
    # total statistics calculation
    grouped_labels[-1] = grouped_labels[-1].replace('――――――', '')

    if legend_ax:
        target_ax = legend_ax
        loc = 'center'
        bbox_to_anchor = None
    else:
        target_ax = ax
        loc = 'center left'
        bbox_to_anchor = (1.025, 0.5)

    legend = target_ax.legend(
        grouped_handles,
        grouped_labels,
        loc=loc,
        bbox_to_anchor=bbox_to_anchor,
        borderaxespad=0.,
        fontsize='small',
        handlelength=1.5,
        handletextpad=0.5,
    )

    return legend.get_figure()

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
        adf,
        run_lbl,
        variable,
        threshold,
        time_unit,
        pre_irrad=False,
        half_lives=10
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
        half_lives (int, float, or None, optional): Option to specify the
            number of half-lives to pass in cooling time before zeroing
            subsequent decay responses. Used to eliminate round-off error for
            long cooling times (relative to half-life length). To avoid half-
            life zeroing, set half_lives to None.
            (Defaults to 10)

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
        pre_irrad=pre_irrad,
        half_lives=half_lives
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

def shade_dominant_nuclides(piv, ax, color_map, cmap_name, n_runs):
    '''
    Identify the time ranges for a single response variable in which any
        individual nuclide is the response's dominant contributor. Over these
        ranges, shade the region with a corresponding color for that nuclide,
        either with a pre-existing color map or by producing a new one if not
        provided. Shading is done as a function of the proportion of the total
        decay response contributed by the dominant nuclide at each time, with
        proportions closer to 1 being shaded darker and vice versa.

    Arguments:
        piv (pandas.DataFrame): Pivot table indexed by nuclides with values
            for each cooling time.
        ax (matplotlib.axes._axes.Axes): Matplotlib Axes object of the plot
            being constructed.
        color_map (dict): Pre-constructed color map for each nuclide to match
            the wedges with the legend. Empty dictionary if no color map
            existing yet for the plot.
        cmap_name (str): Matplotlib Colormap for the plot.
        n_runs (int): Number of runs being comparatively plotted.

    Returns:
        ax (matplotlib.axes._axes.Axes): Updated Axes object with shaded
            regions for dominant nuclides.
        bounds (numpy.ndarray): Logarithmic half-way values between cooling
            times to bound axvspan shading regions.
        dominant_nucs (list of str): List of the dominant nuclide in each
            logarithmically bounded region.
    '''

    piv = piv[piv.index != 'total']
    dominant_nucs = [piv[t].idxmax() for t in piv.columns]
    relative_max = [piv[t].max() / piv[t].sum() for t in piv.columns]
    times = np.asarray(piv.columns, dtype=float)

    if not color_map:
        color_map = build_color_map(
            cmap_name=cmap_name, all_nucs=set(dominant_nucs)
        )

    # Calculate logarithmic half-way values for each cooling time for shading
    # to flow smoothly between nuclide regions
    bounds = np.empty(len(times) + 1)
    bounds[1:-1] = np.sqrt(times[1:] * times[:-1])
    bounds[0] = times[0] * times[0] / bounds[1]
    bounds[-1] = times[-1] * times[-1] / bounds[-2]
    bounds = np.nan_to_num(bounds, nan=0.0)

    for lower, upper, nuc, dominance, in zip(
        bounds[:-1], bounds[1:], dominant_nucs, relative_max
    ):
        ax.axvspan(
            lower,
            upper,
            color=color_map[nuc],
            # Shading transparency as an inverse function of the relative
            # contribution of the dominant nuclide and scaled by the number
            # of runs being compared
            alpha=(0.2 * dominance) / (n_runs * 0.5),
            linewidth=0,
            label=None
        )

    return ax, bounds, dominant_nucs

def deliniate_shading_regions_by_run(
    all_dominance_ranges, run_lbl, piv, ax, color_map, cmap_name, n_runs 
):
    ax, bounds, dominant_nucs = shade_dominant_nuclides(
        piv, ax, color_map, cmap_name=cmap_name, n_runs=n_runs
    )

    prev_nuc = dominant_nucs[0]
    lower = bounds[0]
    for upper, nuc in zip(bounds[1:-1], dominant_nucs[1:]):
        if nuc != prev_nuc:
            all_dominance_ranges[prev_nuc].append((
                run_lbl, lower, upper 
            ))
            lower = upper
            prev_nuc = nuc

    upper = bounds[-1]
    all_dominance_ranges[prev_nuc].append((run_lbl, lower, upper))

    return all_dominance_ranges, ax

def add_shading_legend_labels(
    ax, color_map, all_dominance_ranges, time_unit, show_shading_bounds
):
    '''
    Unpack and format dominant nuclide data produced from 
        shade_dominant_nuclides() to be included in a plot's legend in the
        format:

            nuclide:
                run_lbl: beginning_of_dominance_range - end_of_dominance_range

    Arguments:
        ax (matplotlib.axes._axes.Axes): Updated Axes object with shaded
            regions for dominant nuclides.
        color_map (dict): Copy of the original color map dictionary if a non-
            empty dictionary is provided in the Arguments. Otherwise, a color
            map with keys only of the various dominant nuclides.
        all_dominance_ranges (dict): Dictionary keyed by nuclides that are the
            dominant contributor to a decay response over some time time range
            formatted as:

                {
                    nuclide: 
                        (
                            run_lbl,
                            beginning_of_dominance_range,
                            end_of_dominance_range
                        )
                }

    Returns:
        None 
    '''

    for nuc, run_entries in all_dominance_ranges.items():
        run_lines = '\n'.join(
            f'   -  {rl}: {tmin:.2g} - {tmax:.2g} {time_unit}'
            for rl, tmin, tmax in run_entries
        )
        label = reformat_isotope(nuc)
        if show_shading_bounds:
            label += f':\n{run_lines}' 

        # Zero-width spans for legend purposes only -- not visible on plot fig
        ax.axvspan(
            0,
            0,
            color=color_map[nuc],
            alpha=0.6,
            linewidth=0,
            label=label
        )

# ----- Plotting Functions ------

def plot_single_response(
    adf,
    run_lbls,
    variable,
    nuclides = None,
    time_unit='s',
    sort_by_time='shutdown',
    head=None,
    half_lives=10,
    total=False,
    yscale='log',
    y_limits=(None, None),
    relative=False,
    cmap_name='Dark2',
    plot_type='plot',
    separate_legend=False,
    control_run=None,
    sig_figs=3,
    stat_types=['mean'],
    skip_nans=True,
    mark_thalf=False,
    shading=False,
    shading_color_map={},
    show_shading_bounds=True,
    figsize=(10,6)
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
        run_lbls (str or list of str): Distinguisher of the specified run or
            list of distinguishers for multiple runs.
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
        half_lives (int, float, or None, optional): Option to specify the
            number of half-lives to pass in cooling time before zeroing
            subsequent decay responses. Used to eliminate round-off error for
            long cooling times (relative to half-life length). To avoid half-
            life zeroing, set half_lives to None.
            (Defaults to 10)
        total (bool, optional): Option to include the cumulative total
            contribution from all isotopes towards the select variable in the
            plot. If total=True, the total array will be treated equivalently
            to any of the other isotopes and will be plotted alongside them.
            If total=True and head=1, only the total will be plotted.
            (Defaults to False) 
        yscale (str, optional): Option to set the y-axis scale.
            (Defaults to 'log')
        y_limits (tuple, optional): Option to set the lower and/or upper
            plotting bounds for the y-axis. Formatted as (bottom, top). Each
            can be applied individually or together, but if only one setting
            is chosen the other must be listed explicitly as None (i.e. 
            setting only a lower limit of 1e4 would be (1e4, None)).
            (Defaults to (None, None))
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
        control_run (str or None, optional): Option to set a control run to
            against which to calculate time-series ratios for all other runs.
            If used, must case-sensitively match one of the labels in the list
            run_lbl.
            (Defaults to '')
        sig_figs (int, optional): Option to set a number of significant
            figures for the represenation of statistics calculated for time-
            series ratios (in conjunction with control_run).
            (Defaults to 3)
        stat_types (list of str, optional): List containing the names of each
            statistical metric evaluated. Can be any combination of values
            contained in PlotStats.all_metrics.
            (Defaults to ['mean'])
        skip_nans (bool, optional): Boolean to determine whether NaN values
            encountered in the provided series will be skipped (`True`) or
            raise a `ValueError` (`False`) when calculating plot statistics.
            (Defaults to True)
        mark_thalf (bool, optional): Option to mark a vertical line for the
            half-lives of all nuclides present in the plot.
            (Defaults to False)
        shading (bool, optional): Option to shade the regions in which any
            particular nuclide is the dominant contributor to the plotted
            decay response.
            (Defaults to False)
        shading_color_map (dict, optional): Option to import a pre-existing
            nuclide color map to shade dominant regions consistently with
            other plots.
            (Defaults to {})
        show_shading_bounds (bool, optional): Option to show the time range in
            which a given nuclide is the dominant nuclide for a given
            response in the plot's legend, corresponding to the region(s)
            shaded in the plot.
            (Defaults to True) 
        figsize (tuple of float, optional): Option to set the Matplotlib
            Figure size, by (width, height). Identical to
            `matplotlib.pyplot.Figure.figsize` parameter.
            (Defaults to (10,6))

    Returns:
        fig (matplotlib.figure.Figure): Closed Matplotlib Figure object
            containing the constructed plot.
        legend_fig (matplotlib.figure.Figure or None): Conditionally separated
            Matplotlib Figure object containing only the plot's legend. None
            if separate_legend argument is False.
        shading_color_map (dict): Color map for dominant nuclide shaded
            regions. Only populated if shading=True.
    '''

    ratio_plotting = (control_run is not None)
    data_comp = False
    fig, ax = plt.subplots(figsize=figsize)

    if isinstance(run_lbls, list):
        data_comp = True
    else:
        run_lbls = [run_lbls]
        data_comp = False

    data_list = []
    shade_pivs = {}
    styles = define_line_styles(run_lbls, plot_type=plot_type)

    filtered_concat = pd.DataFrame()
    for run_lbl, style in zip(run_lbls, styles):
        filtered, piv = preprocess_data(
            adf=adf,
            run_lbl=run_lbl,
            variable=variable,
            nuclides=nuclides,
            time_unit=time_unit,
            sort_by_time=sort_by_time,
            head=head,
            half_lives=half_lives
        )
        filtered_concat = pd.concat([filtered_concat, filtered])

        if run_lbl == control_run:
            control_piv = piv
        else:
            data_list.append((run_lbl, filtered, piv, style))
            if shading and total:
                _, shade_piv = preprocess_data(
                    adf=adf,
                    run_lbl=run_lbl,
                    variable=variable,
                    time_unit=time_unit,
                    sort_by_time=sort_by_time,
                    head=head,
                    half_lives=half_lives
                )
                shade_pivs[run_lbl] = shade_piv
            
            else:
                shade_pivs[run_lbl] = piv

    pivs = [data[2] for data in data_list]
    color_map = build_color_map(cmap_name=cmap_name, pivs=pivs)
    thalf_cmap = build_color_map(
        cmap_name='Reds', pivs=pivs, mark_thalf=mark_thalf
    )


    if shading and not shading_color_map:
        shading_color_map = build_color_map(
            cmap_name=cmap_name, pivs=list(shade_pivs.values())
        )

    plotted_nucs = []
    tmax = 0
    all_dominance_ranges = defaultdict(list)
    stats_rows = []

    for run_lbl, filtered, piv, style in data_list:
        if shading:
            all_dominance_ranges, ax = deliniate_shading_regions_by_run(
                all_dominance_ranges,
                run_lbl,
                shade_pivs[run_lbl],
                ax,
                shading_color_map,
                cmap_name=cmap_name,
                n_runs=len(data_list)
            )
            # ax, bounds, dominant_nucs = shade_dominant_nuclides(
            #     shade_pivs[run_lbl], ax, shading_color_map,
            #     cmap_name=cmap_name, n_runs=len(data_list)
            # )

            # prev_nuc = dominant_nucs[0]
            # lower = bounds[0]
            # for upper, nuc in zip(bounds[1:-1], dominant_nucs[1:]):
            #     if nuc != prev_nuc:
            #         all_dominance_ranges[prev_nuc].append((
            #             run_lbl, lower, upper
            #         ))
            #         lower = upper
            #         prev_nuc = nuc

            # upper = bounds[-1]
            # all_dominance_ranges[prev_nuc].append((run_lbl, lower, upper))

        for nuc in piv.index:
            if nuc == 'total' and not total:
                continue

            if nuc not in filtered['nuclide'].unique():
                warn(f'Missing {nuc} from {run_lbl}')
                continue

            label_suffix = f' ({run_lbl})' if data_comp else ''

            # Vectorized division to calculate time-series ratio against the
            # control run. If zeros exist in the control run, a zero-division
            # RuntimeWarning will be raised, but does not cause plotting
            # issues as NaNs will just not be plotted. If a ratio series
            # starts/stops abruptly, this zero-division is the cause and is
            # not necessarily an error, as various nuclides may not be present
            # across all cooling times.

            series = piv.loc[nuc].to_numpy()
            if ratio_plotting:
                if nuc not in control_piv.index:
                    warn(
                        f'{nuc.capitalize()} present in {run_lbl}' \
                        f'but not in {control_run}. Skipping.'
                    )
                    continue

                series /= control_piv.loc[nuc].to_numpy()
                if len(stat_types) > 0 and nuc == 'total':
                    computed = PlotStats.compute_all_metrics(
                        series=series,
                        metrics=PlotStats.unweighted_single_series_metrics,
                        skip_nans=skip_nans
                    )
                    row = PlotStats.initialize_row(run_lbl, variable)
                    row.update({st: obj.stat for st, obj in computed.items()})

                    label_suffix = append_stats_to_label(
                        label_suffix, computed, stat_types, sig_figs=sig_figs
                    )
                    stats_rows.append(row)

            t = piv.columns
            non_zeroes = np.flatnonzero(series)
            if non_zeroes.size > 0:
                last_nonzero_time = t[non_zeroes[-1]]
                if last_nonzero_time > tmax:
                    tmax = last_nonzero_time

            plot_or_scatter(
                ax=ax,
                plot_type=plot_type,
                x=t,
                y=series,
                label=(nuc + label_suffix),
                color=color_map[nuc],
                style=style
            )

            if mark_thalf and nuc not in plotted_nucs:
                thalf = filtered.get_thalf(nuc)
                plt.axvline(
                    x=thalf, color=thalf_cmap[nuc], alpha=0.85, label=(
                    rf'{nuc} ($t_{{1/2}} = {thalf:.2e}{time_unit}$)'
                ))
                plotted_nucs.append(nuc)

    if shading and shading_color_map is not None:
        add_shading_legend_labels(
            ax, shading_color_map, all_dominance_ranges,
            time_unit, show_shading_bounds
        )

    ylabel = f'{variable} [{get_var_unit(filtered_concat)}]'
    title_suffix = (
        f'Ratio of {variable} against {control_run}' if ratio_plotting
        else f'{variable}'
    ) + ' vs Cooling Time'

    if ratio_plotting:
        ylabel = f'Ratio of {variable} against {control_run}'

    if relative:
        title_suffix += ' Relative to Total at Each Cooling Time '
        yscale = 'linear'
        ylabel = f'Proportion of Total {variable}'

    if head:
        title_suffix += (
            f'\n(ALARADFrame Head Sorted by Values at {sort_by_time})'
        )

    title_prefix = (
        f'{", ".join(run_lbls)} Comparison:\n' if data_comp
        else f'{run_lbls[0]}: '
    )

    ax.set_title(title_prefix + title_suffix)
    ax.set_ylabel(ylabel)
    ax.set_xlabel(f'Time [{time_unit}]')
    ax.set_xscale('log')
    ax.set_xlim(right=tmax)
    ax.set_yscale(yscale)
    ax.set_ylim(bottom=y_limits[0], top=y_limits[1])

    legend_ax = None
    if separate_legend:
        n_items = len(color_map)
        _, legend_ax = plt.subplots(figsize=(4, max(4, 0.3 * n_items)))
        legend_ax.axis('off')

    legend_fig = construct_legend(ax, legend_ax)

    ax.grid(True)
    plt.tight_layout(rect=[0, 0, 0.85, 1])

    return fig, legend_fig, shading_color_map, pd.DataFrame(stats_rows)

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

def plot_computational_with_experimental(
    adf,
    variable,
    experimental_data,
    start_zeros,
    experiment_name='Experiment',
    irradiation_description='',
    uncertainties=[],
    cooling_times=[],
    comparison_type='raw',
    runs=[],
    sort_by_time=None,
    time_unit='s',
    half_lives=None,
    threshold=0.025,
    cmap_name='Dark2',
    stat_types=['rmsd'],
    shading=True,
    show_shading_bounds=False,
):

    fig, ax = plt.subplots(figsize=(12,6))
    yscale='log'
    if not irradiation_description:
        irradiation_description = experiment_name

    runs = runs if len(runs) > 0 else list(adf['run_lbl'].unique())
    all_nucs = compile_all_nucs(
        adf,
        runs,
        variable,
        cmap_name=cmap_name,
        threshold=threshold,
        sort_by_time=sort_by_time,
        time_unit=time_unit
    )

    shading_color_map = build_color_map(cmap_name, all_nucs)
    styles = define_line_styles(run_lbls=runs)

    computational_max = 0
    computational_min = np.inf
    all_dominance_ranges = defaultdict(list)
    filtered_concat = pd.DataFrame()
    stats_rows = []
    for i, run in enumerate(runs):
        filtered, piv = preprocess_data(
            adf,
            run,
            variable,
            nuclides=['total'],
            time_unit=time_unit,
            sort_by_time=sort_by_time,
            half_lives=half_lives
        )
        filtered_concat = pd.concat([filtered_concat, filtered])

        computational_data = filtered['value'][:start_zeros]
        if len(computational_data) != len(experimental_data):
            warn(
                f'{run}: Length mismatch, ' \
                f'C={len(computational_data)}, E={len(experimental_data)}. ' \
                'Skipping.'
            )
            continue

        computational_min = min(computational_min, min(computational_data))
        computational_max = max(computational_max, max(computational_data))

        y = computational_data
        c_over_e = computational_data / experimental_data
        run_split = run.split(',')[0].upper()
        label = rf'$\mathbf{{{run_split.replace(' ', r'\ ')}}}$'

        if comparison_type != 'raw':
            y = c_over_e
            if isinstance(stat_types, str):
                stat_types = [stat_types]

            # Calculate full range of PlotStats statistics
            row = PlotStats.initialize_row(run_split, variable)
            computed = {}
            for stat_type in PlotStats.all_metrics:
                stats_series = (
                    y if 'rmsd' not in stat_type
                    and 'percent-diff' not in stat_type
                    else experimental_data 
                )
                stats_obj = PlotStats(
                    series=stats_series,
                    stat_type=stat_type,
                    secondary_series=computational_data,
                    uncertainties=uncertainties
                )
                stats_obj.calculate_statistic()
                computed[stat_type] = stats_obj
                row[stat_type] = stats_obj.stat

            label = append_stats_to_label(
                label, computed, stat_types,
                sig_figs=3, lead_newline=True, trailing_separator=None
            )
            stats_rows.append(row)

            #     statistic = stats_obj.calculate_statistic()
            #     row[stat_type] = statistic

            #     if stat_type not in stat_types:
            #         continue

            #     # Include only selected statistic(s) to plot legend
            #     formatted_statistic = f'{statistic:.3g}'
            #     percent = r'\%'
            #     if percent in stats_obj.tex_name:
            #         formatted_statistic += percent

            #     label += '\n' + rf'${{{stats_obj.tex_name} = {formatted_statistic}}}$'

            stats_rows.append(row)

        if len(cooling_times) == 0:
            cooling_times = piv.columns

        ax.plot(
            cooling_times,
            y,
            linestyle=styles[i],
            label=label,
            color='dimgray'
        )

        if shading:
            _, shade_piv = preprocess_data(
                adf,
                run,
                variable,
                time_unit=time_unit,
                sort_by_time=sort_by_time,
                half_lives=half_lives
            )
            all_dominance_ranges, ax = deliniate_shading_regions_by_run(
                all_dominance_ranges, run, shade_piv, ax, shading_color_map,
                cmap_name=cmap_name, n_runs=len(runs)
            )

    title = (
        f'{variable} vs. Cooling Time\n'\
        f'for {irradiation_description} Irradiation'
    )

    if comparison_type == 'raw':
        ms = 3
        exp_min = np.min(experimental_data)
        exp_upper = 0
        if len(uncertainties) == len(cooling_times):
            ax.errorbar(
                cooling_times,
                experimental_data,
                yerr=uncertainties,
                fmt='*',
                ms=ms,
                color='k',
                label=experiment_name,
                ecolor='k',
                elinewidth=1,
                capsize=3
            )
            exp_lower = experimental_data - uncertainties
            positive_exp_lower = exp_lower[exp_lower > 0]
            exp_min = (
                np.min(positive_exp_lower)
                if positive_exp_lower.size
                else np.min(experimental_data)
            )
            exp_upper = experimental_data + uncertainties

        else:
            ax.scatter(
                cooling_times,
                experimental_data,
                marker='*',
                s=ms**2,
                color='k',
                label=experiment_name
            )

        total_min = min(computational_min, exp_min)
        total_max = max(computational_max, np.max(exp_upper))

        if total_max / total_min < 10:
            yscale = 'linear'
            ax.ticklabel_format(style='sci', scilimits=(0,0), axis='y')

        ylabel = f'{variable} [{get_var_unit(filtered_concat)}]'

    # Experimental over computatational plot
    else:
        ylabel = f'C/E: {variable}'
        yscale = 'linear'
        title = 'Computational/Experimental Ratio Series for ' + title

    if shading and shading_color_map is not None:
        add_shading_legend_labels(
            ax, shading_color_map, all_dominance_ranges,
            time_unit, show_shading_bounds
        )

    ax.set_title(title)
    ax.set_xlabel(f'Cooling Time [{time_unit}]')
    ax.set_ylabel(ylabel)
    ax.set_xscale('log')
    ax.set_yscale(yscale)

    ax.grid()
    ax.legend(loc='center left', bbox_to_anchor=(1,0.5))

    return fig, pd.DataFrame(stats_rows)