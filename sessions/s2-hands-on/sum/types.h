#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc_string.h>

/* We use the Mercury macros to define the input
 * and output structures along with the serialization
 * functions.
 */
MERCURY_GEN_PROC(sum_in_t,
        ((int32_t)(x))\
        ((hg_const_string_t)(str)))

MERCURY_GEN_PROC(sum_out_t, ((int32_t)(ret)))

#endif
