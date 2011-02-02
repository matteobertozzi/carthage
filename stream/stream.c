#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "stream.h"

int stream_file_seek (stream_t *stream, uint64_t offset) {
    if (lseek(stream->data.fd, offset, SEEK_SET) != offset)
        return(-1);
    return(0);
}

int stream_file_flush (stream_t *stream) {
    if (fsync(stream->data.fd) < 0)
        return(-1);
    return(0);
}

int stream_seek (stream_t *stream, uint64_t offset) {
    return(stream->plug->seek(stream, offset));
}

int stream_close (stream_t *stream) {
    return(stream->plug->close(stream));
}

int stream_write (stream_t *stream, const void *buf, int n) {
    return(stream->plug->write(stream, buf, n));
}

int stream_read (stream_t *stream, void *buf, int n) {
    return(stream->plug->read(stream, buf, n));
}

int stream_write_int8 (stream_t *stream, int8_t v) {
    return(stream->plug->write(stream, &v, 1));
}

int stream_read_int8 (stream_t *stream, int8_t *v) {
    if (stream->plug->read(stream, v, 1) != 1)
        return(-1);
    return(0);
}

int stream_write_uint8 (stream_t *stream, uint8_t v) {
    return(stream->plug->write(stream, &v, 1));
}

int stream_read_uint8 (stream_t *stream, uint8_t *v) {
    if (stream->plug->read(stream, v, 1) != 1)
        return(-1);
    return(0);
}


int stream_write_int16 (stream_t *stream, int16_t v) {
    uint8_t buffer[2];

    buffer[0] = (v >> 8);
    buffer[1] = (v >> 0);

    return(stream->plug->write(stream, buffer, 2));
}

int stream_read_int16 (stream_t *stream, int16_t *v) {
    uint8_t buffer[2];
    int16_t v16;

    if (stream->plug->read(stream, buffer, 2) != 2)
        return(-1);

    v16  = ((uint16_t)(buffer[0]) << 8);
    v16 += ((uint16_t)(buffer[1]) << 0);
    *v = v16;

    return(0);
}

int stream_write_uint16 (stream_t *stream, uint16_t v) {
    uint8_t buffer[2];

    buffer[0] = (v >> 8);
    buffer[1] = (v >> 0);

    return(stream->plug->write(stream, buffer, 2));
}

int stream_read_uint16 (stream_t *stream, uint16_t *v) {
    uint8_t buffer[2];
    uint16_t v16;

    if (stream->plug->read(stream, buffer, 2) != 2)
        return(-1);

    v16  = ((uint16_t)(buffer[0]) << 8);
    v16 += ((uint16_t)(buffer[1]) << 0);
    *v = v16;

    return(0);
}


int stream_write_int32 (stream_t *stream, int32_t v) {
    uint8_t buffer[4];

    buffer[0] = (v >> 24);
    buffer[1] = (v >> 16);
    buffer[2] = (v >>  8);
    buffer[3] = (v >>  0);

    return(stream->plug->write(stream, buffer, 4));
}

int stream_read_int32 (stream_t *stream, int32_t *v) {
    uint8_t buffer[4];
    int32_t v32;

    if (stream->plug->read(stream, buffer, 4) != 4)
        return(-1);

    v32  = ((uint32_t)(buffer[0]) << 24);
    v32 += ((uint32_t)(buffer[1]) << 16);
    v32 += ((uint32_t)(buffer[2]) <<  8);
    v32 += ((uint32_t)(buffer[3]) <<  0);
    *v = v32;

    return(0);
}

int stream_write_uint32 (stream_t *stream, uint32_t v) {
    uint8_t buffer[4];

    buffer[0] = (v >> 24);
    buffer[1] = (v >> 16);
    buffer[2] = (v >>  8);
    buffer[3] = (v >>  0);

    return(stream->plug->write(stream, buffer, 4));
}

int stream_read_uint32 (stream_t *stream, uint32_t *v) {
    uint8_t buffer[4];
    uint32_t v32;

    if (stream->plug->read(stream, buffer, 4) != 4)
        return(-1);

    v32  = ((uint32_t)(buffer[0]) << 24);
    v32 += ((uint32_t)(buffer[1]) << 16);
    v32 += ((uint32_t)(buffer[2]) <<  8);
    v32 += ((uint32_t)(buffer[3]) <<  0);
    *v = v32;

    return(0);
}


