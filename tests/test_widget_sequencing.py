import unittest
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget_sequence import WidgetSequence


class WidgetSequenceTest(unittest.TestCase):

    def test_small_qft(self):
        self.__test_qft(10, 30)

    def test_medium_qft(self):
        self.__test_qft(30, 100)

    def test_large_qft(self):
        self.__test_qft(100, 2500)

    def __test_qft(self, n_qubits, max_qubits):
        qft_seq = qft(n_qubits)
        ops = OperationSequence(len(qft_seq))
        for opcode, args in qft_seq:
            ops.append(opcode, *args)

        widget_seq = WidgetSequence(n_qubits, max_qubits)    
        widget_seq(ops, store_output=False, progress=False) 

# Parameterised toffoli gate
def cphase(ctrl, targ, i):
    return [
        (gates.RZ, (ctrl, i)),  
        (gates.RZ, (targ, i)),
        (gates.CNOT, (ctrl, targ)),
        (gates.RZ, (targ, i)),
        (gates.CNOT, (ctrl, targ))
    ]

def qft_round(i, n_qubits):
    ops = [(gates.H, (i, ))]
    for i in range(i, n_qubits - 1): 
        ops += cphase(n_qubits - 1, i, 1)
    return ops 

def qft(n_qubits):
    ops = []
    for i in range(n_qubits):
       ops += qft_round(i, n_qubits) 
    return ops 

if __name__ == '__main__':
    unittest.main()
