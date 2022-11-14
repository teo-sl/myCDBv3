#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "read.h"

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_FAIL,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_OUT_OF_BOUND,
    PREPARE_NEGATIVE_ID
} StatementResult;

typedef enum {
    META_FAIl,
    META_SUCCESS,
    META_CLOSE
} Meta_result;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef struct {
    StatementType statement_type;
    Row row;
} Statement;

Meta_result prepare_meta_command(Read_buffer *read_buffer, Table *table) {
    if (strcmp(read_buffer->buffer, ".exit") == 0) {
        close_buffer(read_buffer);
        return META_CLOSE;
    } else {
        return META_FAIl;
    }
}

StatementResult prepare_statement(Read_buffer *read_buffer, Statement *statement) {
    if (strncmp(read_buffer->buffer, "insert", 6) == 0) {
        statement->statement_type = STATEMENT_INSERT;
        strtok(read_buffer->buffer, " ");
        char *idString = strtok(NULL, " ");
        char *username = strtok(NULL, " ");
        char *email = strtok(NULL, " ");

        if (strtok(NULL, " ") != NULL) {
            return PREPARE_SYNTAX_ERROR;
        }
        if (idString == NULL || username == NULL || email == NULL) {
            return PREPARE_SYNTAX_ERROR;
        }

        int id = atoi(idString);
        if (id < 0) {
            return PREPARE_NEGATIVE_ID;
        }

        if (strlen(username) > USERNAME_SIZE || strlen(email) > EMAIL_SIZE) {
            return PREPARE_STRING_OUT_OF_BOUND;
        }

        statement->row.id = id;
        strcpy(statement->row.username, username);
        strcpy(statement->row.email, email);
        return PREPARE_SUCCESS;
    }
    if (strcmp(read_buffer->buffer, "select") == 0) {
        statement->statement_type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    } else {
        return PREPARE_FAIL;
    }
}
#ifndef MYCDBV3_META_H
#define MYCDBV3_META_H

#endif //MYCDBV3_META_H
