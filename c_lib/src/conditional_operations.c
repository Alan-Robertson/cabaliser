#include "conditional_operations.h"

void conditional_I(widget_t* wid, size_t ctrl, size_t targ)
{
    // No operation
}

void conditional_x(widget_t* wid, size_t ctrl, size_t targ)
{
    pauli_track_x(wid->pauli_tracker, ctrl, targ);
}

void conditional_z(widget_t* wid, size_t ctrl, size_t targ)
{
    pauli_track_z(wid->pauli_tracker, ctrl, targ);
}

void conditional_y(widget_t* wid, size_t ctrl, size_t targ)
{
    pauli_track_y(wid->pauli_tracker, ctrl, targ);
}
