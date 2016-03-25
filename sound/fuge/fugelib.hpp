//      © Christian Sommerfeldt Øien
//      All rights reserved
#ifndef FUGELIB_H
#define FUGELIB_H

#include "Python.h"
#include "unitfwd.hpp"

namespace fuge {

extern double tempo;
void init_orchestra();
void init_filters();
double parse_float(PyObject * n);
bu_ptr parse_filter(bu_ptr && input, PyObject * seq, bool no_duration = false);
bu_ptr parse_note(PyObject * seq, double span = 0);

}

#endif
