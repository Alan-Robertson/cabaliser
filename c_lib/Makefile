SHELL := /bin/bash
CC := gcc 
OMPFLAG := -fopenmp
CFLAGS := -Werror -Wvla -Wall --warn-no-unused-variable -Wunused-result ${OMPFLAG}

# Optimisation Flags
CFLAGS += -Ofast -fPIC
CFLAGS += -mavx2 -mlzcnt -mbmi2 

TARGET := lib_cabaliser.so
TARGET_FLAGS := -shared -Wl,-soname,${TARGET}  
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := build

LIBS := -I${LIBDIR}

TESTDIR := tests
TEST_SRCDIR := ${TESTDIR}/src
TEST_LIBDIR := ${TESTDIR}/lib
TEST_LIBS := -I${TEST_LIBDIR}

BENCHMARK_DIR := benchmarks
BENCHMARK_SRCDIR := ${BENCHMARK_DIR}/src

# Source Files
SRCFILES := $(wildcard ${SRCDIR}/*.c)
OBJFILES := $(patsubst ${SRCDIR}/%.c, ${BUILDDIR}/%.o, ${SRCFILES})
COND_OBJFILES := ${OBJFILES} 

# Transpose requires disabling some loop mangling
# The loop vectorisation flags don't mesh well with the casts between pointer types
TRANSPOSE := simd_transpose
TRANSPOSE_TEST := test_transpose
TRANSPOSE_CFLAGS := -march=native -O2 -fno-tree-loop-vectorize -fno-peel-loops
TRANSPOSE_CFLAGS += -fgcse-after-reload -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpredictive-commoning -fsplit-loops -fsplit-paths -ftree-loop-distribution -ftree-partial-pre -funswitch-loops -fvect-cost-model=dynamic -fversion-loops-for-strides  
TRANSPOSE_CFLAGS_CLANG := -march=native -O1

# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

# TODO: Relative pathing 
PAULI_TRACKER_CLIB := pauli_tracker_clib
PAULI_TRACKER_DIR := pauli_tracker
PAULI_TRACKER_RUSTLIB_DIR := pauli_tracker
PAULI_TRACKER_LIB := $(shell pwd)/${PAULI_TRACKER_RUSTLIB_DIR}/target/release
PAULI_TRACKER_HEADERS := ${PAULI_TRACKER_DIR}/lib

LINK_LIBS := -L${PAULI_TRACKER_LIB} -Wl,-Bstatic -l${PAULI_TRACKER_CLIB} -Wl,-Bdynamic 

LIBS += -I${PAULI_TRACKER_HEADERS} 

# Benchmark Files
BENCHMARK_SRCFILES := $(wildcard ${BENCHMARK_SRCDIR}/*.c)
BENCHMARK_RUNNERS := $(patsubst ${BENCHMARK_SRCDIR}/%.c, ${BENCHMARK_DIR}/%.out, ${BENCHMARK_SRCFILES})

.PHONY: all
.PHONY: clean
.PHONY: test tests
.PHONY: benchmark benchmarks
.PHONY: pandora_api
.PHONY: ${PAULI_TRACKER}


# Build Rules
all : ${TARGET} ${BUILDDIR}



include pandora_api/Makefile 
pandora : LIBS += ${PANDORA_LIBS} 
pandora : PANDORA_LIBS += -I${PAULI_TRACKER_HEADERS} 
pandora : COND_OBJFILES += ${PANDORA_OBJFILES} 
pandora : pandora_api all
	@echo ${LIBS}

debug_L3: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug_L3: test 

debug_L2: CFLAGS += -DDEBUG -DDEBUG_LEVEL=2
debug_L2: test 

debug_L1: CFLAGS += -DDEBUG -DDEBUG_LEVEL=1
debug_L1: test 

debug: CFLAGS += -fopt-info -fopt-info-loop -fopt-info-loop-missed -fopt-info-vec -fopt-info-vec-missed
debug: test 

tests: test
test: all
test : CFLAGS += -g
#tests : test pandora_test ${PAULI_TRACKER_LIB}
test: ${BUILDDIR} ${OBJFILES} ${TEST_RUNNERS} ${PANDORA_TEST_RUNNERS} 


benchmark: benchmarks 
benchmarks: CFLAGS += ${OMPFLAG} -pg
benchmarks: ${TARGET} ${BUILDDIR} ${OBJFILES} ${BENCHMARK_RUNNERS} 

${BUILDDIR} :
	mkdir -p ${BUILDDIR}

${TARGET} : ${BUILDDIR} ${OBJFILES} ${COND_OBJFILES} ${PAULI_TRACKER_LIB}
	${CC} ${CFLAGS} ${TARGET_FLAGS} -o $@ ${COND_OBJFILES} ${LINK_LIBS} ${LIBS}  

# Compilation of the transpose operation
${BUILDDIR}/${TRANSPOSE}.o : ${SRCDIR}/${TRANSPOSE}.c
ifeq ($(CC),gcc) # Clang doesn't support some of the optimisation flags
		${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LIBS} -c -o $@ $^
else
		${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS_CLANG} ${LIBS} -c -o $@ $^
endif

# Dependent libraries
pauli_tracker: ${PAULI_TRACKER_LIB}

${PAULI_TRACKER_LIB}: 
	cd ${PAULI_TRACKER_DIR}; make all

${BUILDDIR}/%.o : ${SRCDIR}/%.c
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${LIBS} -c -o $@ 

${TESTDIR}/%.out : ${OBJFILES} ${TEST_SRCDIR}/%.c
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${TEST_LIBS} ${LIBS} -o $@ 

${TESTDIR}/${TEST_TRANSPOSE}.out : ${OBJFILES} ${TEST_SRCDIR}/${TEST_TRANSPOSE}.c 
	${CC} $^ ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LINK_LIBS} ${LIBS} -o $@ 

${BENCHMARK_DIR}/%.out : ${OBJFILES} ${BENCHMARK_SRCDIR}/%.c 
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${LIBS} -o $@

obj:
	@echo ${OBJFILES}

# Clean build directory
clean: 
	rm -rf ${BUILDDIR}
	rm -rf ${TESTDIR}/*.out
	rm -rf ${BENCHMARK_DIR}/*.out
	cd ${PAULI_TRACKER_DIR}; make clean
