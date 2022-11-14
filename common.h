#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


# define USER_NAME 32
# define EMAIL 255
# define TABLE_MAX_PAGES 100

#define NULL_FATHER 2147483647

typedef enum {
    INSERT_FAIL_TABLE_FULL,
    INSERT_SUCCESS,
    INSERT_DUPLICATED_KEY
} InsertResult;

typedef struct {
    uint32_t n_pages;
    uint32_t root_page_num;
    void* pages[TABLE_MAX_PAGES];
} Table;

typedef struct {
    uint32_t id;
    char username[USER_NAME + 1];
    char email[EMAIL + 1];
} Row;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

//const uint32_t PAGE_SIZE = 4096;
const uint32_t PAGE_SIZE = 700;

void print_row(Row *row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}


void serialize_row(Row *source, void *destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    strncpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    strncpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

uint32_t  get_row_id(void* source) {
    return *(uint32_t*) source;
}

uint32_t get_unused_page(Table* table) {
    for(uint32_t i = 0; i<TABLE_MAX_PAGES;i++) {
        if(table->pages[i]==NULL) {
            table->pages[i] = malloc(PAGE_SIZE);
            return i;
        }
    }
}

uint32_t  get_leaf() {
    return 0;
}
uint32_t  get_internal() {
    return 1;
}
#ifndef MYCDBV3_COMMON_H
#define MYCDBV3_COMMON_H

#endif //MYCDBV3_COMMON_H
