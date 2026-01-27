#include "../include/base.h"
#include "../include/rb_tree.h"
#include <stdbool.h>
#include <stdio.h>

static Node __sentinel_value = {
  0, &__sentinel_value, &__sentinel_value, &__sentinel_value
};

Node* __sentinel = &__sentinel_value; 

typedef enum ChildKind {
    LEFT,
    RIGHT,
    ROOT
} ChildKind; 

/* Some helpers */

static Header* get_footer ( Node* node );
static Node* get_next_node ( Node* node );
static Node* get_substitute ( Node* node );  
static void disconnect_node ( Node* node ); 
static void left_rotate ( Node** root, Node* node );
static void right_rotate ( Node** root, Node* node );
static void left_right_rotate ( Node** root, Node* node );
static void right_left_rotate ( Node** root, Node* node ); 
static bool fix_deletion ( Node** root, Node* result, bool black_token );
static bool fix_subtree ( Node** root, Node* current_subtree ); 
static void swap_nodes ( Node* inorder, Node* node ); /* Just changes the tree hierarchy */
static ChildKind get_child_kind ( Node* node ); 

/* Implementations */

Node* insert ( Node** root, Node* new_node ) { /* top-down insertion */
    u64 target = get_size(new_node->header);
    Node* current = *root;
    Node* parent = __sentinel; 

    while ( current != __sentinel ) {
        parent = current; 
        current = target < get_size(current->header) ? current->left : current->right; 
        if ( get_status(current->left->header) && get_status(current->right->header) )
            fix_subtree(root, current); 
    }

    /* insert node */
    Node** child = target < get_size(parent->header) ? &parent->left : &parent->right;
    *child = new_node;
    (*child)->parent = parent;

    /* perform last fix if needed */
    fix_subtree(root, *child); 
    
    return new_node;     
}

Node* delete ( Node** root, Node* node ) { /* bottom-up deletion */
    if ( !root || *root == __sentinel ) {
        print_error("Corrupted tree: root == NULL\n");
        return NULL; 
    }

    else if ( !node || node == __sentinel ) {
        print_error("Cannot delete a NULL pointer\n");
        return NULL; 
    }

    Node* result = get_substitute(node);
    Node* parent = node->parent;
    Node** new_child = node == *root ? &(*root) : ( parent->left == node ? &parent->left : &parent->right ); 
    bool black_token = !get_color(node->header); 
 
    if ( result == __sentinel ) { /* no child */
        (*new_child) = __sentinel; 
        disconnect_node(node); 
    }
    else if ( result == node->left || result == node->right ) { /* one child */
        (*new_child) = result;
        (*new_child)->left = result->left;
        (*new_child)->right = result->right;
        (*new_child)->parent = new_child == &(*root) ? __sentinel : parent; 
        disconnect_node(node);
    }
    else { /* two child */
        swap_nodes(result, node);
        delete(root, node); 
    }

    fix_deletion(root, result, black_token);
        
    return node; 
}

Node* init_node ( void* ptr, u64 size, bool color, bool status ) { /* init node assumes that ptr will be node's address */
    Node* node = ptr;
    set_color(&node->header, color);
    set_status(&node->header,  status);
    set_size(&node->header, size);
    set_footer(node); 
    return node; 
}

Node* search ( Node* root, u64 target ) { /* best fit algorithm */
    Node* current = root;
    while ( current->left != __sentinel && current->right != __sentinel ) {
        if ( get_size(current->header) == target ) break; 
        current = target < get_size(current->header) ? current->left : current->right;  
    }
    return current; 
}

Node* get_node ( void* ptr ) { /* get_node assumes ptr = (u8 *)original_node + sizeof(Header); */
    return (Node *)((u8 *)ptr - sizeof(Header)); 
}

void set_footer (Node* node) { /* A node's footer must be equal to its header */
    Header* footer = get_footer(node);
    *footer = node->header; 
}

Node* merge_nodes ( Node* a, Node* b ) { /* merge_nodes assumes a and b are free memory contiguous nodes */
    Node* left = a < b ? a : b;
    Node* right = left == a ? b : a; 

    if ( right != get_next_node(left) ) {
        print_error("Trying to merge non-memory-contiguous nodes\n");
        return __sentinel;  
    }
    
    if ( get_status(a->header) || get_status(b->header) ) {
        print_error("Trying to merge non-free nodes\n");
        return __sentinel; 
    }

    u64 new_size = get_size(a->header) + get_size(b->header) + 2 * sizeof(Header); 
    left = init_node(left, new_size, __red, __free); 

    return left; 
}

/* Helper implementations */

static Header* get_footer ( Node* node ) {
    return (Header *)( (u8 *)node + sizeof(Header) + get_size(node->header) );
}

static Node* get_next_node ( Node* node ) {
    return (Node *)( (u8 *)get_footer(node) + sizeof(Header) ); 
}

static void disconnect_node ( Node* node ) {
    node->parent = node->left = node->right = __sentinel; 
}

static ChildKind get_child_kind( Node* child ) {
    return child->parent == __sentinel ? ROOT : ( child->parent->left == child ? LEFT : RIGHT ); 
}

static void left_rotate ( Node** root,  Node* node ) { /* perform rotation by pushing node down and indirect linking */
    Node* current_parent = node;
    Node* current_right = node->right;
    Node* new_right = current_right->left; 

    Node** new_node = node != *root ? ( (node->parent)->left == node ? &(node->parent)->left : &(node->parent)->right ) : root;
    *new_node = current_right;
    (*new_node)->left = current_parent;
    (*new_node)->parent = current_parent->parent;

    current_parent->parent = *new_node;
    current_parent->right = new_right;
    new_right->parent = current_parent; 
}

