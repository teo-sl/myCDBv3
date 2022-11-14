#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "meta.h"
#include "split.h"



//

uint32_t binary_search_leaf(void* page, uint32_t key) {
    uint32_t low = 0;
    uint32_t high = ((Page_header*) page)->n_keys;

    while(low!=high) {
        uint32_t mid = (low+high)/2;
        void* cell = get_n_row_ptr_leaf(page,mid);
        uint32_t mid_value = ((Row*)cell)->id;
        if(key==mid_value) {
            return mid;
        }
        else if(key>mid_value) {
            low=mid+1;
        }
        else {
            high=mid;
        }
    }
    return low;
}

uint32_t binary_search_internal(void* page, uint32_t key) {
    uint32_t low = 0;
    uint32_t high = ((Page_header*) page)->n_keys;

    while(low!=high) {
        uint32_t mid = (low+high)/2;
        void* cell_mid = get_n_key_ptr_internal(page,mid);
        uint32_t mid_value =((uint32_t*) cell_mid);
        if(key==mid_value) {
            return mid;
        }
        else if(key>mid_value) {
            low=mid+1;
        }
        else {
            high=mid;
        }
    }
    return low;
}

void* find_page_with_key(uint32_t key, Table* table, void* page) {
    Page_header*  pageHeader = ((Page_header*) page);
    uint32_t node_type = pageHeader->node_type;


    if((node_type==get_leaf())) {
        return page;
    }
    uint32_t key_idx = binary_search_internal(page,key);
    uint32_t size=pageHeader->n_keys;
    if(key_idx>=size) {
        void* cell = get_n_key_ptr_internal(page,key_idx-1);
        uint32_t  page_son = *((uint32_t*)cell+ROW_SIZE);
        return find_page_with_key(key,table,table->pages[page_son]);
    }
    void* cell = get_n_key_ptr_internal(page,key_idx);
    uint32_t page_son = *((uint32_t*)cell-POINTER_SIZE);
    return find_page_with_key(key,table,table->pages[page_son]);
}




InsertResult execute_insert(Row* row, Table* table) {
    void* page = find_page_with_key(row->id,table,table->pages[table->root_page_num]);
    Page_header* pageHeader = (Page_header*) page;

    uint32_t  idx = binary_search_leaf(page,row->id);
    Row* cell_row_idx = (Row*) get_n_row_ptr_leaf(page,idx);
    uint32_t key_idx = get_row_id(cell_row_idx);

    if(key_idx==row->id) {
        return INSERT_DUPLICATED_KEY;
    }

    uint32_t size = pageHeader->n_keys;

    if(size>=LEAF_MAX_ROWS) {
        return split_leaf(row,table,page,idx);
    }

    uint32_t n_rows_to_move = size-idx;

    memcpy((get_n_row_ptr_leaf(page,idx)+ROW_SIZE),cell_row_idx,(ROW_SIZE*n_rows_to_move));

    serialize_row(row,cell_row_idx);



    uint32_t new_size = size+1;
    pageHeader->n_keys=new_size;




    return INSERT_SUCCESS;
}


void execute_select(Table* table, void* page) {
    Page_header* pageHeader = (Page_header*) page;
    uint32_t  node_type = pageHeader->node_type;

    uint32_t  size = pageHeader->n_keys;

    if(node_type==1) { // is internal
        for(uint32_t i = 0;i<size;i++) {
            void* cell = get_n_key_ptr_internal(page,i);
            uint32_t left_page_num = *((uint32_t*) (cell-POINTER_SIZE));
            execute_select(table,table->pages[left_page_num]);
        }
        void* cell = get_n_key_ptr_internal(page,size-1);
        uint32_t right_page_num = *((uint32_t*) (cell+KEY_SIZE));
        execute_select(table,table->pages[right_page_num]);
    }
    else {
        for(uint32_t i = 0; i<size;++i) {
            void* cell = get_n_row_ptr_leaf(page,i);
            print_row((Row*)cell);
        }
    }
}


void execute_statement(Statement *statement, Table *table) {
    switch (statement->statement_type) {
        case STATEMENT_INSERT:
            switch (execute_insert(&(statement->row), table)) {
                case INSERT_FAIL_TABLE_FULL:
                    printf("Table full \n");
                    break;
                case INSERT_DUPLICATED_KEY:
                    printf("Id duplicated\n");
                    break;
                case INSERT_SUCCESS:
                    break;
            }
            break;
        case STATEMENT_SELECT:
            execute_select(table,table->pages[table->root_page_num]);
            break;
    }
}

#ifndef MYCDBV3_UTIL_H
#define MYCDBV3_UTIL_H

#endif //MYCDBV3_UTIL_H
