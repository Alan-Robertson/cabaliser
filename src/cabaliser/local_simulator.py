from functools import reduce, partial
from itertools import chain

import numpy as np
from numpy import array

def widget_to_operations(widget, table=None) -> tuple:
    '''
        Convertsa widget into a sequence of unitaries
    '''
    initial_state = kr(*[plus_state] * widget.n_qubits)
    cz_operations = widget_to_cz(widget)
    local_clifford_operations = local_cliffords(widget)
    non_clifford_operations = non_clifford_rotations(widget, table=table)

    measurements = measurement_schedule(widget)

    return cz_operations, local_clifford_operations, non_clifford_operations, measurements

def simulate_widget(widget, *input_states, table=None) -> np.array:
    if len(input_states) == 0:
        input_states = [state_prep(1, 0) for _ in range(widget.n_initial_qubits)]

    cz_operations, local_cliffords, non_cliffords, measurements = widget_to_operations(widget, table=table)
    n_inputs = len(input_states)
    if n_inputs > widget.n_initial_qubits:
        raise Exception("More input states than inputs")

    graph_state = kr(
        *[plus_state] * widget.n_qubits,
        *input_states)
    higher_hilbert_space = kr(*[I] * len(input_states))

    cz_operations = kr(cz_operations, higher_hilbert_space)
    local_cliffords = kr(local_cliffords, higher_hilbert_space)
    non_cliffords = kr(non_cliffords, higher_hilbert_space)
    measurements = kr(measurements, higher_hilbert_space)

    input_operations = inject_inputs(widget.n_qubits, n_inputs)

    return norm(
            measurements
            @ non_cliffords
            @ local_cliffords
            @ input_operations
            @ cz_operations
            @ graph_state
        )

def state_prep(alpha, beta):
    '''
        Simple state prep helper function
    '''
    state = alpha * zero_state + beta * one_state
    state = norm(state)
    return state


def widget_to_cz(widget):
    '''
        Turns an adjacencies object into a sequence of CZ operations
    '''
    op = kr(*[I] * widget.n_qubits)

    for qubit_index, adjacencies in enumerate(
            map(
                partial(
                    type(widget).get_adjacencies,
                    widget
                ),
                range(widget.n_qubits)
            )):
        targets = list(filter(lambda x: x > qubit_index, adjacencies.to_list()))
        if len(targets) > 0:
            op = CZZ(
                    widget.n_qubits,
                    qubit_index,
                    *targets
                 ) @ op
    return op

def non_clifford_rotations(widget, table=None):
    '''
        Calculates the non-clifford rotations of a widget
    '''
    if table is None:
        table = {0: I, 1: T}

    ops = kr(
            *list(
                table[tag] for tag in widget.get_measurement_tags()
            )
    )
    return ops

def local_cliffords(widget):
    '''
        Calculates the local Clifford operations of a Widget
    '''
    cliffords = {'I': I, 'H': H, 'S': S, 'Sd': Sdag}
    op = kr(*list(
        cliffords[i] for i in widget.get_local_cliffords().to_list()
        ))
    return op

def measurement_schedule(widget):
    '''
        Implements the measurement schedule of a widget
    '''
    op = kr(*[I] * widget.n_qubits)
    qubit_index = lambda i: next(iter(i))
    io = widget.get_io_map().to_list()
    for layer in widget.get_schedule():
        for targ in map(qubit_index, layer):
            if targ not in io: # Don't measure outputs
                op = measure_x([targ], widget.n_qubits) @ op
                op = kr(*[I] * targ, H, *[I] * (widget.n_qubits - 1 - targ)) @ op
    return op

def inject_inputs(n_graph_qubits: int, n_inputs: int):
    '''
        Injects inputs into the widget
    '''
    return (
        cleanup_inputs(n_graph_qubits, n_inputs)
        @ measure_inputs(n_graph_qubits, n_inputs)
        @ cz_inputs(n_graph_qubits, n_inputs)
        )

