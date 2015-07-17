/**
 * File list.c
 * ---------------------------------------------
 * Generic simple linked lists source file
 */

#include "list.h"

/**
 * Create a new node, and init it with the provided values
 * 
 * @param  data Void pointer to the data. Can be NULL
 * @return      The newly allocated and initialized node
 */
static node* node_new(void* data) {
    node* n = malloc(sizeof(node));
    
    n->next = NULL;
    n->data = data;
    
    return n;
}

/**
 * Return the last node of a list, of NULL if there is no nodes
 * 
 * @param  l The list itself
 * @return   The last node of the list, or NULL
 */
static node* list_last(list* l) {
    node* it = l->nodes;
    
    if(!it)
        return NULL; // No value
        
    while(it->next)
        NEXT(it);
    
    return it;
}

/**
 * Initialize a given list
 * 
 * @param l The list to initialize. Must be allocated
 */
void list_init(list* l) {
    l->length = 0;
    l->nodes = NULL;
}

/**
 * Add a value at the end of the list
 * 
 * @param  l   The list itself
 * @param data The data to prepend to the list. Can be NULL
 */
void list_append(list* l, void* data) {
    node* last = list_last(l);
    
    if(last)
        last->next = node_new(data);
    else
        l->nodes = node_new(data);
        
    l->length++;
}

/**
 * Add a value at the beginning of the list
 * 
 * @param l    The list itself
 * @param data The data to append to the list. Can be NULL
 */
void list_push(list* l, void* data) {
    node* first = l->nodes;
    
    l->nodes = node_new(data);
    
    if(first)
        l->nodes->next = first;
        
    l->length++;
}

/**
 * Apply the provided method to every element of the list. Additional
 * parameters can be given. The handler must take in parameter the value
 * then the provided args (both void* values)
 * 
 * @param l    The list itself
 * @param args The arguments provided to the function to apply to each value
 * @param l    The callback which will be called on each value
 */
void list_each(list* l, void* args, void(*func)(void*, void*)) {
    node* it = l->nodes;
    
    while(it) {
        func(it->data, args);
        NEXT(it);
    }
}

/**
 * Apply the provided method to every element of the list. When the
 * test method return a true result (!= 0) the element is returned.
 * If no result could be found, NULL is returned
 * 
 * @param l    The list itself
 * @param args The arguments provided to the function to apply to each value
 * @param l    The callback which will be called on each value to test it
 */
void* list_find(list* l, void* args, int(*test)(void*, void*)) {
    node* it = l->nodes;
    
    while(it) {
        if(test(args, it->data))
            return it->data;
        NEXT(it);
    }
    
    return NULL;
}

/**
 * Iter on each value of the list, and remove the element if the provided
 * test method returns a true value (!= 0). The handler must take in parameter the value
 * then the provided args (both void* values)
 * 
 * @param l    The list itself
 * @param args The arguments provided to the function to apply to each value
 * @param l    The callback which will be called on each value to find value to delete
 */
void list_remove(list* l, void* args, int(*test)(void*, void*), void(*func)(void*)) {
    node* it = l->nodes;
    node* prev = NULL;
    
    while(it) {
        node* tmp = it;
        NEXT(it);
        
        // Test if element should be deleted
        if(test(args, tmp->data)) {
            if(func) // Custom free
                func(tmp->data);
            free(tmp);
            
            if(prev)
                prev->next = it;
            else
                l->nodes = it;
        } else
            prev = tmp;
    }
}

/**
 * Remove the first element of the list, and return its value. Note that if the list
 * is empty, NULL will be returned (same if the first element contains a NULL value)
 * 
 * @param l    The list itself
 */
void* list_pop(list* l) {
    void* data = NULL;
    
    if(l->nodes) {
        node* first = l->nodes;
        data = first->data;
        
        l->nodes = first->next;
        free(first);
        
        l->length--;
    }
    
    return data;
}

/**
 * Remove the last element of the list, and return its value. Note that if the list
 * is empty, NULL will be returned (same if the last element contains a NULL value)
 * 
 * @param l    The list itself
 */
void* list_pop_last(list* l) {
    void* data = NULL;
    
    if(l->nodes) {
        node* it = l->nodes;
        
        while(it->next)
            NEXT(it);
        
        data = it->data;
        
        l->nodes = it->next;
        free(it);
        
        l->length--;
    }
    
    return data;
}

/**
 * Remove all the elements of the list. If the second parameter is not NULL, the provided
 * functionn will be called for every element of the list
 * 
 * @param l    The list itself
 * @param fun  The function called to each element deleted from the list
 */
void list_clear(list* l, void(*func)(void*)) {
    
    if(l->nodes) {
        node* it = l->nodes;
        node* to_delete;
        
        while(it) {
            to_delete = it;
            NEXT(it);
            
            if(func)
                func(to_delete->data);
            free(to_delete);
        }
    }
    
    l->length = 0;
}

