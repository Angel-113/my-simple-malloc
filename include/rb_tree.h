#ifndef RB_TREE_H
#define RB_TREE_H

#include "base.h"
#include "header.h"

/*  
    |- - -|- - -  - --|- - -|
    |  H  |           |  F  |
    |  E  |           |  O  |
    |  A  |  D A T A  |  O  |
    |  D  |           |  T  |
    |  E  |           |  E  |
    |  R  |           |  R  |
    |- - -|- - - -- --|- - -|

    You could see the DATA segment as:
    union {
      void* ptr; (buffer)
      Node* nodes[3]; (parent, left and right)
    }
 */

typedef struct Node {
  Header header;  
  struct Node* parent;
  struct Node* right;
  struct Node* left;
} Node; 

extern Node* __sentinel;

extern Node* insert ( Node** root, Node* new_node );
extern Node* delete ( Node** root,  Node* node );
extern Node* init_node ( void* ptr, u64 size, bool color, bool status );
extern Node* search ( Node* root, u64 target ); 
extern Node* get_node ( void* ptr ); 
extern void set_footer ( Node* node ); 
extern Node*  merge_nodes( Node* a, Node* b ); 

#endif
