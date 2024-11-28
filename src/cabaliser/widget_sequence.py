"""
Widget Sequence
"""

from cabaliser.widget import Widget
from cabaliser.operation_sequence import OperationSequence
from cabaliser import exceptions


class WidgetSequence:
    """
    Creates a sequence of widget objects
    Each widget has the same number of initial and max qubits
    """

    def __init__(self, qubit_width: int, max_qubits: int = 2000):
        """
        Initialiser for a widget sequence
        :: qubit_width : int :: Number of addressable qubits in each widget
        :: max_qubits : int :: Maximum number of qubits in each widget
        """
        if 2 * qubit_width >= max_qubits:
            raise exceptions.WidgetNotEnoughQubitsException(
                "Max qubits must be more than twice the number of initial qubits"
            )

        self.seq = []
        self._current_wid = None
        self._current_seq = None
        self._json = []
        self.rz_threshold = max_qubits - 2 * qubit_width
        self.qubit_width = qubit_width
        self.max_qubits = max_qubits

    def __call__(
        self,
        ops: OperationSequence,
        progress: bool = True,
        store_output: bool = True,
        **widget_args
    ):
        """
        Processes an operations sequence
        :: ops : OperationSequence :: Sequence of operations to split and process
        :: progress : bool :: Simple progress printer
        :: store_output : bool :: Whether to save json objects
        :: **widget_args :: Args for the widget
            - rz_to_float=False
            - local_clifford_to_string=True 
        """
        ops_sequence = ops.split(self.rz_threshold)
        for i, seq in enumerate(ops_sequence):
            if progress:
                print(f"\r{i + 1} of {len(ops_sequence)}", flush=True, end="")

            wid = Widget(self.qubit_width, self.max_qubits)
            wid(seq)
            wid.decompose()

            if store_output:
                self._json.append(wid.json(**widget_args))

            # Widget objects are large, clear memory!
            del wid

    def json(self):
        """
        Returns the stored json objects for this sequence
        """
        return self._json
