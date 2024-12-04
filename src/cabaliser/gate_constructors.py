'''
    Helper constructors for gates
'''
from ctypes import c_int32, c_float, POINTER

from cabaliser.gates import RZ, MEAS


def measure(targ: int) -> tuple:
    '''
        Measure a target qubit
    '''
    return (MEAS, (targ, targ))

def measure_seq(*targs) -> list:
    '''
        Independently measures a set of qubits
    '''
    return list(map(measure, targs))

def RZ_angle(targ: int, angle: float) -> tuple:
    '''
        Constructor for RZ gate that incorporates angle to tag casting
    '''
    tag = angle_to_tag(angle)

    return (RZ, (targ, tag))


def hard_cast(type_from: type, type_to: type, val: object):
    '''
        Casts between two types
    '''
    val = type_from(val)
    ptr = POINTER(POINTER(type_to))(POINTER(type_from)(val))
    return ptr[0][0]


def angle_to_tag(angle: float, eps: float = 1e-12) -> int:
    '''
        Helper constructors for gates
    '''

    if abs(angle) < eps:
        return 0

    # Truncate to 32 bits of precision
    tag = hard_cast(c_float, c_int32, angle)
    return int(tag)


def tag_to_angle(tag: int) -> float:
    '''
        Casts a tag back to an angle
    '''
    if tag == 0:
        return 0
    angle = hard_cast(c_int32, c_float, tag)
    return float(angle)
