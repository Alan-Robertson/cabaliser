'''
    Corresponding structs and Python Classes
'''
from itertools import chain, repeat

from ctypes import Structure, Union, c_uint32, c_uint8
from cabaliser.gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES, LOCAL_CLIFFORD_MASK, NON_LOCAL_CLIFFORD_MASK, RZ_MASK, OPCODE_TYPE_MASK, RZ_GATES, CONDITIONAL_OPERATION_GATES
from cabaliser.utils import unbound_table_element

OpcodeType = c_uint8

class SingleQubitOperationType(Structure): 
    '''
        ctypes wrapper for single qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('arg', c_uint32),
        ]

    def __repr__(self):
        return f"Op: {self.arg}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return int(self.arg)


def SingleQubitOperation(arr, idx, opcode, arg):
    '''
       Constructor for single qubit operations
        :: arr : array :: Array to write to
        :: idx : int :: Index to write to
        :: opcode : uint8 :: Opcode for single qubit operation
        :: arg : uint32 :: Target qubit
        Writes the operation to the index of the array
    '''
    arr[idx].single.opcode = opcode
    arr[idx].single.arg = arg

SINGLE_QUBIT_OPERATION_HEADER = (1 << 6) 
class TwoQubitOperationType(Structure):
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('ctrl', c_uint32),
        ('targ', c_uint32),
        ]

    def __repr__(self):
        return f"Op: {self.ctrl} {self.targ}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return max(int(self.arg), int(self.targ))


def TwoQubitOperation(arr, idx: int, opcode: OpcodeType, ctrl: int, targ: int):
    '''
       Constructor for two qubit operations
        :: arr : array :: Array to write to
        :: idx : int :: Index to write to
        :: opcode : uint8 :: Opcode for single qubit operation
        :: ctrl : uint32 :: First qubit argument
        :: targ : uint32 :: Second qubit argument
        Writes the operation to the index of the array
    '''
    arr[idx].two_qubits.opcode = opcode
    arr[idx].two_qubits.ctrl = ctrl
    arr[idx].two_qubits.targ = targ


class RzQubitOperationType(Structure):
    '''
        ctypes wrapper for rz operations
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('arg', c_uint32),
        ('tag', c_uint32),
        ]

    def __repr__(self):
        return f"Rz({self.arg}) : {self.tag}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return int(self.arg)


def RzOperation(arr, i, opcode, arg, tag):
    '''
       Constructor for two qubit operations
        :: arr : array :: Array to write to
        :: idx : int :: Index to write to
        :: opcode : uint8 :: RZ opcode
        :: arg : uint32 :: Qubit argument
        :: tag : uint32 :: Tag for the rz angle
        Writes the operation to the index of the array
    '''
    arr[i].rz.opcode = opcode
    arr[i].rz.arg = arg
    arr[i].rz.tag = tag



# This is identical to the two qubit operation 
# But we provide a different class for clarity
class ConditionalOperationType(Structure):
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('ctrl', c_uint32),
        ('targ', c_uint32),
        ]

    def __repr__(self):
        return f"Cond Op: {self.ctrl} {self.targ}"

    def __str__(self):
        return self.__repr__()


def ConditionalOperation(arr, idx, opcode, ctrl, targ):
    '''
    Constructor for conditional operations
    :: arr : array :: Array to write to
    :: idx : int :: Index
    :: opcode : uint8 :: Opcode
    :: ctrl : uin32_t :: Control Qubit 
    :: targ : uin32_t :: Target Qubit
    '''
    arr[idx].two_qubits.opcode = opcode
    arr[idx].two_qubits.ctrl = ctrl
    arr[idx].two_qubits.targ = targ

# Lookup table for Operation types to underlying union members 
opcode_map = [unbound_table_element for _ in range(256)]
for idx, fn in chain(
    zip(SINGLE_QUBIT_GATES, repeat(lambda x: x.single)),
    zip(TWO_QUBIT_GATES, repeat(lambda x: x.two_qubits)),
    zip(RZ_GATES, repeat(lambda x: x.rz)),
    zip(CONDITIONAL_OPERATION_GATES, repeat(lambda x: x.cond_op))
    ):
    opcode_map[idx] = fn 


class OperationType(Union):
    '''
        ctypes wrapper for general operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('single', SingleQubitOperationType),
        ('two_qubits', TwoQubitOperationType),
        ('rz', RzQubitOperationType),
        ('cond_op', ConditionalOperationType), 
        ]

    def __repr__(self):
        return opcode_map[self.opcode](self).__repr__()

    def is_rz(self):
        return RZ_MASK == (self.opcode & OPCODE_TYPE_MASK)  
