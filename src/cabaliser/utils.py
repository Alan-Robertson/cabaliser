'''
    Misc useful utilities
'''
from ctypes import c_void_p

INF = float('inf')


class void_p(c_void_p):
    '''
        Wrapper for the c_void_p object
        Not subclassing this results in Python converting the c_void_p to an int32,
        which breaks passing opaque pointers back to the library
        https://docs.python.org/3/library/ctypes.html#ctypes._SimpleCData
    '''
    def __format__(self, *args, **kwargs):
        return super().__format__(*args, **kwargs)

    def __repr__(self, *args, **kwargs):
        return super().__repr__(*args, **kwargs)


def deref(obj: c_void_p) -> object:
    '''
        Verbose dereference for pointer objects
    '''
    return obj[0]
