/* 
 * AstriFlash context library
 * Adapted from boost 1.60.0 fcontext library
 */
#ifndef AF_CONTEXT_H
#define AF_CONTEXT_H

#include <stdlib.h>

typedef void* af_context_t;

af_context_t make_afcontext(void* sp, size_t stack_size, void (* fn)(void));
void* jump_afcontext(af_context_t* original_context, af_context_t new_context, void* data);

#endif