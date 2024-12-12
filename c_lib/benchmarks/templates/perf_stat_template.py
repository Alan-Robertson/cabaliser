
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

PERF_STAT_TEMPLATE_STR = """
#!/bin/bash

if [ $# -ne {PARAM_COUNT} ]; then
	echo "Perf Stat - {EXE}"
	echo "Please supply: {PARAM_NAMES}"
	exit 1
fi

{VAR_GEN}

BENCH_MSG=${LAST_ARG}

START_DATETIME=$(date  +"%Y%m%d_%H%M%S");
NAME="stat-{EXE}"
OUTPUT_PATH=output/$START_DATETIME-$NAME
OUTPUT_NAME=stat-{PARAM_OUTFILE}.txt

PERF_EVENTS="task-clock,context-switches,cpu-migrations,page-faults,cycles,instructions,uops_issued.any,uops_executed.thread,mem_inst_retired.any"

FILEPATH=$OUTPUT_PATH/$OUTPUT_NAME;

cp -f ../{EXE}.out ./
mkdir -p $OUTPUT_PATH

echo "Running Perf Stat"
{ENV} perf stat -o $OUTPUT_NAME -x ',' \
	-e $PERF_EVENTS ./{EXE}.out {VAR_REFS}

echo "Testing Note: $BENCH_MSG" | cat - $OUTPUT_NAME > $FILEPATH

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


output_str = PERF_STAT_TEMPLATE_STR.format(
    EXE = exe_str,
    ENV = env_str,
    PARAM_COUNT = param_count,
    PARAM_NAMES = " ".join(param_names),
    PARAM_OUTFILE = "-".join(param_names),
    VAR_GEN = "\n".join(param_var_gen),
    VAR_REFS = " ".join(param_var_refs),
    LAST_ARG = param_count + 1)

print(output_str)