int stream_write_int64 (stream_t *stream, int64_t v) {
    int8_t buffer[8];

    buffer[0] = (v >> 56);
    buffer[1] = (v >> 48);
    buffer[2] = (v >> 40);
    buffer[3] = (v >> 32);
    buffer[4] = (v >> 24);
    buffer[5] = (v >> 16);
    buffer[6] = (v >>  8);
    buffer[7] = (v >>  0);

    return(stream->plug->write(stream, buffer, 8));
}

int stream_read_int64 (stream_t *stream, int64_t *v) {
    uint8_t buffer[8];
    int64_t v64;

    if (stream->plug->read(stream, buffer, 8) != 8)
        return(-1);

    v64  = ((uint64_t)(buffer[0]) << 56);
    v64 += ((uint64_t)(buffer[1]) << 48);
    v64 += ((uint64_t)(buffer[2]) << 40);
    v64 += ((uint64_t)(buffer[3]) << 32);
    v64 += ((uint64_t)(buffer[4]) << 24);
    v64 += ((uint64_t)(buffer[5]) << 16);
    v64 += ((uint64_t)(buffer[6]) <<  8);
    v64 += ((uint64_t)(buffer[7]) <<  0);
    *v = v64;

    return(0);
}

int stream_write_uint64 (stream_t *stream, uint64_t v) {
    uint8_t buffer[8];

    buffer[0] = (v >> 56);
    buffer[1] = (v >> 48);
    buffer[2] = (v >> 40);
    buffer[3] = (v >> 32);
    buffer[4] = (v >> 24);
    buffer[5] = (v >> 16);
    buffer[6] = (v >>  8);
    buffer[7] = (v >>  0);

    return(stream->plug->write(stream, buffer, 8));
}

int stream_read_uint64 (stream_t *stream, uint64_t *v) {
    uint8_t buffer[8];
    uint64_t v64;

    if (stream->plug->read(stream, buffer, 8) != 8)
        return(-1);

    v64  = ((uint64_t)(buffer[0]) << 56);
    v64 += ((uint64_t)(buffer[1]) << 48);
    v64 += ((uint64_t)(buffer[2]) << 40);
    v64 += ((uint64_t)(buffer[3]) << 32);
    v64 += ((uint64_t)(buffer[4]) << 24);
    v64 += ((uint64_t)(buffer[5]) << 16);
    v64 += ((uint64_t)(buffer[6]) <<  8);
    v64 += ((uint64_t)(buffer[7]) <<  0);
    *v = v64;

    return(0);
}

/*
 * File Stream
 */
int stream_with_fd (stream_t *stream, int fd) {
    stream->plug = &stream_file_plug;
    stream->data.fd = fd;
    return(0);
}

int stream_file_open (stream_t *stream, const char *path, int flags) {
    stream->plug = &stream_file_plug;
    if ((stream->data.fd = open(path, flags)) < 0)
        return(-1);
    return(0);
}

int stream_file_create (stream_t *stream, const char *path, int mode) {
    stream->plug = &stream_file_plug;
    if ((stream->data.fd = open(path, O_CREAT | O_TRUNC | O_RDWR, mode)) < 0)
        return(-1);
    return(0);
}

int stream_file_close (stream_t *stream) {
    close(stream->data.fd);
    return(0);
}

int stream_file_read (stream_t *stream, void *buffer, int n) {
    uint8_t *p = (uint8_t *)buffer;
    int rdsize = 0;
    ssize_t rd;

    while (rdsize < n) {
        if ((rd = read(stream->data.fd, p, n)) <= 0)
            break;

        rdsize += rd;
        p += rd;
    }

    return(rdsize);
}

int stream_file_write (stream_t *stream, const void *buffer, int n) {
    uint8_t *p = (uint8_t *)buffer;
    int wrsize = 0;
    ssize_t wr;

    while (wrsize < n) {
        if ((wr = write(stream->data.fd, p, n)) <= 0)
            break;

        wrsize += wr;
        p += wr;
    }

    return(wrsize);
}

stream_plug_t stream_file_plug = {
    .read  = stream_file_read,
    .write = stream_file_write,
    .seek  = stream_file_seek,
    .flush = stream_file_flush,
    .close = stream_file_close,
};


