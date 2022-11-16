#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "common.h"
// 0 leaf
// 1 internal

const uint32_t HEADER_ELEMENT_SIZE = sizeof(uint32_t);
const uint32_t HEADER_N_ELEMENTS = 5;
const uint32_t HEADER_SIZE = HEADER_ELEMENT_SIZE*HEADER_N_ELEMENTS;
const uint32_t POINTER_SIZE = sizeof(uint32_t);
const uint32_t KEY_SIZE = sizeof(uint32_t);


typedef struct {
    uint32_t page_num;
    uint32_t node_type;
    uint32_t is_root;
    uint32_t n_keys;
    uint32_t father_page;
} Page_header;

typedef struct {
   uint32_t left_page;
   uint32_t key_value;
   uint32_t right_page;
} Key_internal;


// common header

const uint32_t  HEADER_PAGE_NUM_OFFSET = 0;
const uint32_t  HEADER_NODE_TYPE_OFFSET = HEADER_ELEMENT_SIZE;
const uint32_t  HEADER_IS_ROOT_OFFSET = HEADER_NODE_TYPE_OFFSET + HEADER_ELEMENT_SIZE;
const uint32_t  HEADER_N_KEYS_OFFSET = HEADER_IS_ROOT_OFFSET + HEADER_ELEMENT_SIZE;
const uint32_t  HEADER_FATHER_PAGE_OFFSET = HEADER_N_KEYS_OFFSET + HEADER_ELEMENT_SIZE;


// body

const uint32_t LEAF_BODY_SIZE = PAGE_SIZE - HEADER_SIZE;
const uint32_t INTERNAL_BODY_SIZE = PAGE_SIZE-HEADER_SIZE;


// num cells leaf

const uint32_t LEAF_MAX_ROWS = LEAF_BODY_SIZE/ROW_SIZE;
const uint32_t INTERNAL_MAX_KEYS = (INTERNAL_BODY_SIZE-POINTER_SIZE)/(KEY_SIZE+POINTER_SIZE);



// left right split
const uint32_t LEAF_RIGHT_SPLIT = (LEAF_MAX_ROWS+1)/2;
const uint32_t LEAF_LEFT_SPLIT = (LEAF_MAX_ROWS+1)-LEAF_RIGHT_SPLIT;

// split internal

const uint32_t INTERNAL_RIGHT_SPLIT = (INTERNAL_MAX_KEYS+1)/2;
const uint32_t INTERNAl_LEFT_SPLIT = (INTERNAL_MAX_KEYS+1)-LEAF_RIGHT_SPLIT;




void deserialize_header(void* source, Page_header* destination) {
    memcpy(&(destination->page_num),source+HEADER_PAGE_NUM_OFFSET,HEADER_ELEMENT_SIZE);
    memcpy(&(destination->node_type),source+HEADER_NODE_TYPE_OFFSET,HEADER_ELEMENT_SIZE);
    memcpy(&(destination->is_root),source+HEADER_IS_ROOT_OFFSET,HEADER_ELEMENT_SIZE);
    memcpy(&(destination->n_keys),source+HEADER_N_KEYS_OFFSET,HEADER_ELEMENT_SIZE);
    memcpy(&(destination->father_page),source+HEADER_FATHER_PAGE_OFFSET,HEADER_ELEMENT_SIZE);
}

void serialize_header(Page_header* pageHeader, void* destination) {
    memcpy(destination+HEADER_PAGE_NUM_OFFSET,&(pageHeader->page_num),HEADER_ELEMENT_SIZE);
    memcpy(destination+HEADER_NODE_TYPE_OFFSET,&(pageHeader->node_type),HEADER_ELEMENT_SIZE);
    memcpy(destination+HEADER_IS_ROOT_OFFSET,&(pageHeader->is_root),HEADER_ELEMENT_SIZE);
    memcpy(destination+HEADER_N_KEYS_OFFSET,&(pageHeader->n_keys),HEADER_ELEMENT_SIZE);
    memcpy(destination+HEADER_FATHER_PAGE_OFFSET,&(pageHeader->father_page),HEADER_ELEMENT_SIZE);

}


void* get_n_row_ptr_leaf(void* page, uint32_t idx) {
    return page+HEADER_SIZE+(ROW_SIZE*idx);
}

