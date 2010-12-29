#define _XOPEN_SOURCE 500
//#define __BTREE_DEBUG
#include <sys/time.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "btree.h"

#if 1
    #define __CACHESZ       (4)
    #define __BLOCKSZ       256
    #define __KEYSZ         (10)
    #define __NKEYS         64
    #define __VALUESZ       (32)
#else
    #define __CACHESZ       (128)
    #define __BLOCKSZ       (512 << 10)
    #define __KEYSZ         (10)
    #define __NKEYS         (1 << 20)
    #define __VALUESZ       (64)
#endif

#define TEST_WRITE      1

struct btdisk_data {
    uint64_t offset;
    int fd;
};

static uint64_t time_micros (void) {
    struct timeval now;
    gettimeofday(&now, NULL);
    return(now.tv_sec * 1000000U + now.tv_usec);
}

static int __keycmp (void *user_data,
                     const void *a,
                     const void *b)
{
    return(memcmp(a, b, __KEYSZ));
}

#ifdef __BTREE_DEBUG
static void __data_debug (void *user_data, const void *body, uint32_t size) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s", (const char *)body);
    buffer[size] = '\0';
    printf("%s", buffer);
}

static void __key_debug (void *user_data, const void *key) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s", (const char *)key);
    buffer[__KEYSZ] = '\0';
    printf("%s", buffer);
}
#endif

static uint64_t __btdisk_append (btdisk_t *disk,
                               const void *data,
                               uint32_t size)
{
    struct btdisk_data *dd = (struct btdisk_data *)disk->internal;
    uint64_t offset = dd->offset;
    ssize_t wr;

    //printf("__btdisk_append: %u - %"PRIu64"\n", size, offset);
    if ((wr = pwrite(dd->fd, data, size, offset)) != size) {
        perror("pwrite()");
        printf(" - Return Code %ld offset %"PRIu64" size %u\n",
               wr, offset, size);
    }
    dd->offset += size;

    return(offset);
}

static uint64_t __btdisk_write (btdisk_t *disk,
                              uint64_t block_offset,
                              uint32_t block_size,
                              const void *data,
                              uint32_t size)
{
    struct btdisk_data *dd = (struct btdisk_data *)disk->internal;
    ssize_t wr;

    if (block_offset == 0)
        block_offset = dd->offset;

    printf("__btdisk_write: %u - %"PRIu64"\n", size, block_offset);
    if ((wr = pwrite(dd->fd, data, size, block_offset)) != size) {
        perror("pwrite()");
        printf(" - Return Code %ld offset %"PRIu64" size %u\n",
               wr, block_offset, size);
    }
    dd->offset += size;

    return(block_offset);
}

static uint32_t __btdisk_read (btdisk_t *disk,
                             uint64_t offset,
                             void *buffer,
                             uint32_t size)
{
    struct btdisk_data *dd = (struct btdisk_data *)disk->internal;
    ssize_t rd;

    printf("__btdisk_read: %u - %"PRIu64"\n", size, offset);
    if ((rd = pread(dd->fd, buffer, size, offset)) != size) {
        perror("pread()");
        printf(" - Return Code %ld offset %"PRIu64" size %u\n",
               rd, offset, size);
    }

    return(size);
}

static uint64_t __btdisk_erase (btdisk_t *disk,
                              uint64_t offset,
                              uint32_t size)
{
    printf("__btdisk_erase: %u - %"PRIu64"\n", size, offset);
    return(offset);
}

uint32_t __btdisk_addler32 (btdisk_t *disk,
                          const void *data,
                          uint32_t n)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t s2 = (0U >> 16) & 0xffff;
    uint32_t s1 = 0U & 0xffff;

    while (n--) {
        s1 = (s1 + *p++) % 65521;
        s2 = (s2 + s1) % 65521;
    }

    return((s2 << 16) + s1);
}


static void __test_lookup (btree_t *btree,
                           uint32_t from,
                           uint32_t to,
                           uint32_t step)
{
    char lkvalue[__VALUESZ + 1];
    char value[__VALUESZ + 1];
    char key[__KEYSZ + 1];
    uint32_t lksize, size;
    uint64_t stime, etime;

    printf("Lookup from %u to %u Step %u\n", from, to, step);
    stime = time_micros();

    for (; from < to; from += step) {
        snprintf(key, __KEYSZ + 1, "K-%08d", from);
        size = snprintf(value, __VALUESZ + 1, "V-%08d-%04d", from << 2, from);
        if (!(lksize = btree_lookup(btree, key, lkvalue, __VALUESZ))) {
            printf(" - Lookup Failed %s\n", key);
        } else if (lksize != size || memcmp(value, lkvalue, size)) {
            printf(" - Lookup something wrong: %s %u %u\n", key, size, lksize);
        }
    }

    etime = time_micros();
    printf("[TIME] Lookup %.5f\n", (etime - stime) / 1000000.0f);
}

