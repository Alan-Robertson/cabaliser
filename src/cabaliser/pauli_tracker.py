'''
    Pauli Tracker Wrapper
'''

from ctypes import POINTER, c_size_t, c_uint32, c_void_p
from types import GeneratorType

from cabaliser.utils import deref, INF, void_p

from cabaliser.structs import ScheduleDependencyType, PauliCorrectionType
from cabaliser.io_array_wrappers import ScheduleDependency, PauliCorrection, InvMapper

from cabaliser.lib_cabaliser import lib
lib.lib_pauli_n_layers.restype = c_size_t
lib.lib_pauli_n_dependents.restype = c_size_t

lib.lib_pauli_graph_to_layer.restype = void_p

lib.lib_pauli_tracker_partial_order_graph.restype = void_p  # Opaque Pointer
lib.lib_pauli_layer_to_dependent_node.restype = POINTER(ScheduleDependencyType)

lib.lib_pauli_tracker_create_pauli_corrections.restype = void_p  # Opaque Pointer
lib.lib_pauli_tracker_get_pauli_corrections.restype = POINTER(PauliCorrectionType)
lib.lib_pauli_tracker_get_correction_table_len.restype = c_size_t

lib.lib_pauli_tracker_get_inv_mapper.restype = void_p  # Opaque Pointer


class PauliTracker:
    '''
        PauliTracker
        Wrapper for the rustlib pauli tracker object
    '''
    def __init__(self, widget):
        '''
            PauliTracker
            Wrapper for the rustlib pauli tracker object
        '''
        self.pauli_tracker_ptr = widget.pauli_tracker_ptr
        self.corrections_ptr = None
        self.inv_mapper = None

        self.graph_ptr = None
        self.__n_layers = None
        self.measurement_schedule = None
        self.corrections = None
        self.max_qubit = INF  # Truncates unused qubits

    def __call__(self):
        self.schedule()

    def to_list(self):
        '''
            Copies the array to a Python list
        '''
        return list(iter(self))

    def schedule(self, max_qubit=INF):
        '''
            Sets up the measurement schedule
        '''
        self.max_qubit = max_qubit
        self.graph_ptr = self.widget_to_graph()

    def widget_to_graph(self) -> c_void_p:
        '''
            widget_to_graph
            Gets a graph pointer from a widget pointer
        '''
        graph_ptr = lib.lib_pauli_tracker_partial_order_graph(
            self.pauli_tracker_ptr
            )
        return graph_ptr

    @property
    def n_layers(self) -> int:
        '''
            graph_get_n_layers
            Gets the number of layer in the schedule graph
        '''
        if self.__n_layers is None:
            self.__n_layers = int(lib.lib_pauli_n_layers(self.graph_ptr))
        return self.__n_layers

    def get_layer(self, index: int) -> c_void_p:
        '''
            graph_get_layer
            Gets a layer pointer from the graph
            :: index : int :: Layer index
            Returns an opaque pointer to the layer object
        '''
        return lib.lib_pauli_graph_to_layer(self.graph_ptr, c_uint32(index))

    def layer_get_n_dependents(self, layer_ptr: c_void_p) -> int:
        '''
            layer_get_n_dependents
            Gets the number of dependent nodes in a layer
            :: layer_ptr : c_void_p :: Opaque pointer to layer object
            Returns the number of elements in that layer
        '''
        return int(lib.lib_pauli_n_dependents(layer_ptr))

    def layer_get_dependent(self, layer_ptr: c_void_p, index: int) -> ScheduleDependency:
        '''
            layer_get_dependent
            Gets a dependent node from a layer
            :: layer_ptr : ctypes.c_void_p ::
            :: index : int :: Index of dependent
            Returns a wrapped qubit array object
        '''
        qubit_index = int(lib.lib_pauli_dependent_qubit_idx(layer_ptr, index))

        node_ptr = lib.lib_pauli_layer_to_dependent_node(layer_ptr, c_uint32(index))

        node = deref(node_ptr)

        return ScheduleDependency(node.len, qubit_index, node.arr, node_ptr=node_ptr)

    def __iter__(self):
        if self.measurement_schedule is None:
            self.construct_measurement_schedule()
        return iter(self.measurement_schedule)

    def construct_measurement_schedule(self):
        '''
            construct_measurement_schedule
            Constructs a measurement schedule from
            the rust library
        '''
        if self.measurement_schedule is None:
            schedule = []
            for layer_idx in range(self.n_layers):
                layer = self.get_layer(layer_idx)
                schedule.append([])
                n_dependents = self.layer_get_n_dependents(layer)
                for dep_idx in range(n_dependents):
                    dependent = self.layer_get_dependent(layer, dep_idx)
                    if dependent.qubit_index < self.max_qubit:
                        schedule[-1].append(dependent.to_dict())
            self.measurement_schedule = schedule
        return self.measurement_schedule

    @staticmethod
    def qubit_index(sched_obj: dict) -> int:
        '''
            Gets the index of a qubit
        '''
        return next(iter(sched_obj))

    @staticmethod
    def qubit_indices(layer: list) -> GeneratorType:
        '''
            Call qubit index over each element in the layer
            Returns a generator of the indices
        '''
        return map(PauliTracker.qubit_index, layer)

    def debug_print(self, graph=True):
        '''
            Call through to the rust print functions for the graph and the tracker
        '''
        lib.lib_pauli_tracker_print(self.pauli_tracker_ptr)
        if graph:
            lib.lib_pauli_tracker_graph_print(self.graph_ptr)

    @staticmethod
    def get_correction_ptr(fn):
        '''
        Decorator for ensuring that the correction pointer has been pulled from the tracker
        As there is a timing mismatch between the construction of the tracker and
        running the scheduler this can't be performed as part of init
        '''
        def _wrap(self, *args, **kwargs):
            if self.corrections_ptr is None:
                self.corrections_ptr = lib.lib_pauli_tracker_create_pauli_corrections(
                    self.pauli_tracker_ptr
                )
            if self.inv_mapper is None:
                mapper_ptr = lib.lib_pauli_tracker_get_inv_mapper(
                    self.pauli_tracker_ptr,
                    self.max_qubit
                )
                self.inv_mapper = InvMapper(mapper_ptr)
            return fn(self, *args, **kwargs)
        return _wrap

    @get_correction_ptr
    def __getitem__(self, index):
        '''
            Gets the Pauli corrections for that index
        '''
        return self.__get_correction(index)

    @get_correction_ptr
    def get_pauli_corrections(self, idx=None, fmt=lambda x: x.to_dict()):
        '''
            Returns an iterator of corrections
            :: fmt : lambda :: Format as to_list, to_dict or to_tuple
        '''
        if self.corrections is None:
            self.corrections = list(
                fmt(self.__get_correction(idx)) for idx in range(self._get_correction_table_len())
            )
        if idx is None:
            return self.corrections
        return self.corrections[idx][idx]

    def _get_correction_table_len(self):
        return lib.lib_pauli_tracker_get_correction_table_len(self.corrections_ptr)

    def __get_correction(self, index):
        '''
            Gets the set of corrections for a given index
        '''
        mapper_index = self.inv_mapper[index]
        return PauliCorrection(
            mapper_index,
            lib.lib_pauli_tracker_get_pauli_corrections(self.corrections_ptr, index),
            self.max_qubit
        )

    def __del__(self):
        if self.graph_ptr is not None:
            lib.lib_pauli_tracker_graph_destroy(self.graph_ptr)
