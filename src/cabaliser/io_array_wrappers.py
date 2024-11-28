'''
    Wrapper objects around QubitArray
    Typing is only for verbosity and any potential
    extensibility that might be required
'''

from ctypes import POINTER

from ctypes import c_void_p
from cabaliser.qubit_array import QubitArray
from cabaliser.structs import ScheduleDependencyType, PauliCorrectionType, InvMapperType
from cabaliser.gates import SINGLE_QUBIT_GATE_TABLE
from cabaliser.gate_constructors import tag_to_angle
from cabaliser.utils import deref

from cabaliser.lib_cabaliser import lib
lib.lib_pauli_mapper_to_const_vec.restype = POINTER(InvMapperType)


class MeasurementTags(QubitArray):
    '''
        Ordered array of RZ measurement tags
    '''
    def to_list(self, cache=True, to_float=False):
        if to_float:
            return list(map(tag_to_angle, self))
        return super().to_list(cache=cache)


class LocalCliffords(QubitArray):
    '''
        Ordered array of local clifford operations
    '''
    def to_list(self, cache=True, to_string=True):
        if to_string:
            return list(map(SINGLE_QUBIT_GATE_TABLE.__getitem__, self))
        return super().to_list(cache=cache)


class IOMap(QubitArray):
    '''
        Ordered array of map of input qubits to output qubits
    '''


class ScheduleDependency(QubitArray):
    '''
        ScheduleDependency
        Wrapper for the rustlib pauli tracker schedule
    '''
    def __init__(
            self,
            n_qubits: int,
            qubit_index: int,
            arr: c_void_p,
            node_ptr: [c_void_p, None]
    ):
        self.qubit_index = qubit_index
        self._ptr = node_ptr
        super().__init__(n_qubits, arr)

    @staticmethod
    def from_struct(struct: ScheduleDependencyType):
        return ScheduleDependency(
            struct.len,
            struct.dependent,
            struct.arr,
            None
        )

    def to_dict(self):
        '''
            Returns a dictionary object of the dependencies
        '''
        return {self.qubit_index: self.to_list()}

    def __repr__(self):
        return f"({self.qubit_index}: {list(iter(self))})"

    def __str__(self):
        return self.__repr__()

    def __del__(self):
        # TODO: This is probably leaking about 16 bytes of memory on python cleanup
        # lib.lib_pauli_tracker_const_vec_destroy(self._ptr)
        pass


class InvMapper(QubitArray):
    '''
        Map from correction indicies to qubits
    '''
    def __init__(self, mapper: c_void_p):
        self._vec_ptr = mapper
        self._const_vec_ptr = lib.lib_pauli_mapper_to_const_vec(self._vec_ptr)
        super().__init__(deref(self._const_vec_ptr).len, deref(self._const_vec_ptr).arr)

    def __del__(self):
        lib.lib_pauli_tracker_destroy_inv_map(
            self._vec_ptr,
            self._const_vec_ptr
        )


class PauliCorrection(QubitArray):
    '''
        PauliCorrection
        Adapter wrapper for rust Vec object
    '''
    pauli_to_str = {
        0: 'I',
        1: 'Z',
        2: 'X',
        3: 'Y'
    }

    def __init__(self, index, correction: POINTER(PauliCorrectionType), n_qubits):
        self._ptr = correction
        self.cap = deref(correction).cap
        self._len = deref(correction).len
        self.index = index

        self.__list = None

        super().__init__(n_qubits, deref(correction).arr)

    def to_list(self, cache: bool = True):
        '''
            Converts the array to a Python list
            :: cache : bool :: Whether to cache the list
        '''
        if cache and self.__list is None:
            self.__list = list(map(
                self.pauli_to_str.get,
                super().to_list(cache=cache)
            ))
        elif not cache:
            return list(map(
                self.pauli_to_str.get,
                super().to_list()
            ))
        return self.__list

    def to_dict(self, cache=True):
        '''
            Converts the array to a Python dict
            :: cache : bool :: Whether to cache the underlying list
        '''

        return {self.index: self.to_list(cache=cache)}

    def to_tuple(self, cache=True):
        '''
            Converts the array to a Python tuple
            :: cache : bool :: Whether to cache the underlying list
        '''
        return (self.index, self.to_list(cache=cache))

    def __repr__(self):
        return self.to_dict().__repr__()

    def __str__(self):
        return self.__repr__()

    def __del__(self):
        lib.lib_pauli_tracker_destroy_corrections(
            self._ptr
        )
