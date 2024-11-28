'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser import local_simulator

# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

table = {
_I_: local_simulator.I,
_T_: local_simulator.T,
_Tdag_: local_simulator.Tdag
}

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
json = wid.json()

input_state = local_simulator.kr(
    local_simulator.plus_state,
    local_simulator.plus_state
)

widget_state = local_simulator.simulate_widget(
    wid,
    input_state=input_state,
    table=table)

json_widget_state = local_simulator.simulate_dict_as_widget(
    json,
    input_state=input_state,
    table=table)

print("Simulated state from circuit")
ctrl_a = 0
ctrl_b = 1
local_simulator.vec(
    local_simulator.kr(
        local_simulator.I, 
        local_simulator.H)
    @ local_simulator.CNOT(n_qubits, ctrl_b, ctrl_a)
    @ local_simulator.kr(
        local_simulator.T,
        local_simulator.Tdag)
    @ local_simulator.CNOT(n_qubits, ctrl_b, ctrl_a)
    @ local_simulator.kr(
        local_simulator.I,
        local_simulator.Tdag)
    @ input_state
)

print("Simulated state from widget")
local_simulator.vec(widget_state)

print("Simulated state from json")
local_simulator.vec(json_widget_state)
