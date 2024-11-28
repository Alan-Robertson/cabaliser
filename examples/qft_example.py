'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import sys
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
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


def main(n_qubits=100, max_qubits=16000):
    start_time = time.time()

    # Create operation sequence
    qft_seq = qft(n_qubits)
    ops = OperationSequence(len(qft_seq))
    for opcode, args in qft_seq:
        ops.append(opcode, *args)


    seq_time = time.time()    
    # Create widget, do not teleport input
    wid = Widget(n_qubits, max_qubits)#, teleport_input=False)

    # Apply operation on widget
    wid(ops)

    widget_io_time = time.time()
    ## Decompose widget
    wid.decompose()
    
    decomposition_time = time.time()

    json = wid.json()

    json_time = time.time()

    print(f"Python operation creation time: {seq_time - start_time:.4f}")
    print(f"Widget Input Time: {widget_io_time - seq_time:.4f}")
    print(f"Widget Decomposition Time: {decomposition_time - widget_io_time:.4f}")
    print(f"Json Creation Time: {json_time - decomposition_time:.4f}")

    return json 

if __name__ == '__main__':
    n_qubits = 100 
    max_qubits = 16000
    if len(sys.argv) > 2:
        n_qubits = int(sys.argv[1])
        max_qubits = int(sys.argv[2])
    main(n_qubits, max_qubits)
