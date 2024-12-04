'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import unittest

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser.exceptions import WidgetNotDecomposedException

if __name__ != '__main__':
    def print(*args, **kwargs):
        pass

# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

class ToffoliTest(unittest.TestCase):

    # Parameterised toffoli gate
    @staticmethod
    def toffoli_gate(ctrl_a, ctrl_b, targ):
        return [
        (gates.H, (targ,)),
        (gates.CNOT, (ctrl_b, targ)),
        (gates.RZ, (targ, _Tdag_)),
        (gates.CNOT, (ctrl_a, targ)),
        (gates.RZ, (targ, _T_)),
        (gates.CNOT, (ctrl_b, targ)),
        (gates.RZ, (targ, _Tdag_)),
        (gates.CNOT, (ctrl_b, targ)),
        (gates.RZ, (targ, _T_)),
        (gates.CNOT, (ctrl_a, ctrl_b)),
        (gates.RZ, (ctrl_a, _T_)),
        (gates.RZ, (ctrl_b, _Tdag_)),
        (gates.CNOT, (ctrl_a, ctrl_b)),
        (gates.H, (targ,))
    ]

    @staticmethod
    def toffoli_ops(ctrl_a, ctrl_b, targ):
        toffoli = ToffoliTest.toffoli_gate(ctrl_a, ctrl_b, targ)
        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)
        return ops


    def test_schedule_exception(self, n_qubits=3, max_qubits=64):
        '''
            Test that schedules can't be obtained prior to decomposition
        '''

        wid = Widget(n_qubits, max_qubits)
        ops = self.toffoli_ops(0, 1, 2) 
        for op in ops:
            print(op)

        # Apply operation on widget
        wid(ops)
       
        try:
            wid.get_schedule()
            assert False
        except WidgetNotDecomposedException: 
            pass
     
        # Decompose widget
        wid.decompose()

    def test_graph_callthrough(self):
        wid = Widget(5, 11)
        wid.decompose()

        for i, layer in enumerate(wid.pauli_tracker):
            for dep in layer:
                if i == 0:
                    sum(len(dep[i]) for i in dep) == 0
            
if __name__ == '__main__':
    unittest.main()
