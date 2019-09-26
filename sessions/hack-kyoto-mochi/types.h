#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc_string.h>

/* We use the Mercury macros to define the input
 * and output structures along with the serialization
 * functions.
 */
MERCURY_GEN_PROC(set_in_t,
		 ((hg_const_string_t)(key))\
		 ((hg_const_string_t)(value)))

MERCURY_GEN_PROC(set_out_t, ((int32_t)(ret)))

MERCURY_GEN_PROC(get_in_t,
		 ((hg_const_string_t)(key)))

MERCURY_GEN_PROC(get_out_t, ((hg_string_t)(value)))

MERCURY_GEN_PROC(rm_in_t,
		 ((hg_const_string_t)(key)))

MERCURY_GEN_PROC(rm_out_t, ((int32_t)(value)))

#endif
