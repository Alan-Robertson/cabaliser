import unittest
import numpy as np

from itertools import combinations_with_replacement, permutations
from functools import reduce

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser import local_simulator 

MAX_CLIFFORD_DEPTH = 12
EPS = 1e-7
N_REPETITIONS = 100


class CliffordsTest(unittest.TestCase):

    def test_clifford_depth_1(self):
        self.clifford_sequences(depth=1)

    def test_clifford_depth_2(self):
        self.clifford_sequences(depth=2)

    def test_clifford_depth_3(self):
        self.clifford_sequences(depth=3, test_states=False) # Floating point kicks in around here

    def test_clifford_depth_4(self):
        self.clifford_sequences(depth=4, test_states=False)

    def test_clifford_depth_5(self): # Past depth 5 exhaustive search's exponential scaling hits hard
        self.clifford_sequences(depth=5, test_states=False)

    def clifford_sequences(self, depth=1, test_states=True):
        '''
           Sequence of random local cliffords 
        '''
        n_qubits = 1
        max_qubits = 3
        
        #for i in N_REPETITIONS: 

        # Test all two qubit clifford sequences
        
        for op_seq in combinations_with_replacement(gates.SINGLE_QUBIT_GATES, depth):
            for op_perm in permutations(op_seq):
                operations = [(op, (0,)) for op in op_perm]

                wid = Widget(n_qubits, max_qubits)

                ops = OperationSequence(len(operations))
                for opcode, args in operations: 
                    ops.append(opcode, *args)

                operation = reduce(
                    lambda a, b: b @ a,
                    map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations),
                    local_simulator.I
                 )

                wid(ops) 
                wid.decompose()

                wid_operation = wid.get_local_cliffords().to_list()[1]
                if not cmp_gates(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation], operation):

                    print("OPs:", list(map(gates.SINGLE_QUBIT_GATE_ARR.__getitem__, map(lambda x: x[0], operations))))
                    print("Operation: ", operation)
                    print(wid_operation)
                    print(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation])
                    print(wid.get_local_cliffords().to_list())

                    assert False

                if test_states:
                    ## Test application to random states
                    for _ in range(N_REPETITIONS):
                        input_state = local_simulator.state_prep(
                            *list(np.random.random(2))
                        )

                        widget_state = local_simulator.simulate_widget(
                             wid,
                             input_state=input_state,
                             trace_graph=True
                        )
                        effective_state = (
                             operation @ input_state
                        )
                        
                        eff_const = effective_state[0][0]
                        wid_const = widget_state[0][0] 
                        if eff_const == 0:
                            eff_const = effective_state[0][1] 
                        if wid_const == 0:
                            wid_const = widget_state[0][1]
                        if (np.abs(effective_state / eff_const - widget_state / wid_const) > EPS).any():
                            print("Operation: ", operation)
                            print(input_state)
                            print(effective_state)
                            print(widget_state)
                            assert False


    def test_random_cliffords(self, seed=0):
        generator = np.random.default_rng(seed)
        n_qubits = 1
        max_qubits = 3
        for i in range(N_REPETITIONS):
            operations = list(map(lambda x: (x, (0,)), generator.choice(
            tuple(gates.SINGLE_QUBIT_GATES),
            MAX_CLIFFORD_DEPTH,
            replace=True))) 

            wid = Widget(n_qubits, max_qubits)

            ops = OperationSequence(len(operations))
            for opcode, args in operations: 
                ops.append(opcode, *args)

            operation = reduce(
                lambda a, b: b @ a,
                map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations),
                local_simulator.I
             )

            wid(ops) 
            wid.decompose()

            wid_operation = wid.get_local_cliffords().to_list()[1]
            if not cmp_gates(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation], operation):

                print("OPs:", list(map(gates.SINGLE_QUBIT_GATE_ARR.__getitem__, map(lambda x: x[0], operations))))
                print("Operation: ", operation)
                print(wid_operation)
                print(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation])
                print(wid.get_local_cliffords().to_list())

                assert False



def cmp_gates(gate_a, gate_b, eps=1e-5):
    '''
        Testing gate equivalence
    '''
    tol = int(-1 * np.log10(eps))
    n_qubits = int(np.log2(gate_a.shape[0]))
    
    consts = None
    for basis in range(1 << n_qubits):
        basis_state = np.zeros(1 << n_qubits, dtype=np.complex128)  
        basis_state[basis] = 1 

        state = np.round(gate_a.transpose().conj() @ gate_b @ basis_state, tol) 
        if sum(map(lambda x: (x > eps), state)) > 1: 
            return False
        if consts is None:     
            consts = state[basis] 
        if any(map(lambda x: np.abs(x[0] - consts * x[1]) > eps, zip(state, basis_state))):
            return False
    return True



if __name__ == '__main__':
    unittest.main()
