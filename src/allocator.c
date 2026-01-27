#include "../include/allocator.h"
#include "../include/base.h"
#include "../include/rb_tree.h"
#include <sys/mman.h>

#define PAGES( size ) (((size) + (PAGE - 1)) & ~(PAGE - 1))
#define PAGE 4092
#define MAX_THREADS 10

static u32 id_current_root = 0; 
static Node* current_roots[ MAX_THREADS ] = { 0 }; 

static Node** get_current_root ( void );
static u16 get_current_root_id ( void ); 
static bool memcopy ( void* src, void* dest, u64 size );

static Node* add_mem_page( u64 size ) { /* syscall for mempages */
    return  mmap(NULL, PAGES(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void* my_malloc ( u64 size ) { 
    void* ptr = NULL;
    Node** root = get_current_root(); 

    if ( !root ) {
        *root = add_mem_page( size );
        Node* used_node = init_node( *root, size, __red, __in_use );
        *root = (Node *)((u8 *)used_node + size + 2 * sizeof(Header));
        *root = init_node(*root, PAGES(size) - (size + 4 * sizeof(Header)), __black, __free );  
        ptr = (u8 *)used_node + sizeof(Header);
        current_roots[0] = *root; 
    }
    else if ( (*root)->left == (*root)->right ) { /* still splitting memory */
        ptr = (u8 *)root + sizeof(Header);
        u64 new_size = get_size( (*root)->header ) - size;
        bool status = get_status( (*root)->header );
        bool color = get_color( (*root)->header );

        if ( new_size <= size ) { /* last split in the memory segment */
            Node* new_node = init_node(root, new_size - sizeof(Header), __red, __free);  
            *root = add_mem_page(new_size);
            *root = init_node(root, PAGES(size), __black, __free); 
            insert( root, new_node ); 
        }
        
        else  { /* main split */
            *root = (Node *)( (u8*) root + 2 * sizeof(Header) + size );
            *root = init_node( *root, size, color, status ); 
        }

        current_roots[get_current_root_id()] = *root; /* update the root */
    }
    else { /* now we search in a free list */
        Node* result = search (*root, size);
        delete(root, result);
        ptr = (u8 *)result + sizeof(Header); 
    }

    return ptr; /* let the user handle the NULL case */  
}

void* my_realloc ( void* ptr, u64 size ) {
    void* new_ptr = my_malloc ( size ); 
    if ( !new_ptr ) {
        print_error("Malloc function returned NULL ptr\n"); 
        return NULL;
    }
    return memcopy(ptr, new_ptr, size) ? new_ptr : NULL; /* let the user handle the NULL case */ 
}

void my_free ( void* ptr ) {
    Node* node = (Node *) ((u8*) ptr - sizeof(Header));

    if ( get_status(node->header) ) {
        print_error("Double free operation\n"); 
        return;     
    }

    set_status(&node->header, __free);

    Header prev_footer = (Header)( (u8*)node - sizeof(Header) );
    Node* prev_node = (Node *)( (u8*)node - (2 * sizeof(Header) + get_size(prev_footer)) ); 
    Node* next_node = (Node *)( (u8*)node + 2 * sizeof(Header) + get_size(node->header) );

    /* merge nodes */    
    Node* merged_node = __sentinel;
        if ( get_status(prev_node->header) ) {
        delete(get_current_root(), prev_node);
        merged_node = merge_nodes(prev_node, node); 
    }

    if ( get_status(next_node->header) ) {
        delete(get_current_root(), next_node);
        merged_node = merged_node == __sentinel ? merge_nodes(node, next_node) : merge_nodes(merged_node, next_node); 
    }

    if ( merged_node == __sentinel ) insert(get_current_root(), node);
    else insert(get_current_root(), merged_node); 
}

static bool memcopy( void* src, void* dest, u64 size ) { 
    u8* src_aux = src;
    u8* dest_aux = dest;

    if ( src + size == dest || dest + size == src ) {
        print_error("overlapping memory segments\n");
        return false; 
    }
    
    for ( u64 i = 0; i < size; i++ ) dest_aux[i] = src_aux[i];  

    return true;
}

Node** get_current_root ( void ) { /* will help to mange threads */
    return current_roots[0]->header == 0 ? NULL : &current_roots[0]; 
}

u16 get_current_root_id ( void ) { /* main root is 0 */
    return 0; 
}
