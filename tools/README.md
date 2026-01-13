# ALARA Output Processing

Contained within `ALARA/tools` is the Python package, `alara_output_processing`, a module desgined for the parsing of ALARA output files into Pandas DataFrame-inherited structures, with a robust set of operations to organize and process data to user specifications.

## Dependencies
- Standard Python libraries
    * [ArgParse](https://docs.python.org/3/library/argparse.html)
    * [CSV](https://docs.python.org/3/library/csv.html)
    * [Operator](https://docs.python.org/3/library/operator.html)
    * [Warnings](https://docs.python.org/3/library/warnings.html)
- Generic Python packages
    * [NumPy](https://numpy.org/install/)
    * [Pandas](https://pandas.pydata.org/docs/getting_started/install.html)


## Installation
From the directory `ALARA/tools/` run the following command to install `alara_output_processing`:
```
pip install .
```

## Usage
`alara_output_processing` can be used either as a pure Python library, or as a command line tool. From the command line, calling `alara_output_processing` with the filepath to an ALARA output file will automatically identify and parse all text-formatted tables contained in the output file and write them out to individual CSV files (corresponding to each zone/interval of the ALARA run). To run `alara_output_processing` from the command line, run:
```
alara_output_processing -f /path/to/alara/output.txt
```

Alternatively, when used as a Python library, `alara_output_processing` can read in and parse ALARA output tables as such:
```
import alara_output_processing as aop

alara_data = aop.FileParser("/path/to/alara/output.txt")
output_tables = alara_data.extract_tables()
```

If comparing multiple ALARA runs, a dictionary can be constructed of the form:
```
runs = {
    "run1" : "/path/to/run1/output.txt",
    "run2" : "/path/to/run2/output.txt",
}
```

This dictionary can be input directly into the function `DataLibrary.make_entries()` to create a single `ALARADFrame` containing all data from each table in each run's output files.
```
lib = aop.DataLibrary()
adf = aop.DataLibrary.make_entries(lib, runs)
```
The columns for `adfs` are:
* `time`: Cooling time in seconds.
* `time_unit`: Units for cooling time.
* `nuclide`: Nuclide name formatted as "element-A" (i.e. "h-1" for <sup>1</sup>H) or "total".
* `half_life`: Half-life in seconds of an unstable nuclide. `-1` for stable nuclides, `0` for "total" rows.
* `run_lbl`: Distinguisher between runs (i.e. "run1", "run2", etc.).
* `block`: Integer enumerator for the geometric block key name. Possible block keys are "Interval", "Material", or "Zone", and their enumerator values can be accessed through `ALARADFrame().BLOCK_ENUM[block]`, where `block` is one of the above keys.
* `block_num`: Geometric position of the block.
* `variable`: Integer enumerator for the response variable key name. Possible variable keys are:
    - "Number Density"
    - "Specific Activity"
    - "Total Decay Heat"
    - "Alpha Heat"
    - "Beta Heat"
    - "Gamma Heat"
    - "Contact Dose"
    
    The respective enumerator values can be accessed through `ALARADFrame().VARIABLE_ENUM[variable]`, where `variable` is one of the above variables.
* `var_unit`: Unit for the corresponding variable.
* `value`: Numeric value of the particular variable respone.

Below is the example `head()` of an `ALARADFrame`:


||time|time_unit|nuclide|half_life|run_lbl|block|block_num|variable| var_unit|value|
|-|-|-|-|-|-|-|-|-|-|-|
| 0 | 's' | -1 | h-1 | -1 | fendl2 | 0 | 1 | 0 | atoms/kg | 0.000000e+0
| 1 | 's' | 0.000000e+00 | h-1 | -1 | fendl2 | 0 | 1 | 0 | atoms/kg | 1.176100e+22
| 2 | 's' | 3.153600e+02 | h-1 | -1 | fendl2 | 0 | 1 | 0 | atoms/kg | 1.176100e+22
| 3 | 's' | 3.153600e+05 | h-1 | -1 | fendl2 | 0 | 1 | 0 | atoms/kg | 1.176100e+22
| 4 | 's' | 3.153600e+07 | h-1 | -1 | fendl2 | 0 | 1 | 0 | atoms/kg | 1.176100e+22

The five rows in the head correspond to the number density of <sup>1</sup>H in the 1st interval of a run of FENDL2 data with four cooling times. Note that the 0<sup>th</sup> row's time of `-1` corresponds to the pre-irradiation state, and not any singular cooling time, like all positive and zero times do.

Once `adf` is created, `ALARADFrame.filter_rows()` can be called to select data that matches user specifications for one or more columns:

```
filtered_adf = adf.filter_rows(
    filter_dict={
        Column 1 : Value or [Value1, ..., Value N],
        ...
        Column N : Value or [Value1, ..., Value N]
    }
)
```
The parameter `filter_dict` allows filtering over any number of columns and any number of filters per column, so long as multi-filters are input as a list. Filters are case-sensitive, unless otherwise specified.

To filter pre-irradiation values, which are identified by `adf["time"] == -1` (see above), write `filter_dict["time"] = -1`. Otherwise, to filter post-irradiation cooling times, any other value for `filter_dict["time"]` will be accepted and will remove the pre-irradiation rows. For clarity, `filter_dict["time"] = "post_irradiation"` is recommended.

When filtering the `nuclide` column, `ALARADFrame.filter_rows()` has functionality to select all nuclides of a particular element, as well as selecting individual nuclides. To do so, instead of  `filter_dict["nuclide"] = "fe-55"`, write `filter_dict["nuclide"] = "fe"` to filter all iron isotopes, instead of just <sup>55</sup>Fe, for example. Similarly, multiple whole elements can be selected by inputting them as a list for `filter_dict["nuclide"]`. It is also possible to filter by a combination of whole elements and individual nuclides.

Additional nuclide filtering can be done on the stability of nuclides. To filter all stable nuclides, write `filter_dict["half_lives] = "stable"` or `filter_dict["half_lives] = -1`. To filter all unstable nuclides, write `filter_dict["half_lives] = "unstable"` or `filter_dict["half_lives] = "radioactive". ` Half-life filtering can also be done relative to certain time thresholds, such as filtering all nuclides with half-lives greater than 1e6 seconds. To do so write `filter_dict["half_lives] = ['>', 1e6]`. Generally, the format for this time-operator filtering is `filter_dict["half_lives] = [{operator}, {threshold}]`.

Finally, nuclides can be filtered on their presence in the initial material compositions or not. To filter only nuclides which existed pre-irradiation, write `filter_dict["nuclide"] = "initial"`. Conversely, to filter only new nuclides produced through neutron activation or as decay products, write `filter_dict["nuclide] = "transmuted"`.

Below is an example filtering operation on the same `adf` from the above example:
```
fendl2_spec_act_h3 = adf.filter_rows({
    "run_lbl"  : "fendl2",
    "variable" : adf.VARIABLE_ENUM["Specific Activity"],
    "nuclide"  : "h-3" # Filtering out just tritium,
    "time"     : "post_irradiation"
})
```
The `head()` of `fendl2_spec_act_h3` is:

||time|time_unit|nuclide|half_life|run_lbl|block|block_num|variable| var_unit|value|
|-|-|-|-|-|-|-|-|-|-|-|
| 0 | 's' | 0.000000e+00 | h-3 | 3.880000e+08 | fendl2 | 0 | 1 | 1 | Bq/kg | 1.988300e+09
| 1 | 's' | 3.153600e+02 | h-3 | 3.880000e+08 | fendl2 | 0 | 1 | 1 | Bq/kg | 1.988300e+09
| 2 | 's' | 3.153600e+05 | h-3 | 3.880000e+08 | fendl2 | 0 | 1 | 1 | Bq/kg | 1.987100e+09
| 3 | 's' | 3.153600e+07 | h-3 | 3.880000e+08 | fendl2 | 0 | 1 | 1 | Bq/kg | 1.879900e+09
| 4 | 's' | 3.153600e+09 | h-3 | 3.880000e+08 | fendl2 | 0 | 1 | 1 | Bq/kg | 7.334700e+06

Once data has been filtered, it can be useful to create a pivot table using `pandas.DataFrame.pivot()` to reorganize data by `nuclide` vs `time`. An example filtering and pivot table sequence (without nuclide filtering to show multiple multiple rows) is as such:
```
filtered_adf = adf.filter_rows({
    "run_lbl"  : "fendl2",
    "variable" : adf.VARIABLE_ENUM["Specific Activity"],
    "time"     : "post_irradiation"
})
pivot_df = filtered_adf.pivot(
    index="nuclide",
    columns="time",
    values="value"
)
```

The `head()` of `pivot_df` is:

| time | 0.000000e+00 | 3.153600e+02 | 3.153600e+05 | 3.153600e+07| 3.153600e+09 | 3.153600e+11|
|------|--------------|--------------|--------------|-------------|--------------|-------------|
| nuclide | | | | | | |
| ca-45 | 19910.0 | 19909.0 | 19603.0 | 4.222700e+03 | 0.0 | 0.0 |
| ca-47 | 177440.0 | 177340.0 | 101760.0 | 1.267500e-19 | 0.0 | 0.0 |
| co-57 | 99937.0 | 99936.0 | 99013.0 | 3.949700e+04 | 0.0 | 0.0 |
| co-58 | 497020000.0 | 497010000.0 | 481270000.0 | 1.417400e+07 | 0.0 | 0.0 |
| co-58m | 322330000.0 | 320150000.0 | 368720.0 | 0.000000e+00 | 0.0 | 0.0 |

The following operations are utility functions that are not members of the `ALARADFrame` class:

* `convert_times(vector, from_unit, to_unit)`
    - Convert the values in a vector to and from any units in the list of
        seconds (s), minutes (m), hours (h), days (d), weeks (w), years (y),
        or centuries (c). Can be used to convert the time column of an `ALARADFrame`, for example from seconds to years with the following command:
        ```
        adf['time'] = aop.convert_times(adf['time'], from_unit='s', to_unit='y')
        ```

* `aggregate_small_percentages(piv, relative=False, threshold=0.05)`
    - Consolidate all nuclides that do not have any row-level values greater than a given threshold for totals over all times in a pre-filtered ALARADFrame containing data from a single variable, response, and block. The 'value' column in the ALARADFrame can either contain absolute or relative data, requiring only that the user provide an appropriate threshold value.

    - Nuclides that do not contribute any values that surpass the threshold at any time are aggregated into a new "Other" row for each time. If a nuclide has a value that surpasses the threshold at any time, then all rows for said nuclide are preserved.
