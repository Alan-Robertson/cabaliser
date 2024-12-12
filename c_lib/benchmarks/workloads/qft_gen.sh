#!/bin/bash

EXE_NAME="qft"
BASEFOLDER=$EXE_NAME

QBIT_START=$1
QBIT_INCR=$2
QBIT_STEPS=$3

QBIT_END=$(expr $QBIT_START + $(expr $QBIT_INCR '*' $QBIT_STEPS))

TSTART=$4
TSTEPS=$5

MSG=$6

ARGS_PATH="$BASEFOLDER/args/$EXE_NAME"
STAT_WORKLOAD_FILE="$BASEFOLDER/$EXE_NAME-stat_workload.sh"
RECORD_WORKLOAD_FILE="$BASEFOLDER/$EXE_NAME-record_workload.sh"
STAT_RUNNER_FILE="$BASEFOLDER/$EXE_NAME-stat_runner.sh"
RECORD_RUNNER_FILE="$BASEFOLDER/$EXE_NAME-record_runner.sh"

# Check to see if we have the correct number of arguments

if [ $# -ne 6 ]; then

	echo "You have not specified the correct number of arguments to generate workloads"
	echo "Please supply: <QBIT_START> <QBIT_INCR> <QBIT_STEPS> <THREAD_START> <THREAD_STEPS> <MSG>"
	exit 1
fi

# Remove old files
rm -f $STAT_WORKLOAD_FILE
rm -f $RECORD_WORKLOAD_FILE

# Makes argument path
mkdir -p $ARGS_PATH


# Will generate perf runners from templates
python ../templates/perf_stat_template.py $EXE_NAME 'OMP_NUM_THREADS=$THREADS' QBITS THREADS \
	> $STAT_RUNNER_FILE
python ../templates/perf_record_template.py $EXE_NAME 'OMP_NUM_THREADS=$THREADS' QBITS THREADS \
	> $RECORD_RUNNER_FILE

# Will generate files that can be used with xargs

 
TCURR=$TSTART
for i in $(seq 0 $TSTEPS); do
	
	for QARG in $(seq $QBIT_START $QBIT_INCR $QBIT_END); do
		for GARG in $(seq $GATES_START $GATES_INCR $GATES_END); do
			ARGFILE_NAME="$QARG-$TCURR-$MSG.args"
			WORK_ARGS="$QARG $TCURR $MSG" > $ARGS_PATH/$ARGFILE_NAME

			echo "bash $STAT_WORKLOAD_FILE $EXE_NAME $WORK_ARGS" >> $STAT_WORKLOAD_FILE
			echo "bash $RECORD_WORKLOAD_FILE $EXE_NAME $WORK_ARGS" >> $RECORD_WORKLOAD_FILE
		done
	done

	TCURR=$(expr $TCURR '*' 2)
done

