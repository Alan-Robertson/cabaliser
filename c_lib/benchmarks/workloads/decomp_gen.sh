#!/bin/bash

EXE_NAME="decomp"
BASEFOLDER=$EXE_NAME

QBIT_START=$1
QBIT_INCR=$2
QBIT_STEPS=$3

QBIT_END=$(expr $QBIT_START + $(expr $QBIT_INCR '*' $QBIT_STEPS))

GATES_START=$4
GATES_INCR=$5
GATES_STEPS=$6

GATES_END=$(expr $GATES_START + $(expr $GATES_INCR '*' $GATES_STEPS))

RAN=$7

TSTART=$8
TSTEPS=$9

MSG=${10}

ARGS_PATH="$BASEFOLDER/args/$EXE_NAME"
STAT_WORKLOAD_FILE="$BASEFOLDER/$EXE_NAME-stat_workload.sh"
RECORD_WORKLOAD_FILE="$BASEFOLDER/$EXE_NAME-record_workload.sh"
STAT_RUNNER_FILE="$BASEFOLDER/$EXE_NAME-stat_runner.sh"
RECORD_RUNNER_FILE="$BASEFOLDER/$EXE_NAME-record_runner.sh"
STAT_RUNNER_NAME="$EXE_NAME-stat_runner.sh"
RECORD_RUNNER_NAME="$EXE_NAME-record_runner.sh"

# Remove old files
rm -f $STAT_WORKLOAD_FILE
rm -f $RECORD_WORKLOAD_FILE

# Makes argument path
mkdir -p $ARGS_PATH


# Check to see if we have the correct number of arguments

if [ $# -ne 10 ]; then

	echo "You have not specified the correct number of arguments to generate workloads"
	echo "Please supply: <QBIT_START> <QBIT_INCR> <QBIT_STEPS> <GATES_START> <GATES_INCR> <GATES_STEPS>"\
		"<RANDOM> <THREAD_START> <THREAD_STEPS> <MSG>"
	exit 1
fi



# Will generate perf runners from templates
python ../templates/perf_stat_template.py $EXE_NAME 'OMP_NUM_THREADS=$THREADS' QBITS GATES RAN THREADS \
	> $STAT_RUNNER_FILE
python ../templates/perf_record_template.py $EXE_NAME 'OMP_NUM_THREADS=$THREADS' QBITS GATES RAN THREADS \
	> $RECORD_RUNNER_FILE

# Will generate files that can be used with xargs

 
TCURR=$TSTART
for i in $(seq 0 $TSTEPS); do
	
	for QARG in $(seq $QBIT_START $QBIT_INCR $QBIT_END); do
		for GARG in $(seq $GATES_START $GATES_INCR $GATES_END); do
			ARGFILE_NAME="$QARG-$GARG-$RAN-$TCURR-$MSG.args"
			WORK_ARGS="$QARG $GARG $RAN $TCURR $MSG" > $ARGS_PATH/$ARGFILE_NAME

			echo "bash $STAT_RUNNER_NAME $WORK_ARGS" >> $STAT_WORKLOAD_FILE
			echo "bash $RECORD_RUNNER_NAME $WORK_ARGS" >> $RECORD_WORKLOAD_FILE
		done
	done

	TCURR=$(expr $TCURR '*' 2)
done

