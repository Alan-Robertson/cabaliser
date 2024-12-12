
import sys

def help_msg_and_exit():
        print("Please supply an executable name at minimum\n")
        print("Use -- for second argument for omitting environment string\n")
        print("Format: python perf_stat_template.py <EXE> [ENV_STR] [<PARAM_NAMES>,]")
        print("Example 1: python perf_stat_template.py decomp -- QBITS GATES RAN THREADS")
        print("Example 2: python perf_stat_template.py decomp 'OMP_NUM_THREADS=$THREADS' "\
                    "QBITS GATES RAN THREADS")
        sys.exit(1)

if len(sys.argv) < 3:
    if len(sys.argv) < 2:
        help_msg_and_exit()
    else:
        if sys.argv[1] == '--help' or sys.argv[1] == '-h':    
            help_msg_and_exit()
        else:
            print("No environment variable set, emitting empty string")

PERF_RECORD_TEMPLATE_STR = """
#!/bin/bash

if [ $# -lt {PARAM_COUNT} ]; then
	echo "Perf Record - {EXE}"
	echo "Please supply: {PARAM_NAMES}"
	exit 1
fi

{VAR_GEN}

BENCH_MSG=${LAST_ARG}

START_DATETIME=$(date  +"%Y%m%d_%H%M%S");
NAME="record-{EXE}"
OUTPUT_PATH=output/$START_DATETIME-$NAME
OUTPUT_NAME=record-{EXE}-{PARAM_OUTFILE}.txt

PERF_FIELDS="overhead,overhead_us,overhead_sys,pid,mispredict,symbol,parent,cpu,srcline,sample"
FILEPATH=$OUTPUT_PATH/$OUTPUT_NAME;

cp -f ../{EXE}.out ./
mkdir -p $OUTPUT_PATH

echo "Running Perf Record"
{ENV} perf record --fields $PERF_FIELDS ./{EXE}.out {VAR_REFS} \
        --stdio > $FILEPATH

echo "Testing Note: $BENCH_MSG" | cat - $OUTPUT_NAME > $FILEPATH

echo "$START_DATETIME,$END_DATETIME,{EXE},$BENCH_MSG,$FILEPATH" >> output/record_tracker.csv 
rm $OUTPUT_NAME
echo "Completed"
"""

exe_str = sys.argv[1]

env_str = '' if len(sys.argv) < 2 else '' if sys.argv[2] == '--' else sys.argv[2]

param_names = sys.argv[3:]
param_count = len(param_names)
param_var_gen = [ "{}=${}".format(param_names[i], i+1) 
                 for i in range(len(param_names)) ]
param_var_refs = [ "${}".format(pname) 
                  for pname in param_names ]


output_str = PERF_RECORD_TEMPLATE_STR.format(
    EXE = exe_str,
    ENV = env_str,
    PARAM_COUNT = param_count,
    PARAM_NAMES = " ".join(param_names),
    PARAM_OUTFILE = "-".join(param_var_refs),
    VAR_GEN = "\n".join(param_var_gen),
    VAR_REFS = " ".join(param_var_refs),
    LAST_ARG = param_count + 1)

print(output_str)


