#ifndef TEST_H
#define TEST_H

#include "../include/base.h"

extern bool general_test ( void );

extern bool tree_test ( void );
extern bool tree_test_insertion ( void );
extern bool tree_test_deletion ( void );
extern bool tree_test_fix_deletion ( void );
extern bool tree_test_fix_subtree ( void );

extern bool test_malloc ( void );
extern bool test_realloc ( void );
extern bool test_free ( void );
extern bool test_fragmentation ( void ); 

#endif
