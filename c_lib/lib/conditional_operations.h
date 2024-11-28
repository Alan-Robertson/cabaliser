#ifndef CONDITIONAL_OPERATIONS_H
#define CONDITIONAL_OPERATIONS_H

#include "widget.h"
#include "lib_pauli_tracker.h" 

/*
 * conditional_x
 * Implements a measurement conditioned X operation
 * :: wid : widget_t* :: Widget to implement on
 * :: ctrl : size_t :: Control Qubit 
 * :: targ : size_t :: Target Qubit
 * Acts by calling the Pauli tracker
 */
void conditional_x(widget_t* wid, size_t ctrl, size_t targ);

/*
 * conditional_z
 * Implements a measurement conditioned Z operation
 * :: wid : widget_t* :: Widget to implement on
 * :: ctrl : size_t :: Control Qubit 
 * :: targ : size_t :: Target Qubit
 * Acts by calling the Pauli tracker
 */
void conditional_z(widget_t* wid, size_t ctrl, size_t targ);


/*
 * conditional_y
 * Implements a measurement conditioned Y operation
 * :: wid : widget_t* :: Widget to implement on
 * :: ctrl : size_t :: Control Qubit 
 * :: targ : size_t :: Target Qubit
 * Acts by calling the Pauli tracker
 */
void conditional_y(widget_t* wid, size_t ctrl, size_t targ);



#endif
