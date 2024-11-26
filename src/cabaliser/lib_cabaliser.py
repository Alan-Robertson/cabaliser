'''
    Wrapper around the C lib load
'''

import os
from ctypes import cdll

lib_path = os.path.dirname(__file__)
lib = cdll.LoadLibrary(f'{lib_path}/lib_cabaliser_bin.so')
