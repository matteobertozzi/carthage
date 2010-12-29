#ifndef _BTREE_H_
#define _BTREE_H_

#include <stdint.h>
#include <stdlib.h>

typedef struct btnode btnode_t;

typedef struct btdisk btdisk_t;

typedef uint64_t (*btdisk_append_t) (btdisk_t *disk,
                                     const void *data,
                                     uint32_t size);
typedef uint64_t (*btdisk_erase_t)  (btdisk_t *disk,
                                     uint64_t offset,
                                     uint32_t size);
typedef uint64_t (*btdisk_write_t)  (btdisk_t *disk,
                                     uint64_t block_offset,
                                     uint32_t block_size,
                                     const void *data,
                                     uint32_t size);
typedef uint32_t (*btdisk_read_t)   (btdisk_t *disk,
                                     uint64_t offset,
                                     void *buffer,
                                     uint32_t bufsize);
typedef uint32_t (*btdisk_crc_t)    (btdisk_t *disk,
                                     const void *data,
                                     uint32_t size);

typedef uint32_t (*decompress_t)  (void *user_data,
                                   void *dst,
                                   const void *src,
                                   uint32_t size);
typedef uint32_t (*compress_t)    (void *user_data,
                                   void *dst,
                                   const void *src,
                                   uint32_t size);

typedef void (*memcopy_t)         (void *user_data,
                                   void *dst,
                                   const void *src,
                                   uint32_t n);
typedef int (*compare_t)          (void *user_data,
                                   const void *a,
                                   const void *b);

struct btdisk {
    btdisk_append_t append;         /* Disk Append Function */
    btdisk_erase_t  erase;          /* Disk Erase Function */
    btdisk_write_t  write;          /* Disk Write Function */
    btdisk_read_t   read;           /* Disk Read Function */

    btdisk_crc_t    crc_pointer;    /* CRC Pointer */
    btdisk_crc_t    crc_block;      /* CRC Block */

    decompress_t    decompress;
    compress_t      compress;

    void *          internal;       /* Disk Internal Data */
};

typedef struct btcache_node btcache_node_t;

typedef struct btcache {
    btcache_node_t **table;
    btcache_node_t * head;
    uint32_t           used;
    uint32_t           size;
} btcache_t;

typedef struct btree {
    uint8_t   super[64];          /* B*Tree Super-Block */
    uint64_t  super_offset;       /* B*Tree Super-Offset */
    btcache_t cache;

    btdisk_t *disk;               /* B*Tree Disk Driver */

    memcopy_t prefix_keycpy;      /* Prefix Key from key */
    compare_t prefix_keycmp;      /* Prefix Key compare */
    compare_t keycmp;             /* Key compare */

    btnode_t *root;               /* B*Tree Root */

    void *    user_data;          /* B*Tree User-Data */
} btree_t;

int         btree_create          (btree_t *btree,
                                   btdisk_t *disk,
                                   uint32_t cache_size,
                                   uint64_t super_offset,
                                   uint32_t block_size,
                                   uint8_t format,
                                   uint32_t key_size,
                                   uint32_t btree_magic,
                                   uint16_t node_magic,
                                   compare_t keycmp,
                                   void *user_data);

int         btree_prefix_create   (btree_t *btree,
                                   btdisk_t *disk,
                                   uint32_t cache_size,
                                   uint64_t super_offset,
                                   uint32_t block_size,
                                   uint8_t format,
                                   uint32_t key_size,
                                   uint32_t btree_magic,
                                   uint16_t node_magic,
                                   memcopy_t prefix_keycpy,
                                   compare_t prefix_keycmp,
                                   compare_t keycmp,
                                   void *user_data);

int         btree_open            (btree_t *btree,
                                   btdisk_t *disk,
                                   uint32_t cache_size,
                                   uint64_t super_offset,
                                   uint32_t btree_magic,
                                   uint16_t node_magic,
                                   compare_t keycmp,
                                   void *user_data);

int         btree_prefix_open     (btree_t *btree,
                                   btdisk_t *disk,
                                   uint32_t cache_size,
                                   uint64_t super_offset,
                                   uint32_t btree_magic,
                                   uint16_t node_magic,
                                   memcopy_t prefix_keycpy,
                                   compare_t prefix_keycmp,
                                   compare_t keycmp,
                                   void *user_data);

int         btree_close           (btree_t *btree);

int         btree_sync            (btree_t *btree);

int         btree_insert          (btree_t *btree,
                                   const void *key,
                                   const void *value,
                                   uint32_t size);
int         btree_remove          (btree_t *btree,
                                   const void *key);

uint32_t    btree_contains        (btree_t *btree,
                                   const void *key);

uint32_t    btree_lookup          (btree_t *btree,
                                   const void *key,
                                   void *buffer,
                                   uint32_t bufsize);

#ifdef __BTREE_DEBUG
typedef void (*btree_data_debug_t) (void *user_data,
                                    const void *data,
                                    uint32_t size);
typedef void (*btree_key_debug_t)  (void *user_data,
                                    const void *key);

void        btree_debug            (btree_t *btree,
                                    btree_key_debug_t key_debug,
                                    btree_data_debug_t data_debug);
#endif /* !__BTREE_DEBUG */

#endif /* !_BTREE_H_ */

