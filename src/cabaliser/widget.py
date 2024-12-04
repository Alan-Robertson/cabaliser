'''
    Widget object
    Exposes an API to the cabaliser c_lib's widget object
'''
from ctypes import POINTER, c_buffer

from cabaliser.operation_sequence import OperationSequence
from cabaliser.structs import AdjacencyType, WidgetType
from cabaliser.structs import LocalCliffordType, MeasurementTagType, IOMapType
from cabaliser.io_array_wrappers import MeasurementTags, LocalCliffords, IOMap
from cabaliser.qubit_array import QubitArray
from cabaliser.pauli_tracker import PauliTracker
from cabaliser.schedule_footprint import schedule_footprint
from cabaliser.utils import deref

from cabaliser.exceptions import WidgetNotDecomposedException, WidgetDecomposedException
from cabaliser import local_simulator

from cabaliser.lib_cabaliser import lib
# Override return type
lib.widget_create.restype = POINTER(WidgetType)


class Widget():
    '''
        Widget object
        Exposes an API to the cabaliser c_lib's widget object
    '''
    def __init__(self, n_qubits: int, n_qubits_max: int, teleport_input: bool = True, n_inputs: int=None):
        '''
            __init__
            Constructor for the widget
            Allocates a large tableau
            :: n_qubits : int :: Initial number of qubits, "width" of the widget  
            :: n_qubits_max : int :: Maximum size of the widget 
            :: teleport_input : bool :: Whether the inputs should be teleported
            :: n_inputs : int :: Optional, If the number of inputs differs from the size of the register   
        '''
        self.decomposed = False

        if n_inputs is None:
            n_inputs = n_qubits
        elif not (n_inputs <= n_qubits):
            raise IndexError("More inputs than qubits")

        self.n_inputs = n_inputs

        self.widget = lib.widget_create(n_qubits, n_qubits_max)
        self.teleport_input = teleport_input

        if self.teleport_input:
            lib.teleport_input(self.widget, self.n_inputs)
        else:
            lib.pauli_tracker_disable()

        self.local_cliffords = None
        self.measurement_tags = None
        self.io_map = None
        self.pauli_tracker = PauliTracker(self)

    def get_n_qubits(self) -> int:
        '''
            get_n_qubits
            Getter method for the widget
            Returns the current number of allocated qubits on the widget
        '''
        return lib.widget_get_n_qubits(self.widget)

    @property
    def pauli_tracker_ptr(self):
        '''
            Returns the opaque pointer to the pauli
            tracker object
        '''
        return deref(self.widget).pauli_tracker

    @property
    def n_qubits(self):
        '''
            n_qubits
            Property wrapper to get the current number of qubits in the widget
        '''
        return self.get_n_qubits()

    def get_max_qubits(self) -> int:
        '''
            get_max_qubits
            Getter method for the widget
            Returns the maximum number of allocatable qubits on the widget
        '''
        return lib.widget_get_max_qubits(self.widget)

    @property
    def max_qubits(self) -> int:
        '''
            max_qubits
            Property wrapper for get_max_qubits
        '''
        return self.get_max_qubits()

    def get_n_initial_qubits(self) -> int:
        '''
            get_initial_qubits
            Getter method for the widget
            Returns the initial number of allocatable qubits on the widget
        '''
        return lib.widget_get_n_initial_qubits(self.widget)

    @property
    def n_initial_qubits(self) -> int:
        '''
            Property wrapper for getting the number of initial qubits
        '''
        return self.get_n_initial_qubits()

    def process_operations(self, operations: OperationSequence):
        '''
            Parses an array of operations
            :: operations: Operations :: Wrapper around an array of operations
            Acts in place on the widget
        '''
        lib.parse_instruction_block(
            self.widget,
            operations.ops,
            operations.curr_instructions)

    def __call__(self, *args, **kwargs):
        '''
            Consumes a list of operations
        '''
        self.process_operations(*args, **kwargs)

    def __del__(self):
        '''
            __del__
            Explicit destructor for the widget
            Frees the underlying C object
        '''
        lib.widget_destroy(self.widget)

    def json(self, rz_to_float=False, local_clifford_to_string=True):
        '''
            Returns a dict object of all relevant properties
        '''
        obj = {
               'n_qubits': self.n_qubits,
               'statenodes': list(range(self.n_initial_qubits)),
               'adjacencies': {i: self.get_adjacencies(i).to_list() for i in range(self.n_qubits)},
               'local_cliffords': self.get_local_cliffords().to_list(
                    to_string=local_clifford_to_string),
               'consumptionschedule': self.pauli_tracker.to_list(),
               'measurement_tags': self.get_measurement_tags().to_list(to_float=rz_to_float),
               'paulicorrections': self.get_pauli_corrections(),
               'outputnodes': self.get_io_map().to_list()
               }
        # Frames flags
        # Corrections
        # Initialiser - Pretty sure this is all + states
        obj['time'] = len(obj['consumptionschedule'])
        obj['space'] = schedule_footprint(
            obj['adjacencies'],
            obj['consumptionschedule']
        )
        return obj

    @staticmethod
    def require_decomposed(fn):
        '''
            Decorator asserting that the widget has been decomposed before this method is called
        '''
        def _wrap(self, *args, **kwargs):
            if not self.decomposed:
                raise WidgetNotDecomposedException(
                    """Attempted to read out the graph state without decomposing the tableau.
                     Please call `Widget.decompose()` before extracting the adjacencies"""
                )
            return fn(self, *args, **kwargs)
        return _wrap

    @staticmethod
    def require_not_decomposed(fn):
        '''
            Decorator asserting that the widget has not been decomposed before this method is called
        '''
        def _wrap(self, *args, **kwargs):
            if self.decomposed:
                raise WidgetDecomposedException("Attempted to decompose twice")
            return fn(self, *args, **kwargs)
        return _wrap

    def get_local_cliffords(self):
        '''
            get_corrections
            Returns a wrapper around an array of the Pauli corrections
        '''

        if self.local_cliffords is None:

            local_cliffords = POINTER(LocalCliffordType)()
            ptr = POINTER(LocalCliffordType)(local_cliffords)
            lib.widget_get_local_cliffords_api(self.widget, ptr)
            self.local_cliffords = LocalCliffords(self.get_n_qubits(), local_cliffords)

        return self.local_cliffords

    @require_decomposed
    def get_measurement_tags(self):
        '''
            get_measurement_tags
            Returns a wrapper around an array of measurements
        '''
        if self.measurement_tags is None:
            measurement_tags = POINTER(MeasurementTagType)()
            ptr = POINTER(MeasurementTagType)(measurement_tags)
            lib.widget_get_measurement_tags_api(self.widget, ptr)
            self.measurement_tags = MeasurementTags(self.get_n_qubits(), measurement_tags)

        return self.measurement_tags

    @require_decomposed
    def get_io_map(self, idx=None):
        '''
            get_io_map
            Returns a wrapper around an array of measurements
        '''
        if self.io_map is None:
            io_map = POINTER(IOMapType)()
            ptr = POINTER(IOMapType)(io_map)
            lib.widget_get_io_map_api(self.widget, ptr)

            # In case any of these objects have been measured out
            measurement_tags = self.get_measurement_tags()
            self.io_map = IOMap(self.get_n_initial_qubits(), io_map, measurement_tags)
        if idx is None: 
            return self.io_map
        return self.io_map[idx]

    @require_decomposed
    def get_adjacencies(self, qubit: int) -> AdjacencyType:
        '''
            Given a qubit get the adjacencies on the graph
            Reported qubits are graph state indices
        '''
        if qubit > self.get_n_qubits():
            raise IndexError("Attempted to access qubit out of range")

        # Honestly this is cleaner than manually setting return types on globally scoped objects
        adjacency_obj = AdjacencyType()
        ptr = POINTER(AdjacencyType)(adjacency_obj)
        lib.widget_get_adjacencies_api(self.widget, qubit, ptr)
        adj = Adjacency(adjacency_obj)

        return adj

    @require_not_decomposed
    def decompose(self):
        '''
            Decomposes the operation sequence into an algorithmically specific graph (asg)
        '''
        lib.widget_decompose(self.widget)

        # Create the measurement schedule
        self.__schedule()

        # Set the decomposed flag
        self.decomposed = True

    def __schedule(self):
        '''
            Call through to the pauli tracker for
             scheduling
        '''
        self.pauli_tracker.schedule(max_qubit=self.n_qubits)

    @require_decomposed
    def get_pauli_corrections(self, idx=None):
        '''
            Dispatch method to get pauli corrections from pauli tracker wrapper
        '''
        return self.pauli_tracker.get_pauli_corrections(idx=idx)

    @require_decomposed
    def get_schedule(self):
        '''
            Gets the schedule from the pauli tracker
        '''
        return self.pauli_tracker.to_list()

    @require_not_decomposed
    def load_pandora(self, db_name: str):
        '''
            load_pandora
            Loads gates from a pandora database
        '''
        db_name = c_buffer(db_name.encode('ascii'))
        lib.pandora_n_qubits(db_name)

        lib.pandora_load_db(self.widget, db_name)

    @require_decomposed
    def to_sim(self, *input_states, table=None):
        '''
            to_sim
            Simulates the widget
        '''
        return local_simulator.simulate_widget(self, *input_states, table=table)

    def tableau_print(self):
        '''
            Prints the current state of the underlying tableau
        '''
        lib.widget_print_tableau_api(self.widget)


class Adjacency(QubitArray):
    '''
        Adjacency
        Python wrapper for adjacency object
    '''
    def __init__(self, adj: AdjacencyType):
        self.adj = adj
        self.src = int(self.adj.src)
        self.n_adjacent = int(self.adj.n_adjacent)
        super().__init__(self.n_adjacent, self.adj.adjacencies)

    def __repr__(self) -> str:
        return f"Qubit: {self.src} {list(iter(self))}"

    def __str__(self) -> str:
        return self.__repr__()

    def __del__(self):
        lib.widget_destroy_adjacencies(
            POINTER(AdjacencyType)(self.adj)
        )
