#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "stream.h"

#define BUF16_DATA              "abcdefghijklmnop"

int main (int argc, char **argv) {
    stream_t stream;
    uint8_t buf[16];
    uint64_t u64;
    uint32_t u32;
    uint16_t u16;
    int64_t i64;
    int32_t i32;
    int16_t i16;
    uint8_t u8;
    int8_t i8;

    stream_file_create(&stream, "test.stream", 0644);

    stream_write_int8(&stream, -123);
    stream_write_uint8(&stream, 213);

    stream_write_int16(&stream, -32563);
    stream_write_uint16(&stream, 65532);

    stream_write_int32(&stream, -2147423641L);
    stream_write_uint32(&stream, 4294965296L);

    stream_write_int64(&stream, -9223372031854775808LL);
    stream_write_uint64(&stream, 18446744073709451616ULL);

    memcpy(buf, "abcdefghijklmnop", 16);
    stream_write(&stream, buf, 16);

    stream_seek(&stream, 0);

    stream_read_int8(&stream, &i8);
    assert(i8 == -123);

    stream_read_uint8(&stream, &u8);
    assert(u8 == 213);

    stream_read_int16(&stream, &i16);
    assert(i16 == -32563);

    stream_read_uint16(&stream, &u16);
    assert(u16 == 65532);

    stream_read_int32(&stream, &i32);
    assert(i32 == -2147423641L);

    stream_read_uint32(&stream, &u32);
    assert(u32 == 4294965296L);

    stream_read_int64(&stream, &i64);
    assert(i64 == -9223372031854775808LL);

    stream_read_uint64(&stream, &u64);
    assert(u64 == 18446744073709451616ULL);

    memset(buf, 0, 16);
    stream_read(&stream, buf, 16);
    assert(!memcmp(buf, BUF16_DATA, 16));

    stream_close(&stream);

    return(0);
}

