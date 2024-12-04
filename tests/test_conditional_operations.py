import unittest
import numpy as np

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser.gate_constructors import measure


class ConditionalOperationTest(unittest.TestCase):

    def test_measure_qubit(self):
        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [measure(0)] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # No correction terms on this qubit 
        assert len(wid.get_pauli_corrections()) == 2 

        # Qubit is measured 
        assert wid.get_io_map(0) is None 

    def test_mcx_call(self):
        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.MCX, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        assert wid.get_pauli_corrections(2) == 'IIIX'
        assert wid.get_io_map(0) is None 


    def test_mcz_call(self):
        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.MCZ, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        assert wid.get_pauli_corrections(2) == 'IIIZ'
        assert wid.get_io_map(0) is None

    def test_mcy_call(self):
        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.MCY, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        wid.get_pauli_corrections(2) == 'IIIY'
        assert wid.get_io_map(0) is None


if __name__ == '__main__':
    unittest.main()
 
