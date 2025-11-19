# ALARA Output Processing

Contained within `ALARA/tools` is the Python package, `alara_output_processing`, a module desgined for the parsing of ALARA output files into Pandas DataFrame-inherited structures, with a robust set of operations to organize and process data to user specifications.

## Dependencies
- Standard Python libraries
    * [ArgParse](https://docs.python.org/3/library/argparse.html)
    * [IO](https://docs.python.org/3/library/io.html)
    * [Re](https://docs.python.org/3/library/re.html)
- Generic Python packages
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
adfs = aop.DataLibrary.make_entries(lib, runs)
```
The columns for `adfs` are:
* `time`: Cooling time in seconds.
* `nuclide`: Nuclide name formatted as "element-A" (i.e. "h-1" for <sup>1</sup>H) or "total".
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
* `value`: Numeric value of the particular variable respone.

Below is the example `head()` of an `ALARADFrame`:


| | time | nuclide | run_lbl | block | block_num | variable | value |
|-|------|---------|---------|-------|-----------|----------|-------|
| 0 | 0.000000e+00 | h-1 | fendl2 | 0 | 1 | 0 | 1.176100e+22
| 1 | 3.153600e+02 | h-1 | fendl2 | 0 | 1 | 0 | 1.176100e+22
| 2 | 3.153600e+05 | h-1 | fendl2 | 0 | 1 | 0 | 1.176100e+22
| 3 | 3.153600e+07 | h-1 | fendl2 | 0 | 1 | 0 | 1.176100e+22
| 4 | 3.153600e+09 | h-1 | fendl2 | 0 | 1 | 0 | 1.176100e+22

The five rows in the head correspond to the number density of <sup>1</sup>H in the 1st interval of a run of FENDL2 data with four cooling times.

Once `adf` is created, `ALARADFrame.filter_rows()` can be called to select data that matches user specifications for one or more columns:

```
filtered_adf = adf.filter_rows(
    filter_dict={
        Column 1 : Value or [Value1, ..., Value N],
        ...
        Column N : Value or [Value1, ..., Value N]
    },
    inplace=False
)
```
The parameter `filter_dict` allows filtering over any number of columns and any number of filters per column, so long as multi-filters are input as a list. Filters are case-sensitive. Similarly to the Pandas parameter, `inplace` allows the user to decide whether to operate on the original `ALARADFRame` itself, or create a new frame.

**Note:** When filtering the `nuclide` column, `ALARADFrame.filter_rows()` has functionality to select all nuclides of a particular element, as well as selecting individual nuclides. To do so, instead of  `filter_dict["nuclide"] = "fe-55"`, write `filter_dict["nuclide"] = "fe"` to filter all iron isotopes, instead of just <sup>55</sup>Fe, for example. Similarly, multiple whole elements can be selected by inputting them as a list for `filter_dict["nuclide"]`, however, filtering by whole elements and individual nuclides cannot be done in the same filtering operation.

Below is an example filtering operation on the same `adf` from the above example:
```
fendl2_spec_act_h3 = adf.filter_rows({
    "run_lbl"  : "fendl2",
    "variable" : adf.VARIABLE_ENUM["Specific Activity"],
    "nuclide"  : "h-3" # Filtering out just tritium
})
```
The `head()` of `fendl2_spec_act_h3` is:

| | time | nuclide | run_lbl | block | block_num | variable | value |
|-|------|---------|---------|-------|-----------|----------|-------|
| 0 | 0.000000e+00 | h-3 | fendl2 | 0 | 1 | 0 | 1.988300e+09
| 1 | 3.153600e+02 | h-3 | fendl2 | 0 | 1 | 0 | 1.988300e+09
| 2 | 3.153600e+05 | h-3 | fendl2 | 0 | 1 | 0 | 1.987100e+09
| 3 | 3.153600e+07 | h-3 | fendl2 | 0 | 1 | 0 | 1.879900e+09
| 4 | 3.153600e+09 | h-3 | fendl2 | 0 | 1 | 0 | 7.334700e+06

The following operations are available on any given `ALARADFrame`, regardless of filtering:

* `ALARADFrame.extract_time_vals(parsing=False)`
    - Return the cooling times of an `ALARADFrame` from their column headers as a list in seconds. This is done internally when creating an `ALARADFrame` through `aop.FileParser.extract_tables()` or `aop.DataLibrary.make_entries()`, but can be done on any existing `ALARADFrame` to convert time values back to seconds if they have been otherwise converted using `ALARADFrame.convert_times()`. To operate on an existing `ALARADFrame`, set the optional parameter `parsing` to `True`.
* `ALARADFrame.convert_time(unit, inplace=False, pivot=False)`
    - Convert the units of the cooling times in an `ALARADFrame` from seconds to minutes (m), hours (h), days (d), weeks (w), years (y), or centuries (c). If desired to operate on the `ALARADFrame` itself, instead of a copy, set the optional parameter `inplace` to `True`. If operating on a pivot table (**see below for pivoting instructions**), set the optional parameter `pivot` to `True`.

The following operation requires filtering to one run and one response variable:
* `ALARADFrame.extract_totals()`
    - Select the values from the "total" row of an ALARA output table ALARADFrame and write them out to a list. If nuclides were previously filtered, this method will be able to return a non-empty list so long as "total" was not filtered out. Additionally, pivoted tables can be operated on, so long as the are converted back to a `ALARADFrame`s prior to operation.

Once data has been filtered, it can be useful to create a pivot table using `pandas.DataFrame.pivot()` to reorganize data by `nuclide` vs `time`. An example filtering and pivot table sequence (without nuclide filtering to show multiple multiple rows) is as such:
```
filtered_adf = adf.filter_rows({
    "run_lbl" : "fendl2",
    "variable" : adf.VARAIBLE_ENUM["Specific Activity"]
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

The following operation requires the pivoting of a filtered `ALARADFrame` as its input and is not a member of the `ALARADFrame` class:

* `aggregate_small_percentages(piv, relative=False, threshold=0.05)`
    -  Consolidate all rows in a pre-filtered and pivoted DataFrame derived from a larger ALARADFRame that do not have any cells with a contribution of more than a threshold value to the total for its respective column. Rows that do not have any cells that surpass its column's threshold are aggregated into a new "Other" row. If a row has at least one column value above the threshold, then the whole row is preserved.

    This function sets an automatic threshold percentage of 5%, but it can be modified by explicitly setting the optional `threshold` parameter to any proportional value.