void* get_n_key_ptr_internal(void* page, uint32_t idx) {
    return page+HEADER_SIZE+POINTER_SIZE+(KEY_SIZE+POINTER_SIZE)*idx;
}



void initialize_root(void* page, uint32_t node_type,uint32_t idx) {
    Page_header pageHeader;
    pageHeader.page_num=idx;
    pageHeader.is_root=1;
    pageHeader.n_keys=0;
    pageHeader.node_type=node_type;
    pageHeader.father_page=NULL_FATHER;
    serialize_header(&(pageHeader),page);
}

void initialize_leaf_node(void* page, uint32_t idx) {
    Page_header  pageHeader;
    pageHeader.page_num = idx;
    pageHeader.is_root=0;
    pageHeader.n_keys=0;
    pageHeader.node_type=0;
    pageHeader.father_page=NULL_FATHER;

    serialize_header(&(pageHeader),page);

}

void print_header(void* page) {
    Page_header pageHeader;
    deserialize_header(page,&(pageHeader));
    printf("Header : \n");
    printf("page num : %d\n",pageHeader.page_num);
    printf("is root : %d\n",pageHeader.is_root);
    printf("node type : %d\n",pageHeader.node_type);
    printf("num keys : %d\n",pageHeader.n_keys);
    printf("father page num : %d\n",pageHeader.father_page);
}


void print_page_leaf(void* page) {
    Page_header* pageHeader = (Page_header*) page;
    print_header(page);

    for(uint32_t i = 0;i<pageHeader->n_keys;i++) {
        Row* row = ((Row*) get_n_row_ptr_leaf(page,i));
        print_row(row);
    }
}

void print_page_internal(void* page) {
    Page_header* pageHeader = (Page_header*) page;
    print_header(page);

    for(uint32_t i = 0;i <pageHeader->n_keys;i++) {

        void* cell = get_n_key_ptr_internal(page,i);
        void* left = cell-POINTER_SIZE;
        void* right = cell+KEY_SIZE;
        printf("left: %d value : %d right: %d\n",*((uint32_t*)left),*((uint32_t*)cell),*((uint32_t*)right));
    }

}

uint32_t binary_search_internal(void* page, uint32_t key) {
    uint32_t low = 0;
    uint32_t high = ((Page_header*) page)->n_keys;

    while(low!=high) {
        uint32_t mid = (low+high)/2;
        void* cell_mid = get_n_key_ptr_internal(page,mid);
        uint32_t mid_value =*((uint32_t*) cell_mid);
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

void print_b_tree(void* page, Table* table) {
    Page_header* pageHeader = (Page_header*) page;
    if(pageHeader->node_type==get_internal()) {
        for(uint32_t i = 0; i<pageHeader->n_keys;++i) {
            void* cell = get_n_key_ptr_internal(page,i);
            Key_internal* keyInternal  = (Key_internal*) (cell-POINTER_SIZE);
            uint32_t  key = keyInternal->key_value;
            uint32_t  left = keyInternal->left_page;
            uint32_t  right = keyInternal->right_page;
            printf("left: %d value: %d right: %d\n",left,key,right);
            print_b_tree(table->pages[left],table);
            if(i==pageHeader->n_keys-1) {
                print_b_tree(table->pages[right],table);
            }
        }
    }
    else if(pageHeader->node_type==get_leaf()) {
        for(uint32_t i = 0; i<pageHeader->n_keys;++i) {
            void* cell = get_n_row_ptr_leaf(page,i);
            printf("%d : ",pageHeader->page_num);
            print_row((Row*) cell);
        }
    }
    else {
        printf("Error when printing btree, bad format\n");
    }

}



void initialize_internal_node(void* page, uint32_t page_num) {
    Page_header* pageHeader = (Page_header*) page;
    pageHeader->n_keys=0;
    pageHeader->node_type=get_internal();
    pageHeader->page_num=page_num;
    pageHeader->father_page=NULL_FATHER;
    pageHeader->is_root=0;

}

void serialize_key(Key_internal keyInternal, void* destination) {
    Key_internal* keyInternal_dst = (Key_internal*) destination;
    keyInternal_dst->key_value=keyInternal.key_value;
    keyInternal_dst->left_page=keyInternal.left_page;
    keyInternal_dst->right_page=keyInternal.right_page;
}
#ifndef MYCDBV3_NODES_H
#define MYCDBV3_NODES_H

#endif //MYCDBV3_NODES_H
