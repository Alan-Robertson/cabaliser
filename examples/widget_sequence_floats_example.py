'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import sys
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.gate_constructors import RZ_angle
from cabaliser.widget_sequence import WidgetSequence
import time
# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

# Parameterised toffoli gate
def cphase(ctrl, targ, angle):
    return [
        (RZ_angle(ctrl, angle)),  
        (RZ_angle(targ, angle)),
        (gates.CNOT, (ctrl, targ)),
        (RZ_angle(targ, angle)),
        (gates.CNOT, (ctrl, targ))
    ]

def qft_round(i, n_qubits):
    ops = [(gates.H, (i, ))]
    for rot, i in enumerate(range(i, n_qubits - 1)): 
        ops += cphase(n_qubits - 1, i, 2 ** (-1 * (2 + rot)))
    return ops 

def qft(n_qubits):
    ops = []
    for i in range(n_qubits):
       ops += qft_round(i, n_qubits) 
    return ops 

def main(n_qubits=50, max_qubits=1000, store_output=True):

    # Create operation sequence
    qft_seq = qft(n_qubits)
    ops = OperationSequence(len(qft_seq))
    for opcode, args in qft_seq:
        ops.append(opcode, *args)

    widget_seq = WidgetSequence(n_qubits, max_qubits)    
    
    # As the sequencer discards widget objects, the float tracking moves to the sequence construction
    widget_seq(ops, store_output=store_output, rz_to_float=True)
    if store_output:
        return widget_seq.json()

if __name__ == '__main__':
    n_qubits = 100 
    max_qubits = 2000
    if len(sys.argv) > 2:
        n_qubits = int(sys.argv[1])
        max_qubits = int(sys.argv[2])
    json = main(n_qubits, max_qubits)
    print(json)
