# Template Scripts

Each python script within here is used to generate a perf script to make
life a little easier when producing benchmarks and executing test cases.

At the moment, only two perf templates are here

* stat
* record

These are prioritised as they typically provide the most useful bits of
information.

## How To

Select a script and you should be able to execute the following :

```sh

python <template file> <executable> [<param>, ...]
```

Example 1 (omitting environment string as second argument):
```
python perf_stat_template.py decomp.out -- QBIT GATES RANDOM
```

Example 2 (omitting environment string for setting `OMP_NUM_THREADS`):
```
python perf_stat_template.py decomp.out OMP_NUM_THREADS=$THREADS QBIT GATES RANDOM THREADS
```

## Results

The results of the execution are stored in `output/` folder along with it being
encoded by the date and time (start and end time). Since the same folder is used by
all scripts executing in the current directory, additional information is held
in a `*_tracker.csv` file, (`*`), being a placeholder for `stat` and `record` which
will append the latest execution to the end of the csv.