static void __test_insert (btree_t *btree,
                           uint32_t from,
                           uint32_t to,
                           uint32_t step)
{
    char value[__VALUESZ + 1];
    char key[__KEYSZ + 1];
    uint64_t stime, etime;
    uint32_t size;

    printf("Insert from %u to %u Step %u\n", from, to, step);
    stime = time_micros();

    for (; from < to; from += step) {
        snprintf(key, __KEYSZ + 1, "K-%08d", from);
        size = snprintf(value, __VALUESZ + 1, "V-%08d-%04d", from << 2, from);

        if (btree_insert(btree, key, value, size)) {
            printf(" - Failed - %s: %s (%u)\n", key, value, size);
            break;
        }
    }

    etime = time_micros();
    printf("[TIME] Insert %.5f\n", (etime - stime) / 1000000.0f);
}

static void __test_remove (btree_t *btree,
                           uint32_t from,
                           uint32_t to,
                           uint32_t step)
{
    char key[__KEYSZ + 1];
    uint64_t stime, etime;

    printf("Remove from %u to %u Step %u\n", from, to, step);
    stime = time_micros();

    for (; from < to; from += step) {
        snprintf(key, __KEYSZ + 1, "K-%08d", from);

        if (btree_remove(btree, key)) {
            printf(" - Failed - %s\n", key);
            break;
        }
    }

    etime = time_micros();
    printf("[TIME] Remove %.5f\n", (etime - stime) / 1000000.0f);
}

static void __test_sync (btree_t *btree) {
    uint64_t stime, etime;

    stime = time_micros();

    printf("Sync\n");
    if (btree_sync(btree))
        printf(" - Failed\n");

    etime = time_micros();
    printf("[TIME] Sync %.5f\n", (etime - stime) / 1000000.0f);
}

int main (int argc, char **argv) {
    struct btdisk_data data;
    btree_t btree;
    btdisk_t disk;

    memset(&data, 0, sizeof(struct btdisk_data));
    disk.append = __btdisk_append;
    disk.write = __btdisk_write;
    disk.read = __btdisk_read;
    disk.erase = __btdisk_erase;
    disk.crc_pointer = __btdisk_addler32;
    disk.crc_block = __btdisk_addler32;
    disk.decompress = NULL;
    disk.compress = NULL;

    data.offset = 512U + 64U;
    disk.internal = &data;

#if TEST_WRITE
    if ((data.fd = open("test.disk", O_CREAT | O_RDWR, 0600)) < 0) {
        perror("open()");
        return(1);
    }

    if (btree_create(&btree, &disk, __CACHESZ, data.offset - 64U,
                       __BLOCKSZ, 0, __KEYSZ,
                       0xf5bc2eac, 0xb4e6, __keycmp, NULL))
    {
        printf("btree_create(): Failed\n");
        return(1);
    }
#else
    if ((data.fd = open("test.disk", O_RDONLY)) < 0) {
        perror("open()");
        return(1);
    }

    if (btree_open(&btree, &disk, __CACHESZ, data.offset - 64U,
                     0xf5bc2eac, 0xb4e6, __keycmp, NULL))
    {
        printf("btree_open(): Failed\n");
        return(1);
    }
#endif

#if TEST_WRITE
    __test_insert(&btree, 0, __NKEYS, 2);
    __test_insert(&btree, 1, __NKEYS, 2);
#ifdef __BTREE_DEBUG
    printf("Debug\n");
    btree_debug(&btree, __key_debug, __data_debug);
#endif
#if 1
    __test_sync(&btree);
#endif
#endif
#if 1
    __test_lookup(&btree, 0, __NKEYS, 1);
#endif
#ifdef __BTREE_DEBUG
    printf("Debug\n");
    btree_debug(&btree, __key_debug, __data_debug);
#endif
#if TEST_WRITE
    __test_remove(&btree, 0, 20, 1);
    __test_lookup(&btree, 20, __NKEYS, 1);

#ifdef __BTREE_DEBUG
    printf("Debug\n");
    btree_debug(&btree, __key_debug, __data_debug);
#endif
#endif
    printf("CLOSE\n");
    btree_close(&btree);
    close(data.fd);
    return(0);
}
