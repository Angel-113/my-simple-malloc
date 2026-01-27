#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "base.h"


extern void* my_malloc ( u64 size );
extern void* my_realloc ( void* ptr, u64 size );
extern void my_free ( void* ptr ); 

#endif