def cz_inputs(n_graph_qubits: int, n_inputs: int):
    '''
        Performs a sequence of CZs to inject inputs
    '''
    n_qubits = n_graph_qubits + n_inputs
    return reduce(
        lambda a, b: a @ b,
        (CZ(n_qubits, i, i + n_graph_qubits) for i in range(n_inputs)),
        kr(*[I] * n_qubits)
        )

def cleanup_inputs(n_graph_qubits: int, n_inputs: int):
    '''
        Hadamards the input qubits to return them to the 0 state
    '''
    return kr(*[I] * n_graph_qubits, *[H] * n_inputs)

def measure_inputs(n_graph_qubits: int, n_inputs: int):
    '''
        Input measurements all commute
    '''
    n_qubits = n_graph_qubits + n_inputs

    return reduce(lambda a, b: a @ b, (measure_x([i], n_qubits) for i in range(n_graph_qubits, n_qubits)), kr(*[I] * n_qubits))

def kr(*args):
    '''
        Kronnecker product
    '''
    return reduce(np.kron, filter(lambda x: len(x) > 0, args))

def dop(x):
    '''
        Density Operator
    '''
    return np.kron(x, x.conjugate().transpose())

def vec(x, non_zero=False, eps=3):
    '''
        Prints the non-zero elements of the statevector  
    '''
    n_bits = int(np.ceil(np.log2(x.shape[0
 ])))
    for i, val in enumerate(x):
        if non_zero or np.abs(val[0]) >= 10**(-1 * eps):
            idx = bin(i)[2:].zfill(n_bits)
            print(f"{np.round(val, eps)}|{idx}>")

def norm(x, eps=15):
    '''
        Normalises a vector
    '''
    return np.round(x * sum(map(lambda i: np.abs(i ** 2), x)) ** -0.5, eps)

'''
    Begin Operator and State definitions
'''

I = np.eye(2)

X = array([[0, 1],
      [1, 0]])
Y = array([[ 0.+0.j, -0.-1.j],
      [ 0.+1.j,  0.+0.j]])
Z = array([[ 1,  0],
      [ 0, -1]])

PAULIS = [I, X, Y, Z]

H = 2 ** -0.5 * array([[ 1,  1],
      [ 1, -1]])

S = array([[ 1,  0],
      [ 0, 1j]])
Sdag = Z @ S

T = np.eye(2, dtype=np.complex64)
T[1][1] = 2 ** -0.5 *(1 + 1j)
Tdag = S @ T

SINGLE_QUBIT_CLIFFORDS = [H, S, Sdag]


CNOT_mat = array([[1, 0, 0, 0],
       [0, 1, 0, 0],
       [0, 0, 0, 1],
       [0, 0, 1, 0]])
CZ_mat = array([[1, 0, 0, 0],
       [0, 1, 0, 0],
       [0, 0, 1, 0],
       [0, 0, 0, -1]])

CNOT_mat_r = array([[1., 0., 0., 0.],
       [0., 0., 0., 1.],
       [0., 0., 1., 0.],
       [0., 1., 0., 0.]])

SWAP_mat = CNOT_mat @ CNOT_mat_r @ CNOT_mat

toff_mat = np.eye(8)
toff_mat[6:,6:] = X

u = array([[1],
       [0]])
zero_state = u
d = X @ u
one_state = d
p = H @ u
plus_state = p
m = H @ d
minus_state = m

x_u = H @ u
x_d = H @ d
z_u = u
z_d = d
y = S @ z_u

states = {
    '|0>':u,
    '|1>':d,
    '|+>':p,
    '|->':m,
    '|Y>':S @ p,
    '|S>':S @ p,
    '|T>':T @ p
}
states_dressed = {}
for key in states:
    states_dressed[key[1]] = states[key]
states |= states_dressed
del states_dressed
def state_string(*args):
    '''
        Niceness of notation
    '''
    return kr(*list(map(
        states.__getitem__,
        reduce(chain, map(
                lambda x: iter(x[1:-1]),
                args
        ))
    )))

def _CNOT(n_qubits=2, ctrl=0, targ=1):
    return two_qubit_gate(CNOT_mat, n_qubits, ctrl, targ)

