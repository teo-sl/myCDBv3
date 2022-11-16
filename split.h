#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


uint32_t create_new_root(Table* table,uint32_t left_page_num,uint32_t right_page_num,uint32_t new_key);

void set_left_right_father(Table* table,uint32_t left_page_num, uint32_t right_page_num, uint32_t left_father, uint32_t right_father) {
    Page_header* pageHeader_left = (Page_header*) table->pages[left_page_num];
    Page_header* pageHeader_right = (Page_header*) table->pages[right_page_num];

    pageHeader_left->father_page=left_father;
    pageHeader_right->father_page=right_father;
}

void add_to_father(Table* table, void* page, uint32_t key, uint32_t left_page_num, uint32_t right_page_num) {
    uint32_t  idx = binary_search_internal(page,key);
    Page_header* pageHeader = (Page_header*) page;
    uint32_t size = pageHeader->n_keys;

    if(size<INTERNAL_MAX_KEYS) { // add directly
        uint32_t keys_to_move = size-idx;
        void* cell_dst = get_n_key_ptr_internal(page,idx);
        Key_internal* keyInternal = (Key_internal*) (cell_dst-POINTER_SIZE);

        Key_internal* keyInternal1 = (Key_internal*) (cell_dst+KEY_SIZE);

        memcpy((cell_dst+KEY_SIZE+POINTER_SIZE),cell_dst,keys_to_move*(KEY_SIZE+POINTER_SIZE));
        keyInternal->left_page=left_page_num;
        keyInternal->right_page=right_page_num;
        keyInternal->key_value=key;
        pageHeader->n_keys=size+1;
        set_left_right_father(table,left_page_num,right_page_num,pageHeader->page_num,pageHeader->page_num);
    }
    else {
        uint32_t idx_split = get_unused_page(table);
        void* new_page = table->pages[idx_split];
        void* old_page = page;

        // better define later n_keys, father_page and is_root
        initialize_internal_node(new_page,idx_split);

        Key_internal key_internal;
        key_internal.right_page=right_page_num;
        key_internal.left_page=left_page_num;
        key_internal.key_value=key;

        uint32_t pos = binary_search_internal(page,key);
        void* destination__;
        uint32_t father_to_return;

        for (uint32_t i = INTERNAL_MAX_KEYS; i >= 0; i--) {
            void *destination_node;
            if (i >= LEAF_LEFT_SPLIT) {
                destination_node = new_page;
            } else {
                destination_node = old_page;
            }
            uint32_t idx_pos = i % LEAF_LEFT_SPLIT;
            // destination from left pointer
            void *destination = (get_n_key_ptr_internal(destination_node, idx_pos)-POINTER_SIZE);

            if (i == pos) {
                destination__ = destination;
                father_to_return=((Page_header*) destination_node)->page_num;
                serialize_key(key_internal, destination);
            } else if (i > pos) {
                memcpy(destination, (get_n_key_ptr_internal(old_page,i-1)-POINTER_SIZE),POINTER_SIZE+KEY_SIZE+POINTER_SIZE);
            } else {
                memcpy(destination-POINTER_SIZE, (get_n_key_ptr_internal(old_page,i)-POINTER_SIZE),POINTER_SIZE+KEY_SIZE+POINTER_SIZE);
            }
            if (i == 0) {
                break;
            }
        }

        // preserve from pointer loss
        serialize_key(key_internal, destination__);

        Page_header *pageHeader_old = (Page_header *) old_page;
        Page_header *pageHeader_new = (Page_header *) new_page;



        uint32_t  new_key = ((Key_internal*) (get_n_key_ptr_internal(old_page,pageHeader_old->n_keys-1)-POINTER_SIZE))->key_value;
        uint32_t right_pointer_to_propagate = ((Key_internal*) (get_n_key_ptr_internal(old_page,pageHeader_old->n_keys-1)-POINTER_SIZE))->right_page;

        //propagate right pointer to the first element in right page
        *((uint32_t*)(get_n_key_ptr_internal(new_page,0)-POINTER_SIZE))=right_pointer_to_propagate;

        // last left element is promoted to father node so internal_left_split-1
        pageHeader_old->n_keys = INTERNAl_LEFT_SPLIT-1;
        pageHeader_new->n_keys = INTERNAL_RIGHT_SPLIT;

        if(pageHeader_old->is_root) {
            pageHeader_old->is_root=0;
            uint32_t root_page = create_new_root(table,pageHeader_old->page_num,pageHeader_new->page_num,new_key);
            pageHeader_old->father_page = root_page;
            pageHeader_old->father_page = root_page;
            set_left_right_father(table,left_page_num,right_page_num,pageHeader_old->page_num,pageHeader_new->page_num);
        }
        else {
            add_to_father(table, table->pages[pageHeader_old->father_page], new_key,
                                            pageHeader_old->page_num, pageHeader_new->page_num);
            set_left_right_father(table,left_page_num,right_page_num,pageHeader_old->page_num,pageHeader_new->page_num);
        }
    }
}




