import gc
'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import sys
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget_sequence import WidgetSequence
import time
# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

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


def main(n_qubits=50, max_qubits=1000, store_output=False):

    # Create operation sequence
    print("Generating QFT Circuit")
    qft_seq = qft(n_qubits)

    print("Constructing Operation Sequence")
    ops = OperationSequence(len(qft_seq))
    for opcode, args in qft_seq:
        ops.append(opcode, *args)

    del qft_seq
    print("Constructing Widget Sequence")

    widget_seq = WidgetSequence(n_qubits, max_qubits)    
    print("Executing...")
    widget_seq(ops, store_output=store_output)
    if store_output:
        return widget_seq.json()
    
    del widget_seq 
    del ops
    gc.collect()

if __name__ == '__main__':
    n_qubits = 250 
    max_qubits = 2000
    if len(sys.argv) > 2:
        n_qubits = int(sys.argv[1])
        max_qubits = int(sys.argv[2])
    ops = main(n_qubits, max_qubits)