static void right_rotate ( Node** root, Node* node ) {
    Node* current_parent = node;
    Node* current_left = node->left;
    Node* new_left = current_left->right;

    Node** new_node = node != *root ? ( (node->parent)->left == node ? &(node->parent)->left : &(node->parent)->right ) : root;
    *new_node = current_left;
    (*new_node)->right = current_left;
    (*new_node)->parent = current_parent->parent;

    current_parent->parent = *new_node;
    current_parent->left = new_left;
    new_left->parent = current_parent;
}

static void left_right_rotate ( Node** root, Node* node ) {
    left_rotate(root, node->right);
    right_rotate(root, node);
}

static void right_left_rotate ( Node** root, Node* node ) {
    right_rotate(root, node->left);
    left_rotate(root, node);
}

static void swap_nodes ( Node* inorder, Node* node ) { /* inorder->left is __sentinel */
    Node* node_parent = node->parent;
    Node* node_left = node->left;
    Node* node_right = node->right;
    Node* inor_parent = inorder->parent;
    Node* inor_right = inorder->right;

    bool color_node = get_color(node->header);
    bool color_inor = get_color(inorder->header); 

    Node** new_node = node_parent != __sentinel ? (node_parent->left == node ? &node_parent->left : &node_parent->right) : &node;
    Node** new_inorder = inor_parent != node ? (inor_parent->left == inorder ? &inor_parent->left : &inor_parent->right) : &inorder;

    /* perform swap by indirect linking */
    (*new_node) = inorder;
    (*new_inorder) = node; 

    (*new_node)->parent = node_parent; 
    (*new_node)->right = node_right == inorder ? *new_inorder : node_right;
    (*new_node)->left = node_left;

    (*new_inorder)->parent = inor_parent; 
    (*new_inorder)->right = inor_right;
    (*new_inorder)->left = __sentinel;

    /* preserve color: if node was black, then, new_node is black  */
    set_color(&(*new_inorder)->header, color_inor);
    set_color(&(*new_node)->header, color_node); 
}

static Node* get_substitute ( Node* node ) {
    Node* result = __sentinel; 
    if ( node->left != __sentinel && node->right != __sentinel ) {
        result = node->right;
        while ( result->left != __sentinel ) result = result->left;
    }
    else if ( node->left != __sentinel || node->right != __sentinel ) result = node->left != __sentinel ? node->left : node->right; 
    return result; 
}

static bool fix_deletion ( Node** root, Node* result, bool black_token ) { /* bottom-up deletion */
    Node* current = result;
    
    while ( current != *root && black_token ) { /* push the black_token up the tree */
        Node* sibling = current->parent->left == current ? current->parent->right : current->parent->left;        
        Node* grandparent = current->parent->parent;

        if ( current == __sentinel ) break;  /* reach the root */
        
        bool color_parent = get_color(current->parent->header);
        bool color_sibling = get_color(sibling->header);
        bool color_current = get_color(current->header); 
        ChildKind current_kind = get_child_kind(current);

        if ( get_color(current->header) ) { /* found a red node */
            set_color(&current->header, __black);
            black_token = !black_token; 
        }
        else if ( get_color(sibling->header) ) { /* double black violations and red sibling */
            set_color(&(current->parent)->header, __red);
            set_color(&sibling->header, __black); 
            current_kind == LEFT ? left_rotate(root, current->parent) : right_rotate(root, current->parent); 
        }
        else if ( get_color(sibling->left->header) || get_color(sibling->right->header) ) { /* double black violatons and black sibling */
            Node** near = current_kind == LEFT ? &sibling->left : &sibling->right;
            Node** far = current_kind == LEFT ? &sibling->right : &sibling->left;  
            if ( !get_color((*far)->header) ) current_kind == LEFT ? left_rotate(root, sibling) : right_rotate(root, sibling); 
            current_kind == LEFT ? right_rotate(root, current->parent) : left_rotate(root, current->parent); 

            black_token = !black_token;       
        }
        else { /* black sibling and two black nephews */
             set_color(&sibling->header, __red);
             if ( get_color(sibling->parent->header) ) set_color(&(sibling->parent)->header, __black); 
        }
        
        current = grandparent; 
    } 
      
    return true;
}

static bool fix_subtree ( Node** root, Node* current_subtree ) { /* fix insertion before inserting */
    set_color(&current_subtree->header, __red);
    set_color(&current_subtree->left->header, __black);
    set_color(&current_subtree->right->header, __black);

    Node* parent = current_subtree->parent;
    Node* grandpa = parent->parent; 

    if ( get_status(current_subtree->parent->header) ) { /* perform fixes */
        ChildKind current_kind = get_child_kind(current_subtree);
        ChildKind parent_kind = get_child_kind(current_subtree->parent);

        if ( current_kind == parent_kind ) { /* straight line */
            set_color(&parent->header, __black);
            set_color(&grandpa->header, __red); 
            current_kind == LEFT ? right_rotate(root, current_subtree) : left_rotate(root, current_subtree);
        }
        else { /* zig-zag */
            set_color(&grandpa->header, __red);
            set_color(&current_subtree->header, __black);  
            current_kind == LEFT ? right_left_rotate(root, current_subtree) : left_right_rotate(root, current_subtree);  
        }
    }

    set_color(&(*root)->header, __black); 
    return true;
}
