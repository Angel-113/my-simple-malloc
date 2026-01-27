#ifndef HEADER_H
#define HEADER_H

#include "base.h"

typedef u64 Header;

extern bool get_color ( Header header );
extern bool get_status ( Header header );
extern u64 get_size ( Header header );

extern void set_color ( Header* header, bool color );
extern void set_status ( Header* header, bool status );
extern void set_size ( Header* header, u64 size );   


#define __red 1
#define __black 0
#define __free 1
#define __in_use 0

#endif
