SHELL := /bin/bash
CC := gcc
# W-unused is ignored due to an Apple clang bug w/ static inline functions
CFLAGS := -Werror -Wvla -Wall -Wno-unused --warn-no-unused-variable -Wunused-result

# Determine architecture based on uname -m
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
ARCHITECTURE := x86_64
# This is required because uname -m can return anything of the form arm<specific version>
else ifneq ($(filter arm% aarch64,$(UNAME_M)),)
ARCHITECTURE := arm
else
ARCHITECTURE := UNKNOWN
$(error Architecture $(UNAME_M) is not recognised)
endif

# Determine platform based on uname -s
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
PLATFORM := OSX
CC := clang
else ifeq ($(UNAME_S),Linux)
PLATFORM := Linux
else
PLATFORM := UNKNOWN
$(error Platform $(UNAME_S) is unsupported)
endif

# Optimisation Flags
ifeq ($(CC),clang)
CFLAGS += -O3 -ffast-math -fPIC
else
CFLAGS += -Ofast -fPIC
endif

ifeq ($(ARCHITECTURE),x86_64)
CFLAGS += -mavx2 -mlzcnt -mbmi2
endif

TARGET := lib_cabaliser.so
ifeq ($(PLATFORM),OSX)
TARGET_FLAGS := -shared -Wl,-install_name,${TARGET}
else
TARGET_FLAGS := -shared -Wl,-soname,${TARGET}
endif

SRCDIR := src
ifneq ($(filter $(ARCHITECTURE),arm aarch64),)
SIMD_DIR := arm_simd
else
SIMD_DIR := simd
endif
LIBDIR := lib
BUILDDIR := build

LIBS := -I${LIBDIR} -I${LIBDIR}/simd

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


SIMD_SRCDIR := ${SRCDIR}/${SIMD_DIR}
SIMD_SRCFILES := $(wildcard ${SIMD_SRCDIR}/*.c)
SIMD_OBJFILES := $(patsubst ${SIMD_SRCDIR}/%.c, ${BUILDDIR}/simd/%.o, ${SIMD_SRCFILES})

OBJFILES += ${SIMD_OBJFILES}

# Transpose requires disabling some loop mangling
# The loop vectorisation flags don't mesh well with the casts between pointer types
SIMD_TESTS := test_transpose test_rowsum
ifeq ($(CC),clang)
SIMD_CFLAGS := -march=native -O2
SIMD_CFLAGS += -mllvm -enable-loopinterchange -fno-slp-vectorize -fno-vectorize -mllvm -enable-unroll-and-jam -mllvm -enable-nontrivial-unswitch
else
SIMD_CFLAGS := -march=native -O2 -fno-tree-loop-vectorize -fno-peel-loops
SIMD_CFLAGS += -fgcse-after-reload -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpredictive-commoning -fsplit-loops -fsplit-paths -ftree-loop-distribution -ftree-partial-pre -funswitch-loops -fvect-cost-model=dynamic -fversion-loops-for-strides
endif

# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

# TODO: Relative pathing
PAULI_TRACKER_CLIB := pauli_tracker_clib
PAULI_TRACKER_DIR := pauli_tracker
PAULI_TRACKER_RUSTLIB_DIR := pauli_tracker
PAULI_TRACKER_LIB := $(shell pwd)/${PAULI_TRACKER_RUSTLIB_DIR}/target/release
PAULI_TRACKER_HEADERS := ${PAULI_TRACKER_DIR}/lib

ifeq ($(PLATFORM),OSX)
LINK_LIBS := -L${PAULI_TRACKER_LIB} -l${PAULI_TRACKER_CLIB}
else
LINK_LIBS := -L${PAULI_TRACKER_LIB} -Wl,-Bstatic -l${PAULI_TRACKER_CLIB} -Wl,-Bdynamic
endif


LIBS += -I${PAULI_TRACKER_HEADERS}

# Benchmark Files
BENCHMARK_SRCFILES := $(wildcard ${BENCHMARK_SRCDIR}/*.c)
BENCHMARK_RUNNERS := $(patsubst ${BENCHMARK_SRCDIR}/%.c, ${BENCHMARK_DIR}/%.out, ${BENCHMARK_SRCFILES})

.PHONY: all
.PHONY: clean
.PHONY: test tests
.PHONY: run_tests
.PHONY: benchmark benchmarks
.PHONY: ${PAULI_TRACKER}


# Build Rules
all : ${TARGET} ${BUILDDIR}

debug_L3: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug_L3: test

debug_L2: CFLAGS += -DDEBUG -DDEBUG_LEVEL=2
debug_L2: test

debug_L1: CFLAGS += -DDEBUG -DDEBUG_LEVEL=1
debug_L1: test

debug: CFLAGS += -fopt-info -fopt-info-loop -fopt-info-loop-missed -fopt-info-vec -fopt-info-vec-missed
debug: test

tests: test pauli_tracker
test: all
test : CFLAGS += -g
test: ${BUILDDIR} ${OBJFILES} ${TEST_RUNNERS} ${PANDORA_TEST_RUNNERS}

run_tests: test
	$(for i in tests/*.out; do echo $i; ./$i; done)


benchmark: benchmarks
ifeq ($(PLATFORM),Linux)
benchmarks: CFLAGS += -pg
endif
benchmarks: ${TARGET} ${BUILDDIR} ${OBJFILES} ${BENCHMARK_RUNNERS}

${BUILDDIR} :
	mkdir -p ${BUILDDIR}
	mkdir -p ${BUILDDIR}/simd


${TARGET} : ${BUILDDIR} ${OBJFILES} ${SIMD_OBJFILES} ${COND_OBJFILES} ${PAULI_TRACKER_LIB}
	${CC} ${CFLAGS} ${TARGET_FLAGS} -o $@ ${COND_OBJFILES} ${SIMD_OBJFILES} ${LINK_LIBS} ${LIBS}

# Compilation of the transpose operation
# TODO: TRANSPOSE and TRANSPOSE_CFLAGS is currently unset
# 		(switch at definition for clang vs gcc as needed)
${BUILDDIR}/${TRANSPOSE}.o : ${SRCDIR}/${TRANSPOSE}.c
	${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LIBS} -c -o $@ $^

# Dependent libraries
pauli_tracker: ${PAULI_TRACKER_LIB}

# Pass active CC along to the pauli_tracker compilation
${PAULI_TRACKER_LIB}:
	cd ${PAULI_TRACKER_DIR}; ${MAKE} all CC=$(CC)

${BUILDDIR}/%.o : ${SRCDIR}/%.c
ifeq ($(CC),clang) # Clang errors for unused linker flags
	${CC} $^ ${CFLAGS} ${LIBS} -c -o $@
else
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${LIBS} -c -o $@
endif

${BUILDDIR}/simd/%.o : ${SRCDIR}/${SIMD_DIR}/%.c
	${CC} $^ ${CFLAGS} ${SIMD_CFLAGS} ${LIBS} -c -o $@

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
