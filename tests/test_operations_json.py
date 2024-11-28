import unittest
import numpy as np

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser import local_simulator 

EPS = 1e-7
N_REPETITIONS = 100
class OperationJSONTest(unittest.TestCase):

    def test_identity(self): 
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 1
        max_qubits = 10
        n_inputs = 1
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.I, (0,))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        json = wid.json()
        # One input, one output
        assert json['n_qubits'] == 2

        # No local cliffords
        assert json['local_cliffords'][0] == 'I'
        assert json['local_cliffords'][1] == 'I'


        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state, trace_graph=False)
       
            zero_state = local_simulator.zero_state 
            effective_state = input_state
            embedded_state = local_simulator.kr(*([zero_state] * 2 * n_inputs), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              



    def test_hadamard(self): 
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 1
        n_inputs = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.H, (0,))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        json = wid.json()
        # One input, one output
        assert json['n_qubits'] == 2

        # Hadamard on the output
        assert json['local_cliffords'][1] == 'H'

        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state, trace_graph=False)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.H @ input_state
            )
            embedded_state = local_simulator.kr(*([zero_state] * 2 * n_inputs), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              

    def test_cnot(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 2
        n_inputs = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.CNOT, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        zero_state = local_simulator.zero_state 

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.kr(
                local_simulator.state_prep(*list(np.random.random(2))),
                zero_state
            )
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            effective_state = (
                local_simulator.CNOT(n_qubits, 0, 1)
                @ input_state 
            )
            embedded_state = local_simulator.kr(*([zero_state] * n_inputs * 2), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)

            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()


    def test_cz(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''
        n_qubits = 2
        n_inputs = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.CZ, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        zero_state = local_simulator.zero_state 

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.kr(
                local_simulator.state_prep(*list(np.random.random(2))),
                zero_state
            )
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the CZ of the input state on qubits 2 and 3 
            effective_state = (
                local_simulator.CZ(n_qubits, 0, 1)
                @ input_state 
            )
            embedded_state = local_simulator.kr(*([zero_state] * n_inputs * 2), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)

            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()
 
    def test_phase(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 1
        n_inputs = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.S, [0])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.S @ input_state
            )
            embedded_state = local_simulator.kr(*([zero_state] * 2 * n_inputs), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              
   

    def test_phase_dag(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 1
        n_inputs = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.Sd, [0])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.Sdag @ input_state
            )
            embedded_state = local_simulator.kr(*([zero_state] * 2 * n_inputs), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              
   
    def test_rz_no_rotation(self):  
        '''
            RZ gate triggering a teleport
        '''

        n_qubits = 1
        n_inputs = 1
        n_anc = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
        I = 0
    
        operations = [(gates.RZ, [0, I])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                 input_state
            )
            embedded_state = local_simulator.kr(*([zero_state] * (2 * n_inputs + n_anc)), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              


    def test_rz(self):  
        '''
            RZ gate triggering a teleport
        '''
        n_qubits = 1
        n_inputs = 1
        n_anc = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
        T = 1
    
        operations = [(gates.RZ, [0, T])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=False)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                 local_simulator.T @ input_state
            )
            embedded_state = local_simulator.kr(*([zero_state] * (2 * n_inputs + n_anc)), effective_state) 
            traced_state = local_simulator.trace_out_graph(wid, widget_state)
                                                                                              
            assert (np.abs(embedded_state - widget_state) < EPS).all()
            assert (np.abs(traced_state - effective_state) < EPS).all()                              


    def test_trace_graph(self):  
        '''
            RZ gate triggering a teleport
        '''

        n_qubits = 1
        n_inputs = 1
        n_anc = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
        T = 1
    
        operations = [(gates.RZ, [0, T])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()
        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_dict_as_widget(json, input_state=input_state, trace_graph=True)
       
            # Output should be the Hadamard of the input state on qubit 2

            effective_state = (
                 local_simulator.T @ input_state
            )
                                                                                              
            assert (np.abs(effective_state - widget_state) < EPS).all()

    def test_feed_forward(self):  
        '''
            RZ gate triggering a teleport
        '''
        n_qubits = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.H, (0,))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        json = wid.json()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(
                *list(np.random.random(2))
            )

            widget_state = local_simulator.simulate_dict_as_widget(
                 json,
                 input_state=input_state,
                 trace_graph=True
            )
       
            effective_state = (
                 local_simulator.H @ input_state
            )
                                                                                              
            assert (np.abs(effective_state - widget_state) < EPS).all()

            # Apply the widget again 
            widget_state = local_simulator.simulate_dict_as_widget(
                 json,
                 input_state=widget_state,
                 trace_graph=True
            )
            assert (np.abs(input_state - widget_state) < EPS).all()


if __name__ == '__main__':
    unittest.main()
