'''
    Wrapper object for Ctype arrays
    Exposes some simple python interfaces
'''

from ctypes import Structure, c_void_p


class QubitArray:
    '''
        QubitArray
        Python wrapper for Ctype arrays
    '''
    def __init__(self, n_qubits: int, arr: c_void_p):
        '''
            Initialiser
            :: n_qubits : int :: Number of elements in the array
            :: arr : POINTER(<T>) :: Pointer object to the array
        '''
        self.n_qubits = n_qubits
        self.arr = arr

        self.__list = None

    @staticmethod
    def from_struct(struct: Structure, n_qubits='len', arr='arr'):
        '''
            Constructor from struct objects
        '''
        n_qubits = getattr(struct, n_qubits)
        arr = getattr(struct, arr)
        return QubitArray(n_qubits, arr)

    def __iter__(self):
        '''
            Generator style __iter__
            Yields elements of the array in order
        '''
        for i in range(self.n_qubits):
            yield self.arr[i]

    def __getitem__(self, idx: int):
        '''
            Returns an item from the array
            :: idx : int :: Index to query
        '''
        if idx > self.n_qubits:
            raise IndexError("Qubit index is out of range")
        return self.arr[idx]

    def __setitem__(self, idx: int, val):
        raise TypeError("Cannot write to array object")

    def __len__(self):
        '''
            Returns the length of the array
        '''
        return self.n_qubits

    def _set_list(self, lst): 
        '''
            Helper function for setting the list
            Should only be called by the appropriate child classes 
        '''
        self.__list = lst

    def _get_list(self):
        '''
            Helper function to get the internal list object
        '''
        return self.__list

    def to_list(self, cache=True):
        '''
            Converts the underlying array to a Python list
        '''
        if cache and self.__list is None:
            self.__list = list(iter(self))
        elif not cache:
            return list(iter(self))
        return self.__list