uint32_t create_new_root(Table* table,uint32_t left_page_num,uint32_t right_page_num,uint32_t new_key) {

    uint32_t root_page_num = get_unused_page(table);

    table->root_page_num = root_page_num;
    void* root_page = table->pages[root_page_num];

    initialize_root(root_page,get_internal(),root_page_num);

    void* cell_key = get_n_key_ptr_internal(root_page,0);
    *((uint32_t*) cell_key)=new_key;
    *((uint32_t*)(cell_key-POINTER_SIZE))=left_page_num;
    *((uint32_t*)(cell_key+KEY_SIZE))=right_page_num;

    Page_header* pageHeader = (Page_header*) root_page;
    pageHeader->n_keys=1;
    pageHeader->is_root=1;

    return root_page_num;
}

InsertResult split_leaf(Row *row, Table *table, void *old_page, uint32_t pos) {
    uint32_t idx_split = get_unused_page(table);
    void *new_page = table->pages[idx_split];
    initialize_leaf_node(new_page, idx_split);


    for (uint32_t i = LEAF_MAX_ROWS; i >= 0; i--) {
        void *destination_node;
        if (i >= LEAF_LEFT_SPLIT) {
            destination_node = new_page;
        } else {
            destination_node = old_page;
        }
        uint32_t idx = i % LEAF_LEFT_SPLIT;
        void *destination = get_n_row_ptr_leaf(destination_node, idx);
        if (i == pos) {
            serialize_row(row, destination);
        } else if (i > pos) {
            memcpy(destination, get_n_row_ptr_leaf(old_page, i - 1), ROW_SIZE);
        } else {
            memcpy(destination, get_n_row_ptr_leaf(old_page, i), ROW_SIZE);
        }
        if (i == 0) {
            break;
        }
    }

    Page_header *pageHeader_old = (Page_header *) old_page;
    Page_header *pageHeader_new = (Page_header *) new_page;

    pageHeader_old->n_keys = LEAF_LEFT_SPLIT;
    pageHeader_new->n_keys = LEAF_RIGHT_SPLIT;

    uint32_t new_key = ((Row*)get_n_row_ptr_leaf(old_page, pageHeader_old->n_keys - 1))->id;

    if (pageHeader_old->is_root) {
        uint32_t root_page = create_new_root(table, pageHeader_old->page_num, pageHeader_new->page_num, new_key);
        pageHeader_old->father_page = root_page;
        pageHeader_new->father_page = root_page;
        pageHeader_old->is_root=0;
        return INSERT_SUCCESS;
    }

    else {
        add_to_father(table,table->pages[pageHeader_old->father_page],new_key,pageHeader_old->page_num,pageHeader_new->page_num);
        return INSERT_SUCCESS;
    }

}






#ifndef MYCDBV3_SPLIT_H
#define MYCDBV3_SPLIT_H

#endif //MYCDBV3_SPLIT_H