def commuting_gate(gate, ctrl, *args, n_qubits=3):
    return reduce(lambda x, y: x @ y, map(partial(gate, n_qubits, ctrl), args))

def _CZ(n_qubits=2, ctrl=0, targ=1):
    '''
        Two qubit CZ operation
    '''
    return two_qubit_gate(CZ_mat, n_qubits, ctrl, targ)

def CS(n_qubits=2, ctrl=0, targ=1):
    '''
        Controlled Phase gate operation
    '''
    CS_mat = np.eye(4, dtype=np.complex128)
    CS_mat[2:, 2:] = S
    return two_qubit_gate(CS_mat, n_qubits, ctrl, targ)

def CXX(n_qubits, ctrl, *targs):
    '''
        Multi target CX operation
    '''
    return commuting_gate(_CNOT, ctrl, *targs, n_qubits=n_qubits)

# CNOT is equivalent to CXX 
CNOT = CXX
# Reverse CNOT
CNOT_r = _CNOT(ctrl=1, targ=0)

def CZ(n_qubits, ctrl, *targs):
    '''
        CZ operation
        Single control, arbitrary number of targets
    '''
    return commuting_gate(_CZ, ctrl, *targs, n_qubits=n_qubits)

# CZZ is equivalent to multi-target CZ
CZZ = CZ

def SWAP(n_qubits=2, ctrl=0, targ=1):
    '''
        Two qubit SWAP gate
    '''
    return two_qubit_gate(SWAP_mat, n_qubits, ctrl, targ)

def Toffoli(n_qubits=3, ctrl_0=0, ctrl_1=1, targ=2):
    '''
        Toffoli gate
    '''
    return n_qubit_gate(toff_mat, n_qubits, 3, ctrl_0, ctrl_1, targ)

def n_qubit_gate(gate, n_qubits, n_qubits_gate, *args):
    '''
        Composes an n qubit gate over an arbitrary set of targets using swaps 
    '''
    mat = kr(gate, *([I] * (n_qubits - n_qubits_gate)))

    swap_mat = kr(*([I] * n_qubits))
    permutation = {i:i for i in range(n_qubits)}

    for position, target_qubit in enumerate(args):
        if position != permutation[target_qubit]:
            swap_mat = SWAP(n_qubits, position, permutation[target_qubit]) @ swap_mat
            permutation[position] = permutation[target_qubit]
            permutation[target_qubit] = position

    return swap_mat.transpose() @ mat @ swap_mat

def two_qubit_gate(gate, n_qubits, ctrl, targ):
    '''
        Constructs an arbitrary two qubit gate via swaps
    '''
    mat = kr(gate, *([I] * (n_qubits - 2)))
    targ_curr = targ
    swap_mat = kr(*([I] * n_qubits))

    for i in range(ctrl, 0, -1):
        swap_idx = [i - 1, i]
        swap_mat = (kr(
                        *([I] * swap_idx[0]),
                        SWAP_mat,
                         *([I] * (n_qubits - swap_idx[1] - 1))
                    ) @ swap_mat)

        if targ_curr == i - 1:
            targ_curr = i

    for i in range(targ_curr, 1, -1):
        swap_idx = [i - 1, i]

        swap_mat = (kr(
                        *([I] * swap_idx[0]),
                        SWAP_mat,
                         *([I] * (n_qubits - swap_idx[1] - 1))
                    ) @ swap_mat)
    return swap_mat.transpose() @ mat @ swap_mat


'''
    Measurement operations
'''
_MEASUREMENT = lambda measurement_operator: lambda measurement_targets, n_qubits: kr(*([I, measurement_operator][i in measurement_targets] for i in range(n_qubits)))

measure_x = _MEASUREMENT(dop(x_u))
measure_x_0 = measure_x
measure_x_1 = _MEASUREMENT(dop(x_d))

measure_z = _MEASUREMENT(dop(z_u))
measure_z_0 = measure_z
measure_z_1 = _MEASUREMENT(dop(z_d))
