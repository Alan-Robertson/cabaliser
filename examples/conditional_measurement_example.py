'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget

# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

n_qubits = 2
max_qubits = 20

# Parameterised gate
def gate(ctrl_a, ctrl_b):
    return [
    (gates.H, (ctrl_b,)),
    (gates.MCX, (0, 1))
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
print(wid.json())
