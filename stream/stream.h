#ifndef _STREAM_H_
#define _STREAM_H_

#include <stdint.h>

typedef union  stream_data stream_data_t;
typedef struct stream_plug stream_plug_t;
typedef struct stream stream_t;

typedef int (*stream_write_t) (stream_t *, const void *, int);
typedef int (*stream_read_t)  (stream_t *, void *, int);
typedef int (*stream_seek_t)  (stream_t *, uint64_t);
typedef int (*stream_flush_t) (stream_t *);
typedef int (*stream_close_t) (stream_t *);

union stream_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
};

struct stream_plug {
    stream_read_t  read;
    stream_write_t write;
    stream_seek_t  seek;
    stream_flush_t flush;
    stream_close_t close;
};

struct stream {
    stream_plug_t *plug;
    stream_data_t  data;
};

extern stream_plug_t    stream_file_plug;

int stream_with_fd      (stream_t *stream, int fd);

int stream_file_open    (stream_t *stream, const char *path, int flags);
int stream_file_create  (stream_t *stream, const char *path, int mode);

int stream_seek         (stream_t *stream, uint64_t offset);
int stream_close        (stream_t *stream);

int stream_write        (stream_t *stream, const void *buf, int n);
int stream_read         (stream_t *stream, void *buf, int n);

int stream_write_int8   (stream_t *stream, int8_t v);
int stream_read_int8    (stream_t *stream, int8_t *v);

int stream_write_uint8  (stream_t *stream, uint8_t v);
int stream_read_uint8   (stream_t *stream, uint8_t *v);

int stream_write_int16  (stream_t *stream, int16_t v);
int stream_read_int16   (stream_t *stream, int16_t *v);

int stream_write_uint16 (stream_t *stream, uint16_t v);
int stream_read_uint16  (stream_t *stream, uint16_t *v);

int stream_write_int32  (stream_t *stream, int32_t v);
int stream_read_int32   (stream_t *stream, int32_t *v);

int stream_write_uint32 (stream_t *stream, uint32_t v);
int stream_read_uint32  (stream_t *stream, uint32_t *v);

int stream_write_int64  (stream_t *stream, int64_t v);
int stream_read_int64   (stream_t *stream, int64_t *v);

int stream_write_uint64 (stream_t *stream, uint64_t v);
int stream_read_uint64  (stream_t *stream, uint64_t *v);

#endif /* !_STREAM_ */

