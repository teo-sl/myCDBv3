#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


uint32_t add_to_father(Table* table, void* page, uint32_t key, uint32_t left_page_num, uint32_t right_page_num) {
    uint32_t  idx = binary_search_internal(page,key);
    Page_header* pageHeader = (Page_header*) page;
    uint32_t size = pageHeader->n_keys;

    if(size<INTERNAL_MAX_KEYS) { // add directly
        uint32_t keys_to_move = size-idx;
        void* cell_dst = get_n_key_ptr_internal(page,idx);
        Key_internal* keyInternal = (Key_internal*) (cell_dst-POINTER_SIZE);
        Key_internal* keyInternal1 = (Key_internal*) cell_dst+KEY_SIZE;
        memcpy((get_n_key_ptr_internal(page,idx)+KEY_SIZE+POINTER_SIZE),cell_dst,keys_to_move*(KEY_SIZE+POINTER_SIZE));
        keyInternal->left_page=left_page_num;
        keyInternal->right_page=right_page_num;
        keyInternal->key_value=key;
        uint32_t new_size = size+1;
        pageHeader->n_keys=new_size;
        return pageHeader->page_num;
    }
    else {

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
        uint32_t  father_page = add_to_father(table,table->pages[pageHeader_old->father_page],new_key,pageHeader_old->page_num,pageHeader_new->page_num);
        pageHeader_old->father_page=father_page;
        pageHeader_new->father_page=father_page;
        return INSERT_SUCCESS;
    }

}






#ifndef MYCDBV3_SPLIT_H
#define MYCDBV3_SPLIT_H

#endif //MYCDBV3_SPLIT_H
