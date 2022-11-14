#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


typedef struct {
    char *buffer;
    size_t buffer_size;
    ssize_t input_size;
} Read_buffer;


Read_buffer *new_read_buffer() {
    Read_buffer *read_buffer = (Read_buffer *) malloc(sizeof(Read_buffer));
    read_buffer->buffer = NULL;
    read_buffer->buffer_size = 0;
    read_buffer->input_size = 0;

    return read_buffer;
}
void close_buffer(Read_buffer *read_buffer) {
    free(read_buffer->buffer);
    free(read_buffer);
}

void read_input(Read_buffer *read_buffer) {
    ssize_t bytesRead =
            getline(&(read_buffer->buffer), &(read_buffer->buffer_size), stdin);
    if (bytesRead <= 0) {
        printf("Error when reading.\n");
        exit(EXIT_FAILURE);
    }
    read_buffer->input_size = bytesRead - 1;
    read_buffer->buffer[bytesRead - 1] = 0;
}



#ifndef MYCDBV3_READ_H
#define MYCDBV3_READ_H

#endif //MYCDBV3_READ_H
