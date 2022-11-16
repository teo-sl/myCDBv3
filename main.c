#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "nodes.h"
#include "util.h"






Table* new_db() {
    Table* table = malloc(sizeof(Table));
    table->n_pages=1;
    table->root_page_num=0;

    for(uint32_t i = 1; i<TABLE_MAX_PAGES; ++i) {
        table->pages[i]=NULL;
    }

    table->pages[0] = malloc(PAGE_SIZE);

    initialize_root(table->pages[0],get_leaf(),0); //set root as leaf

    return table;
}

void db_close(Table* table) {

    for(uint32_t i = 0;i<TABLE_MAX_PAGES;++i) {
        if(table->pages[i]) {
            free(table->pages[i]);
            table->pages[i]=NULL;
        }
    }
    free(table);
}





int main() {
    Table *table = new_db();
    Read_buffer *read_buffer = new_read_buffer();
    while (true) {
        printf("myDB > ");
        read_input(read_buffer);
        if ((read_buffer->buffer[0] == '.')) {
            switch (prepare_meta_command(read_buffer, table)) {
                case META_CLOSE:
                    db_close(table);
                    exit(EXIT_SUCCESS);
                case META_SUCCESS:
                    continue;
                case META_FAIl:
                    printf("Unrecognized command \"%s\". \n", read_buffer->buffer);
                    continue;
            }
        }
        Statement statement;
        bool success = true;
        switch (prepare_statement(read_buffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_FAIL:
                printf("Unrecognized statement \"%s.\" \n", read_buffer->buffer);
                success = false;
                continue;
            case PREPARE_SYNTAX_ERROR:
                printf("Error. Syntax error\n");
                success = false;
                break;
            case PREPARE_STRING_OUT_OF_BOUND:
                printf("Error. Strings too long\n");
                success = false;
                break;
            case PREPARE_NEGATIVE_ID:
                printf("Error. Negative id\n");
                success = false;
        }
        if (!success) {
            printf("Not executed\n");
            continue;
        }
        execute_statement(&statement, table);
        printf("Executed.\n");
    }
}