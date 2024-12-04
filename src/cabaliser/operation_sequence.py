'''
    Operation Sequence
    Wrapper for sequences of operations
    Exposes these sequences to the C api
'''
from itertools import chain, repeat

from cabaliser.gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES, RZ_GATES, CONDITIONAL_OPERATION_GATES, RZ 
from cabaliser.operations import (
    OperationType, SingleQubitOperation,
    TwoQubitOperation, RzOperation,
    ConditionalOperation)

from cabaliser.utils import unbound_table_element


class OperationSequence():
    '''
        Operation sequence object
        This is a wrapper for an array of ctypes
         operations
        :: n_instructions: int :: Maximum number of
         instructions for this sequence
        This object has a pre-allocated maximum number
         of supported operations
    '''
    CONSTRUCTOR_MAP = [unbound_table_element for _ in range(256)]
    for idx, fn in chain(
        zip(SINGLE_QUBIT_GATES, repeat(SingleQubitOperation)),
        zip(TWO_QUBIT_GATES, repeat(TwoQubitOperation)),
        zip(RZ_GATES, repeat(RzOperation)),
        zip(CONDITIONAL_OPERATION_GATES, repeat(ConditionalOperation))
        ):
        CONSTRUCTOR_MAP[idx] = fn 

    def __init__(self, n_instructions: int):
        '''
        Constructor for the operation sequence object
        :: n_instructions: int :: Maximum number of instructions for this sequence
        '''
        self.n_instructions = n_instructions
        self.ops = (OperationType * n_instructions)()
        self.curr_instructions = 0

        self.max_qubit_index = 0
        self.n_rz_operations = 0

    def __getitem__(self, idx: int):
        '''
            __getitem__
            Gets an operation from the array
            :: idx: int :: index to query
            Warning: negative integers will return the value from
            negative addresses, they will NOT index from the end
            of the array
        '''
        return self.ops[idx]

    def __setitem__(self, idx, value: OperationType):
        '''
            __setitem__
            Sets an operation from the array
            :: idx: int :: index to query
            :: value: OperationType :: The operation to write to that element
            Warning: negative integers will write to addresses
            below the address of the array,
            they will NOT index from the end of the array
        '''
        self.ops[idx] = value
        #self.max_qubit_index = max(self.max_qubit_index, value.max_qubit_index)

    def __iter__(self):
        '''
            __iter__
            Iterator for the OperationSequence object
            Iterates over the set of currently set operations
        '''
        for i in range(self.curr_instructions):
            yield self.ops[i]

    def append(self, opcode, *args):
        '''
            append
            :: opcode :: Operation code to append
            :: *args :: Arguments to pass to the instruction
            Calls an internal constructor table to build the appropriate operation
        '''
        if self.curr_instructions == self.n_instructions:
            raise IndexError("Exceeded Length of Array")

        # This function acts to append the opcode to the ops array
        self.CONSTRUCTOR_MAP[opcode](self.ops, self.curr_instructions, opcode, *args)

        # Update sequence params
        if opcode == RZ:
            self.sequence_params_rz(*args)
        else:
            self.sequence_params(*args)
        self.curr_instructions += 1

    def _append(self, *operations):
        if self.curr_instructions + len(operations) > self.n_instructions:
            raise IndexError("Exceeded Length of Array")

        for operation in operations:
            self[self.curr_instructions] = operation
            self.curr_instructions += 1

    def __add__(self, other):
        '''
            __add__
            Helper function to make these arrays more list-like
            :: other : OperationSequence :: Another operation sequence
            Returns a new operation sequence containing lhs operations then rhs operations
        '''
        seq = OperationSequence(self.n_instructions + other.n_instructions)
        # This could probably be done with a wrap around memcpy
        for op in self:
            seq._append(op)
        for op in other:
            seq._append(op)

        seq.max_qubit_index = max(self.max_qubit_index, other.max_qubit_index)
        seq.n_rz_operations = self.n_rz_operations + other.n_rz_operations
        return seq

    def __repr__(self):
        print(self.ops[0])
        return '\n'.join(repr(self.ops[i]) for i in range(self.curr_instructions))

    def __str__(self):
        return self.__repr__()

    def sequence_params_rz(self, *params):
        '''
            Updates parameters for rz operations
            This corresponds to one extra qubit
        '''
        self.n_rz_operations += 1
        self.sequence_params(*params)

    def sequence_params(self, *params):
        '''
            Updates the maximum qubit index in use
        '''
        self.max_qubit_index = max(self.max_qubit_index, *params)

    def split(self, rz_threshold):     
        if self.n_rz_operations < rz_threshold:
            return [self,]

        sequences = list()

        rz_instructions = 0
        start_ops = 0
        for i in range(self.n_instructions): 
            if self.ops[i].is_rz():
                if rz_instructions < rz_threshold:
                    rz_instructions += 1
                else:
                    sequences.append(self._subsequence(start_ops, i)) 
                    rz_instructions = 1
                    start_ops = i

        # Final Sequence
        sequences.append(self._subsequence(start_ops, i)) 
        return sequences
       
    def _subsequence(self, start, end): 
        '''
            Copies a subsequence from self to a new sequence
        '''
        sequence_length = end - start
        seq = OperationSequence(sequence_length)  
        seq.curr_instructions = sequence_length
        seq.n_instructions = sequence_length
        seq.max_qubits_index = self.max_qubit_index
        for j in range(sequence_length):
            if self[start + j].is_rz(): 
                seq.n_rz_operations += 1
            seq[j] = self[start + j]
        return seq
