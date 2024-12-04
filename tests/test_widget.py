'''
    Tests basic widget logic
'''
import unittest

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser.gate_constructors import RZ_angle, tag_to_angle


class WidgetTest(unittest.TestCase):

    def test_toffoli(self):
        # Rz tags
        _I_ = 0
        _T_ = 1
        _Tdag_ = 2

        n_qubits = 3
        max_qubits = 20


        # Parameterised toffoli gate
        def toffoli_gate(ctrl_a, ctrl_b, targ):
            return [
                (gates.RZ, (ctrl_a, _T_)),
                (gates.RZ, (ctrl_b, _T_)),
                (gates.H, (targ,)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _T_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.RZ, (ctrl_b, _Tdag_)),
                (gates.RZ, (targ, _T_)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _Tdag_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _Tdag_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.H, (targ,))
            ]
            

        # Create operation sequence
        toffoli = toffoli_gate(0, 1, 2)

        # Create widget, do not teleport input
        wid = Widget(n_qubits, max_qubits)#, teleport_input=False)

        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()
        wid.json()

    def test_two_qubit(self):
        # Rz tags
        _I_ = 0
        _T_ = 1
        _Tdag_ = 2

        n_qubits = 2
        max_qubits = 20

        # Parameterised gate
        def gate(ctrl_a, ctrl_b):
            return [
            (gates.RZ, (ctrl_b, _Tdag_)),
            (gates.CNOT, (ctrl_b, ctrl_a)),
            (gates.RZ, (ctrl_a, _T_)),
            (gates.RZ, (ctrl_b, _Tdag_)),
            (gates.CNOT, (ctrl_b, ctrl_a)),
            (gates.H, (ctrl_b,))
        ]

        # Create operation sequence
        operation = gate(0, 1)
        ops = OperationSequence(len(operation))
        for opcode, args in operation:
            ops.append(opcode, *args)

        # Create widget, do not teleport input
        wid = Widget(n_qubits, max_qubits)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()
        wid.json()

    def test_tagging(self):
        # Rz tags
        _I_ = 0
        _T_ = 1
        _Tdag_ = 2

        n_qubits = 2
        max_qubits = 40

        # Parameterised gate
        def gate(ctrl_a, ctrl_b, angle_a, angle_b, angle_c):
            return [
            (RZ_angle(ctrl_a, angle_a)),
            (gates.CNOT, (ctrl_b, ctrl_a)),
            (RZ_angle(ctrl_a, angle_b)),
            (RZ_angle(ctrl_b, angle_c)),
            (gates.CNOT, (ctrl_b, ctrl_a)),
            (gates.H, (ctrl_b,))
        ]

        # Create operation sequence
        operation = gate(0, 1, 0.1, 0.2, 0.3)
        ops = OperationSequence(len(operation))
        for opcode, args in operation:
            ops.append(opcode, *args)

        # Create widget, do not teleport input
        wid = Widget(n_qubits, max_qubits)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()

        # Map the tags back to angles
        wid.json(rz_to_float=True)

    def test_local_toffoli(self):
        # Rz tags
        _I_ = 0
        _T_ = 1
        _Tdag_ = 2

        n_qubits = 3
        max_qubits = 20


        # Parameterised toffoli gate
        def toffoli_gate(ctrl_a, ctrl_b, targ):
            return [
                (gates.RZ, (ctrl_a, _T_)),
                (gates.RZ, (ctrl_b, _T_)),
                (gates.H, (targ,)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _T_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.RZ, (ctrl_b, _Tdag_)),
                (gates.RZ, (targ, _T_)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _Tdag_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.CNOT, (ctrl_a, ctrl_b)),
                (gates.RZ, (targ, _Tdag_)),
                (gates.CNOT, (ctrl_b, targ)),
                (gates.H, (targ,))
            ]
            

        # Create operation sequence
        toffoli = toffoli_gate(0, 1, 2)

        # Create widget, do not teleport input
        wid = Widget(n_qubits, max_qubits)#, teleport_input=False)

        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()
        wid.json()

if __name__ == '__main__':
    unittest.main()

