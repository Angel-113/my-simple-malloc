#include "../include/test.h"
#include "../include/rb_tree.h"
#include "../include/header.h"
#include "../include/rand.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>

#define MAX_NODES 1000

static Node* root = NULL;
static Node* nodes[MAX_NODES] = { 0 }; 
static pcg32_random_t my_rng = { 0 };

/* helpers */
static void init_rng ( void );
static u64 get_rng64 ( void );
static u32 get_rng32 ( void );
static void init_tester ( void );
static void insert_nodes ( void );
static void delete_nodes ( u32 n_nodes );

static bool blacks_property ( Node* root);
static u64 count_blacks ( Node* root );
static bool red_root ( Node* root );
static bool red_red ( Node* node );

int main ( void ) { /* main test */
    if ( !tree_test() ) print_error("SOMETHING WENT WRONG WITH THE TREE\n"); 
    return 0;
}

/* test implementations */

bool tree_test() {
    insert_nodes();
    bool test_insert = blacks_property(root) && red_root(root) && red_red(root);
    delete_nodes(MAX_NODES / 2); 
    bool test_delete = blacks_property(root) && red_root(root) && red_red(root);
    return test_delete && test_insert;   
}

bool test_insertion ( void ) {
    bool test = blacks_property(root) && red_red(root);
    if ( !nodes[0] ) {
        insert_nodes();
        test = blacks_property(root) && red_root(root) && red_red(root); 
    }
    else {
        static Node* current_nodes[MAX_NODES] = { 0 };
        for ( u64 i = 0; i < MAX_NODES; i++ ) {
            current_nodes[i] = init_node(malloc(sizeof(Node)), get_rng64(), __red, __free);
            insert(root, current_nodes[i]); 
        }
        test = blacks_property(root) && red_root(root) && red_root(root);
        for ( u64 i = 0; i < MAX_NODES; i++ ) free((void *)current_nodes[i]);
    }
    return test; 
}

/* helpers implementation */

static void init_rng ( void ) {
    pcg32_srandom_r(&my_rng, time(NULL), (intptr_t)&my_rng); 
}

static u64 get_rng64 ( void ) {
    return ldexpl(pcg32_random_r(&my_rng), -32); 
}

static u32 get_rng32 ( void ) {
    return pcg32_random_r(&my_rng); 
}

static void init_tester ( void ) { 
    if ( root ) return;
    root = malloc(sizeof(Node *));
    init_rng();
    set_size(&root->header, get_rng64());
    set_color(&root->header, __black);
    set_status(&root->header, __free); 
}

static void insert_nodes ( void ) {
    for ( u16 i = 0; i < MAX_NODES; i++ ) {
        if ( !nodes[i] ) nodes[i] = malloc( sizeof(Node ));
        nodes[i] = init_node(nodes[i], get_rng64(), __red, __free);
        insert(root, nodes[i]); 
    }
}

static void delete_nodes ( u32 n_nodes ) {
    while ( n_nodes-- ) {
        u32 index = get_rng32() % MAX_NODES; 
        if ( !nodes[index] ) n_nodes++;
        else {
            delete(root, nodes[index]);
            free((void*)nodes[index]); 
            nodes[index] = NULL; 
        }
    }
}

static u64 count_blacks ( Node* root ) {
    u64 blacks = 0;

    Node* current = root;  
    while ( current->left != __sentinel ) current = current->left; 

    while ( current != root ) {
        blacks += !get_color(current->header);
        current = current->parent;
        if ( current->right != __sentinel ) {
            current = current->right; 
            while ( current->left != __sentinel ) current = current->left;
        }
    }
    
    return blacks; 
}

static bool blacks_property ( Node* root ) {
    return count_blacks(root->left) == count_blacks(root->right); 
}

static bool red_root ( Node* root ) {
    return !get_color(root->header); 
}

static bool red_red ( Node* root ) { 
    return ( get_color(root->header) && (get_color(root->left->header) || get_color(root->right->header)) ); 
}
