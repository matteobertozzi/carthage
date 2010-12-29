/*
 * This is a variant of B+Tree, node split when gets full and there's no
 * right-linking due to copy-on-write ability.
 * When a leaf change parent nodes must be rewritten due to leaf checksum.
 *                                  __
 *                                 |__|
 *                +------------------+------------------+
 *                __                 __                 __
 *               |__|               |__|               |__|
 *           +----+----+        +----+----+         +----+----+
 *          __   __   __       __   __   __       __   __   __
 *         |__| |__| |__|     |__| |__| |__|     |__| |__| |__|
 */

#define __BTREE_DEBUG
#ifdef __BTREE_DEBUG
    #include <inttypes.h>
    #include <stdio.h>
#endif /* !__BTREE_DEBUG */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "btree.h"

/* ===========================================================================
 *  On-Disk Data Structure
 */

/* B-Tree Super-Block 64 byte */
struct super_block {
    uint32_t   sb_magic;               /* BTree Super-Block Magic */

    uint16_t   sb_format;              /* BTree Format (Unused) */
    uint16_t   sb_node_magic;          /* BTree Node-Head Magic */

    uint32_t   sb_block_size;          /* BTree Block Size */
    uint16_t   sb_prefix_size;         /* BTree Internal Node Key Size */
    uint16_t   sb_key_size;            /* BTree Leaf Node Key Size */
    uint16_t   sb_fanout;              /* BTree Fanout */
    uint8_t    sb_height;              /* BTree Height */
    uint8_t    sb_pad0;
    uint32_t   sb_pad1;

    uint64_t   sb_item_count;          /* BTree Item Count */
    uint64_t   sb_node_count;          /* BTree Node Count */
    uint64_t   sb_stored_data;         /* BTree Stored Data Size */

    uint64_t   sb_root;                /* BTree Root Node Pointer */
    uint32_t   sb_root_crc;            /* BTree Root Block CRC    */
    uint32_t   sb_root_size;           /* BTree Root Block Size   */
} __attribute__((packed));

/* Node Header - 16 byte */
struct node_head {
    uint32_t   nh_crc;                 /* BTree Node CRC */

    uint16_t   nh_magic;               /* BTree Node Header Magic */
    uint8_t    nh_bmagic;              /* BTree Node BTree Trunc-Magic */
    uint8_t    nh_level;               /* BTree Node Level */

    uint32_t   nh_items;               /* BTree Node Number of Items */
    uint32_t   nh_free;                /* BTree Node Free Space */
} __attribute__((packed));

/* Item Header - 8 byte */
struct item_head {
    /* _____   ih_key */
    uint32_t   ih_offset;              /* BTree Leaf Item Offset */
    uint32_t   ih_size;                /* BTree Leaf Item Size   */
} __attribute__((packed));

/* Node Pointer - 16 byte */
struct node_pointer {
    uint64_t   np_blocknr;             /* BTree Iternal Node Pointer */
    uint32_t   np_crc;                 /* BTree Pointed Block CRC    */
    uint32_t   np_size;                /* BTree Pointed Block Size   */
} __attribute__((packed));

/* ============================================================================
 *  Data Structures size.
 */
#define SUPER_BLOCK_SIZE            (sizeof(struct super_block))
#define NODE_POINTER_SIZE           (sizeof(struct node_pointer))
#define NODE_HEAD_SIZE              (sizeof(struct node_head))
#define ITEM_HEAD_SIZE              (sizeof(struct item_head))

/* ===========================================================================
 *  Data Structure cast
 */
#define SUPER_BLOCK(x)              ((struct super_block *)(x))
#define NODE_POINTER(x)             ((struct node_pointer *)(x))
#define NODE_HEAD(x)                ((struct node_head *)(x))
#define ITEM_HEAD(x)                ((struct item_head *)(x))

/* ===========================================================================
 *  Super-Block Macros
 */
#define __btree_super(btree)        SUPER_BLOCK((btree)->super)
#define __btree_root(btree)         (__btree_super(btree)->sb_root)
#define __btree_flags(btree)        (__btree_super(btree)->sb_flags)
#define __btree_magic(btree)        (__btree_super(btree)->sb_magic)
#define __btree_height(btree)       (__btree_super(btree)->sb_height)
#define __btree_fanout(btree)       (__btree_super(btree)->sb_fanout)
#define __btree_format(btree)       (__btree_super(btree)->sb_format)
#define __btree_key_size(btree)     (__btree_super(btree)->sb_key_size)
#define __btree_item_size(btree)    (__btree_key_size(btree) + ITEM_HEAD_SIZE)
#define __btree_block_size(btree)   (__btree_super(btree)->sb_block_size)
#define __btree_node_magic(btree)   (__btree_super(btree)->sb_node_magic)
#define __btree_node_space(btree)   (__btree_block_size(btree) - NODE_HEAD_SIZE)
#define __btree_prefix_size(btree)  (__btree_super(btree)->sb_prefix_size)
#define __btree_stored_size(btree)  (__btree_super(btree)->sb_stored_data)
#define __btree_item_count(btree)   (__btree_super(btree)->sb_item_count)
#define __btree_node_count(btree)   (__btree_super(btree)->sb_node_count)
#define __btree_is_empty(btree)     (__btree_item_count(btree) == 0)
#define __btree_is_null(btree)      (__btree_node_count(btree) == 0)

#define __btree_root_pointer(btree)                                          \
    NODE_POINTER((btree)->super + SUPER_BLOCK_SIZE - NODE_POINTER_SIZE)

/* ============================================================================
 * Node Levels
 */
#define TWIG_NODE_LEVEL             (2)   /* Twigs contains Pointers */
#define LEAF_NODE_LEVEL             (1)   /* Leafs contains Data     */

/* ============================================================================
 *  Node-Header Macros
 */
#define __node_head(node)           NODE_HEAD(((node)->data))
#define __node_level(node)          (__node_head(node)->nh_level)
#define __node_items(node)          (__node_head(node)->nh_items)
#define __node_magic(node)          (__node_head(node)->nh_magic)
#define __node_free(node)           (__node_head(node)->nh_free)
#define __node_crc(node)            (__node_head(node)->nh_crc)

#define __node_is_leaf(node)        (__node_level(node) == LEAF_NODE_LEVEL)
#define __node_is_twig(node)        (__node_level(node) == TWIG_NODE_LEVEL)
#define __node_is_internal(node)    (__node_level(node) > LEAF_NODE_LEVEL)

#define NODE_CRC_OFFSET             (sizeof(uint32_t))

/* ============================================================================
 *  Node Flags Macros
 */
#define NODE_FLAG_DIRTY                (1 << 0)
#define NODE_FLAG_ON_DISK              (1 << 1)
#define NODE_FLAG_UPDATE               (1 << 2)
#define NODE_FLAG_NOSORT               (1 << 3)

#define __node_has_flag(node, flag)    ((node)->flags & (flag))
#define __node_set_flag(node, flag)    (node)->flags |= (flag)
#define __node_unset_flag(node, flag)  (node)->flags &= ~(flag)

#define __node_is_dirty(node)          __node_has_flag(node, NODE_FLAG_DIRTY)
#define __node_set_dirty(node)         __node_set_flag(node, NODE_FLAG_DIRTY)
#define __node_set_clean(node)         __node_unset_flag(node, NODE_FLAG_DIRTY)

#define __node_is_on_disk(node)        __node_has_flag(node, NODE_FLAG_ON_DISK)
#define __node_set_on_disk(node)       __node_set_flag(node, NODE_FLAG_ON_DISK)

#define __node_need_update(node)       __node_has_flag(node, NODE_FLAG_UPDATE)
#define __node_set_to_update(node)     __node_set_flag(node, NODE_FLAG_UPDATE)

#define __node_is_unsorted(node)       __node_has_flag(node, NODE_FLAG_NOSORT)
#define __node_set_sorted(node)        __node_unset_flag(node, NODE_FLAG_NOSORT)
#define __node_set_unsorted(node)      __node_set_flag(node, NODE_FLAG_NOSORT)

/* ============================================================================
 *  Twig Node Macros
 */
#define __twig_pointer(btree, node, index)                                  \
    ((struct node_pointer *)                                                \
    ((((node)->data) + NODE_HEAD_SIZE) +                                    \
     (__btree_fanout(btree) * __btree_prefix_size(btree)) +                 \
     ((index) * NODE_POINTER_SIZE)))

#define __twig_key(btree, node, index)                                      \
    ((((node)->data) + NODE_HEAD_SIZE) +                                    \
     ((index) * __btree_prefix_size(btree)))

#define __twig_split_target(btree, key, node_left, node_right)              \
    ((btree->keycmp(btree->user_data, key,                                  \
                   __twig_first_key(btree, node_right)) < 0) ?              \
                   node_left : node_right)

#define __twig_first_key(btree, node)                                       \
    __twig_key(btree, node, 0)

#define __twig_last_key(btree, node)                                        \
    __twig_key(btree, node, __node_items(node) - 1)

#define __twig_last_prev_key(btree, node)                                   \
    __twig_key(btree, node, __node_items(node) - 2)

#define __twig_needed(btree)                                                \
    (__btree_prefix_size(btree) + NODE_POINTER_SIZE)

/* ============================================================================
 *  Leaf Node Macros
 */
#define __leaf_key(btree, node, index)                                      \
    ((((node)->data) + NODE_HEAD_SIZE) +                                    \
     ((index) * __btree_key_size(btree)) +                                  \
     ((index) * ITEM_HEAD_SIZE))

#define __leaf_split_target(btree, key, node_left, node_right)              \
    ((btree->keycmp(btree->user_data, key,                                  \
                   __leaf_first_key(btree, node_right)) < 0) ?              \
                   node_left : node_right)

#define __leaf_first_key(btree, node)                                       \
    __leaf_key(btree, node, 0)

#define __leaf_mid_key(btree, node)                                         \
    __leaf_key(btree, node, __node_items(node) >> 1)

#define __leaf_last_key(btree, node)                                        \
    __leaf_key(btree, node, __node_items(node) - 1)

#define __leaf_last_prev_key(btree, node)                                   \
    __leaf_key(btree, node, __node_items(node) - 2)

/* ============================================================================
 *  Item (Leaf node) Macros
 */
#define __item_needed_size(btree, size)                                     \
    (__btree_key_size(btree) + ITEM_HEAD_SIZE + (size))

#define __item_head(btree, node, index)                                     \
    (ITEM_HEAD(__leaf_key(btree, node, index) + __btree_key_size(btree)))

#define __item_body(btree, node, index)                                     \
    ((node)->data + (__item_head(btree, node, index)->ih_offset))

#define __item_body_size(btree, node, index)                                \
    (__item_head(btree, node, index)->ih_size)

/* ============================================================================
 *  BTree Disk Macros
 */
#define __btdisk_append(btree, data, size)                                  \
    (btree)->disk->append((btree)->disk, data, size)

#define __btdisk_write(btree, block_offset, block_size, data, size)         \
    (btree)->disk->write((btree)->disk, block_offset, block_size, data, size)

#define __btdisk_erase(btree, offset, size)                                 \
    (btree)->disk->erase((btree)->disk, offset, size)

#define __btdisk_read(btree, offset, data, size)                            \
    (btree)->disk->read((btree)->disk, offset, data, size)

#define __btdisk_crc(btree, func, data, size)                               \
    (btree)->disk->func((btree)->disk, data, size)

#define __btdisk_crc_pointer(btree, data, size)                             \
    __btdisk_crc(btree, crc_pointer, data, size)

#define __btdisk_crc_block(btree, offset, data, size)                       \
    __btdisk_crc(btree, crc_block, ((uint8_t *)data)+offset, size-(offset))

/* ===========================================================================
 *  In-Memory Data Structure
 */
typedef struct btnode_place btnode_place_t;

struct btnode {
    btnode_t **pointers;
    uint8_t *    data;
    uint64_t     blocknr;
    unsigned int size;
    unsigned int flags;
    int          refs;
};

struct btnode_place {
    const void *value;          /* Node Value Pointer */
    uint32_t    index;          /* Position of object in the node */
    uint32_t    size;           /* Size of object value */
    uint8_t     found;          /* True if object is found */
};

struct btcache_node {
    btcache_node_t *next;     /* Next pointer in LRU cache */
    btcache_node_t *prev;     /* Prev pointer in LRU cache */
    btcache_node_t *hash;     /* Next pointer in Hash Table */

    uint64_t blocknr;           /* Node on disk data offset */
    uint8_t *block;             /* Node on disk data block */
};

/* ===========================================================================
 *  PRIVATE Binary Search Utils
 */
static const void *__index_search (btree_t *btree,
                                   const void *base,
                                   uint32_t n,
                                   uint32_t size,
                                   const void *key,
                                   compare_t cmp_func,
                                   uint32_t *index)
{
    const void *mid;
    long low, high;
    int cmp;
    long i;

    high = ((long)n) - 1;
    low = 0;
    for (i = ((low + high) >> 1); low <= high; i = ((low + high) >> 1)) {
        mid = ((const uint8_t *)base) + (i * size);
        if (!(cmp = cmp_func(btree->user_data, mid, key))) {
            *index = i;
            return(mid);
        }

        if (cmp < 0)
            low = i + 1;
        else
            high = i - 1;
    }

    *index = low;
    return(NULL);
}

/* ===========================================================================
 *  PRIVATE Operations (Node Twigs)
 *
 *  B-Tree Node Twig:
 *      +----+----+----+----+----+----+
 *      | K1 | K2 |    | P1 | P2 |    |
 *      +----+----+----+----+----+----+
 *
 * B-Tree Twig Node Split:
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 *    | K1 | K2 | K3 | P1 | P2 | P3 |        |    |    |    |    |    |    |
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 * Given 'node' (left one) is full, we allocate another node (the right one),
 * then we search for the median key (of the left node), all the keys and
 * pointers on the right of median_key are shifted to the right node.
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 *    | K1 | K2 |    | P1 | P2 |    |        | K3 |    |    | P3 |    |    |
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 */
static void __twig_replace (btree_t *btree,
                            btnode_t *node,
                            uint32_t index,
                            const void *key,
                            const void *value)
{
    void *src;

    /* Replace Key */
    src = __twig_key(btree, node, index);
    if (btree->prefix_keycpy != NULL) {
        btree->prefix_keycpy(btree->user_data, src, key,
                             __btree_prefix_size(btree));
    } else {
        memcpy(src, key, __btree_prefix_size(btree));
    }

    /* Replace Pointer */
    src = __twig_pointer(btree, node, index);
    memcpy(src, value, NODE_POINTER_SIZE);

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __twig_inline_replace (btree_t *btree,
                                   btnode_t *node,
                                   uint32_t index,
                                   const void *value)
{
    void *src;

    /* Replace Pointer */
    src = __twig_pointer(btree, node, index);
    memcpy(src, value, NODE_POINTER_SIZE);

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __twig_insert (btree_t *btree,
                           btnode_t *node,
                           uint32_t index,
                           const void *key,
                           const void *value)
{
    struct node_head *header;
    void *src, *dst;
    uint32_t n;

    header = __node_head(node);
    n = (header->nh_items - index);

    /* Move Keys and Insert the new one */
    src = __twig_key(btree, node, index);
    dst = __twig_key(btree, node, index + 1);
    memmove(dst, src, n * __btree_prefix_size(btree));

    /* Copy key on node */
    if (btree->prefix_keycpy != NULL) {
        btree->prefix_keycpy(btree->user_data, src, key,
                             __btree_prefix_size(btree));
    } else {
        memcpy(src, key, __btree_prefix_size(btree));
    }

    /* Move Pointers and Insert the new one */
    src = __twig_pointer(btree, node, index);
    dst = __twig_pointer(btree, node, index + 1);
    memmove(dst, src, n * NODE_POINTER_SIZE);
    memcpy(src, value, NODE_POINTER_SIZE);

    /* Move In-Memory Pointers */
    src = &(node->pointers[index]);
    dst = &(node->pointers[index + 1]);
    memmove(dst, src, n * sizeof(btnode_t *));

    /* Setup node header */
    header->nh_free -= __twig_needed(btree);
    header->nh_items++;

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __twig_remove (btree_t *btree,
                           btnode_t *node,
                           uint32_t index)
{
    struct node_head *header;
    void *src, *dst;
    uint32_t n;

    header = __node_head(node);

    if ((index + 1) < header->nh_items) {
        n = (header->nh_items - index);

        /* Move Keys */
        src = __twig_key(btree, node, index + 1);
        dst = __twig_key(btree, node, index);
        memmove(dst, src, n * __btree_prefix_size(btree));

        /* Move Pointers */
        src = __twig_pointer(btree, node, index + 1);
        dst = __twig_pointer(btree, node, index);
        memmove(dst, src, n * NODE_POINTER_SIZE);

        /* Move In-Memory Pointers */
        src = &(node->pointers[index + 1]);
        dst = &(node->pointers[index]);
        memmove(dst, src, n * sizeof(btnode_t *));
    }

    /* Setup node header */
    header->nh_free -= __twig_needed(btree);
    header->nh_items--;
    node->pointers[header->nh_items] = NULL;

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __twig_split (btree_t *btree,
                          btnode_t *left,
                          btnode_t *right)
{
    struct node_head *header;
    uint32_t right_key;
    uint32_t left_key;
    void *rdst;
    void *mid;

    /* Initialize Key Info */
    left_key = __node_items(left) >> 1;
    right_key = __node_items(left) - left_key;

    /* Right Neighbor - Copy Keys */
    mid = __twig_key(btree, left, left_key);
    rdst = __twig_key(btree, right, 0);
    memcpy(rdst, mid, right_key * __btree_prefix_size(btree));

    /* Right Neighbor - Copy Pointers */
    mid = __twig_pointer(btree, left, left_key);
    rdst = __twig_pointer(btree, right, 0);
    memcpy(rdst, mid, right_key * NODE_POINTER_SIZE);

    /* Move In-Memory Pointers */
    mid = &(left->pointers[left_key]);
    rdst = &(right->pointers[0]);
    memcpy(rdst, mid, right_key * sizeof(btnode_t *));
    memset(mid, 0, right_key * sizeof(btnode_t *));

    /* Right Neighbor - Adjust Header */
    header = __node_head(right);
    header->nh_items = right_key;
    header->nh_free = __btree_node_space(btree) -
                     (right_key * __btree_prefix_size(btree)) -
                     (right_key * NODE_POINTER_SIZE);

    /* Left Neighbor - Adjust Header */
    header = __node_head(left);
    header->nh_items = left_key;
    header->nh_free = __btree_node_space(btree) -
                      (left_key * __btree_prefix_size(btree)) -
                      (left_key * NODE_POINTER_SIZE);

    /* Mark left & right Node as Dirty */
    __node_set_dirty(right);
    __node_set_dirty(left);
}

static void __twig_merge (btree_t *btree,
                          btnode_t *left,
                          btnode_t *right)
{
    struct node_head *header;
    const void *value;
    const void *key;
    uint32_t index;
    uint32_t i;

    header = __node_head(right);
    index = __node_items(left);

    for (i = 0; i < header->nh_items; ++i) {
        key = __twig_key(btree, right, i);
        value = __twig_pointer(btree, right, i);
        __twig_insert(btree, left, index + i, key, value);
    }

    /* Mark right node as dirty, (but you should delete it) */
    header->nh_free = __btree_node_space(btree);
    header->nh_items = 0;
    __node_set_dirty(left);
}

static int __twig_can_merge (btree_t *btree,
                             btnode_t *left,
                             btnode_t *right)
{
    return((__node_items(left) + __node_items(right)) <= __btree_fanout(btree));
}

static struct node_pointer *__twig_index_search (btree_t *btree,
                                                 btnode_t *node,
                                                 const void *key,
                                                 uint32_t *index)
{
    if (__index_search(btree,
                       __twig_first_key(btree, node),
                       __node_items(node),
                       __btree_prefix_size(btree),
                       key,
                       btree->prefix_keycmp,
                       index))
    {
        return(__twig_pointer(btree, node, *index));
    }

    return(NULL);
}

static int __twig_search (btree_t *btree,
                          btnode_t *node,
                          const void *key,
                          btnode_place_t *place)
{
    struct node_pointer *pointer;

    if ((pointer = __twig_index_search(btree, node, key, &(place->index)))) {
        place->size = NODE_POINTER_SIZE;
        place->value = pointer;
    }

    return((place->found = (pointer != NULL)));
}

#ifdef __BTREE_DEBUG
static void __twig_debug (btree_t *btree,
                          btnode_t *node,
                          btree_key_debug_t key_debug)
{
    struct node_pointer *pointer;
    struct node_head *header;
    const void *key;
    uint32_t i;

    header = __node_head(node);

    printf("\n=============================================\n");
    printf("Twig %p - ", node);
    printf("Refs %d - ", node->refs);
    printf("Items %3u - ", header->nh_items);
    printf("Level %2u\n", header->nh_level);

    for (i = 0; i < header->nh_items; ++i) {
        key = __twig_key(btree, node, i);
        pointer = __twig_pointer(btree, node, i);

        printf("(%3u:", i);
        key_debug(btree->user_data, key);
        printf("[%4"PRIu64":%u])", pointer->np_blocknr, pointer->np_size);
    }

    printf("\n=============================================\n");
}
#endif /* !__BTREE_DEBUG */

/* ===========================================================================
 *  PRIVATE Operations (Node Leaf)
 *
 *  B-Tree Leaf Node: Data Insert/Remove
 *   124XXXXDDBBAA
 *   12 4XCCDDBBAA
 *   1234XCCDDBBAA
 *
 *  B-Tree Leaf Node Split
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 *    | K1 | K2 | K3 | D1 | D2 | D3 |        |    |    |    |    |    |    |
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 * Given 'node' (left one) is full, we allocate another node (the right one),
 * then we search for the median key (of the left node), all the keys and
 * pointers on the right of median_key are shifted to the right node.
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 *    | K1 | K2 |    |    | D1 | D2 |        | K3 |    |    |    |    | D3 |
 *    +----+----+----+----+----+----+        +----+----+----+----+----+----+
 */
static uint32_t __leaf_body_size (btree_t *btree,
                                  btnode_t *node,
                                  uint32_t from,
                                  uint32_t to)
{
    uint32_t size = 0U;
    uint32_t offset;
    uint8_t *p;

    p = (uint8_t *)__item_head(btree, node, from);
    offset = __btree_item_size(btree);
    for (; from < to; ++from) {
        size += ITEM_HEAD(p)->ih_size;
        p += offset;
    }

    return(size);
}

static void __leaf_move_offsets (btree_t *btree,
                                 btnode_t *node,
                                 uint32_t from,
                                 uint32_t to,
                                 int32_t diff)
{
    uint32_t offset;
    uint8_t *p;

    p = (uint8_t *)__item_head(btree, node, from);
    offset = __btree_item_size(btree);
    for (; from < to; ++from) {
        ITEM_HEAD(p)->ih_offset += diff;
        p += offset;
    }
}

static void __leaf_replace (btree_t *btree,
                            btnode_t *node,
                            uint32_t index,
                            const void *value,
                            uint32_t size)
{
    struct node_head *header;
    uint32_t old_size;
    long size_diff;
    void *dst;

    header = __node_head(node);
    dst = __item_body(btree, node, index);

    old_size = __item_body_size(btree, node, index);
    size_diff = size - old_size;
    header->nh_free += size_diff;

    /* Move data */
    memmove(dst + old_size, dst + size,
            __leaf_body_size(btree, node, index, header->nh_items - 1));

    /* Move Item Offsets */
    __leaf_move_offsets(btree, node, index, header->nh_items, size_diff);

    /* Mark node as dirty */
    __node_set_dirty(node);
}

static void __leaf_inline_replace (btree_t *btree,
                                   btnode_t *node,
                                   uint32_t index,
                                   const void *value,
                                   uint32_t size)
{
    void *dst;

    /* Replace Key Body */
    dst = __item_body(btree, node, index);
    memcpy(dst, value, size);

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __leaf_insert (btree_t *btree,
                           btnode_t *node,
                           uint32_t index,
                           const void *key,
                           const void *value,
                           uint32_t data_size)
{
    struct item_head item_head;
    struct node_head *header;
    void *src_body;
    uint32_t size;
    void *dst;
    void *src;

    header = __node_head(node);
    src = __leaf_key(btree, node, index);

    /* Setup Item Header */
    item_head.ih_size = data_size;
    item_head.ih_offset = (header->nh_free - data_size) + NODE_HEAD_SIZE +
                          (header->nh_items * __btree_item_size(btree));

    if (index < header->nh_items) {
        /* Move Data */
        size = __leaf_body_size(btree, node, index, header->nh_items);
        src_body = __item_body(btree, node, header->nh_items - 1);
        dst = src_body - data_size;
        memmove(dst, src_body, size);

        /* Setup Item Offset */
        item_head.ih_offset = ((uint8_t *) dst - node->data) + size;

        /* Move Items Offset */
        __leaf_move_offsets(btree, node, index, header->nh_items, -data_size);

        /* Move Keys */
        dst = __leaf_key(btree, node, index + 1);
        size = (header->nh_items - index) * __btree_item_size(btree);
        memmove(dst, src, size);
    }

    /* Write Key, ItemHead and Data */
    memcpy(src, key, __btree_key_size(btree));
    memcpy(src + __btree_key_size(btree), &item_head, ITEM_HEAD_SIZE);
    memcpy(node->data + item_head.ih_offset, value, data_size);

    /* Setup Node Header for New Item */
    header->nh_free -= (__btree_item_size(btree) + data_size);
    header->nh_items++;

    /* Update Super-Block with One more Item, and update stored data size */
    __btree_super(btree)->sb_item_count++;
    __btree_super(btree)->sb_stored_data += data_size;

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __leaf_remove (btree_t *btree,
                           btnode_t *node,
                           uint32_t index)
{

    struct node_head *header;
    uint32_t data_size;
    void *src, *dst;
    uint32_t size;

    header = __node_head(node);
    data_size = __item_body_size(btree, node, index);

    if ((index + 1) < header->nh_items) {
        /* Move Data */
        src = __item_body(btree, node, header->nh_items - 1);
        size = __leaf_body_size(btree, node, index + 1, header->nh_items);
        dst = src + data_size;
        memmove(dst, src, size);

        /* Move Items Offset */
        __leaf_move_offsets(btree, node, index, header->nh_items, data_size);

        /* Move Keys */
        dst = __leaf_key(btree, node, index);
        src = __leaf_key(btree, node, index + 1);
        size = (header->nh_items - index) * __btree_item_size(btree);
        memmove(dst, src, size);
    }

    /* Setup Node Header for New Item */
    header->nh_free += (__btree_item_size(btree) + data_size);
    header->nh_items--;

    /* Update Super-Block with One more Item, and update stored data size */
    __btree_super(btree)->sb_item_count--;
    __btree_super(btree)->sb_stored_data -= data_size;

    /* Mark Node as Dirty */
    __node_set_dirty(node);
}

static void __leaf_split (btree_t *btree,
                          btnode_t *left,
                          btnode_t *right)
{
    struct node_head *right_header;
    struct node_head *left_header;
    uint32_t right_key;
    uint32_t left_key;
    uint32_t size;
    void *dst;
    void *src;

    /* Initialize Key Info */
    right_header = __node_head(right);
    left_header = __node_head(left);
    left_key = (left_header->nh_items >> 1);
    right_key = left_header->nh_items - left_key;

    /* Move Keys */
    src = __leaf_key(btree, left, left_key);
    dst = __leaf_key(btree, right, 0);
    size = __btree_item_size(btree) * right_key;
    memcpy(dst, src, size);

    left_header->nh_free += size;
    right_header->nh_free -= size;

    /* Move Data */
    src = __item_body(btree, left, left_header->nh_items - 1);
    size = __leaf_body_size(btree, left, left_key, left_header->nh_items);
    dst = right->data + __btree_block_size(btree) - size;
    memcpy(dst, src, size);

    left_header->nh_free += size;
    left_header->nh_items -= right_key;

    right_header->nh_free -= size;
    right_header->nh_items += right_key;

    /* Update right data offsets */
    size = __leaf_body_size(btree, left, 0, left_key);
    __leaf_move_offsets(btree, right, 0, right_key, size);

    /* Mark left & right Node as Dirty */
    __node_set_dirty(right);
    __node_set_dirty(left);
}

static void __leaf_merge (btree_t *btree,
                          btnode_t *left,
                          btnode_t *right)
{
    struct item_head *item_head;
    struct node_head *header;
    const void *key;
    uint32_t index;
    uint32_t i;

    header = __node_head(right);
    index = __node_items(left);

    for (i = 0; i < header->nh_items; ++i) {
        item_head = __item_head(btree, right, i);
        key = __leaf_key(btree, right, i);

        __leaf_insert(btree, left, index + i, key,
                      right->data + item_head->ih_offset,
                      item_head->ih_size);
    }

    /* Mark right node as dirty, (but you should delete it) */
    header->nh_free = __btree_node_space(btree);
    header->nh_items = 0;
    __node_set_dirty(left);
}

static int __leaf_can_merge (btree_t *btree,
                             btnode_t *left,
                             btnode_t *right)
{
    uint32_t size;

    size  = __leaf_body_size(btree, right, 0, __node_items(right));
    size += __leaf_body_size(btree, left, 0, __node_items(left));
    size += __btree_item_size(btree);
    size += __btree_item_size(btree);

    return(size <= __btree_node_space(btree));
}

static struct item_head *__leaf_index_search (btree_t *btree,
                                              btnode_t *node,
                                              const void *key,
                                              uint32_t *index)
{
    if (__index_search(btree,
                       __leaf_first_key(btree, node),
                       __node_items(node),
                       __btree_key_size(btree) + ITEM_HEAD_SIZE,
                       key,
                       btree->keycmp,
                       index))
    {
        return(__item_head(btree, node, *index));
    }

    return(NULL);
}

static int __leaf_search (btree_t *btree,
                          btnode_t *node,
                          const void *key,
                          btnode_place_t *place)
{
    struct item_head *head;

    if ((head = __leaf_index_search(btree, node, key, &(place->index)))) {
        place->value = __item_body(btree, node, place->index);
        place->size = head->ih_size;
    }

    return((place->found = (head != NULL)));
}

#ifdef __BTREE_DEBUG
static void __leaf_debug (btree_t *btree,
                          btnode_t *node,
                          btree_key_debug_t key_debug,
                          btree_data_debug_t data_debug)
{
    struct item_head *item_head;
    struct node_head *header;
    const void *body;
    const void *key;
    uint32_t i;

    header = __node_head(node);

    printf("\n=============================================\n");
    printf("Leaf %p - ", node);
    printf("Refs %d - ", node->refs);
    printf("Items %3u - ", header->nh_items);
    printf("Free %5u - ", header->nh_free);
    printf("Level %2u\n", header->nh_level);

    for (i = 0; i < header->nh_items; ++i) {
        item_head = __item_head(btree, node, i);
        body = __item_body(btree, node, i);
        key = __leaf_key(btree, node, i);

        printf("(%3u:", i);
        key_debug(btree->user_data, key);
        printf("[%4u:%4u:", item_head->ih_offset, item_head->ih_size);
        data_debug(btree->user_data, body, item_head->ih_size);
        printf("]) ");
    }

    printf("\n=============================================\n");
}
#endif /* !__BTREE_DEBUG */

/* ===========================================================================
 *  PRIVATE Block Cache
 */
#define __btcache_can_add(btree, cache)          ((cache)->size != 0)
#define __btcache_hash(btree, cache, blocknr)    ((blocknr) % (cache)->size)

static btcache_node_t *__btcache_add_node (btree_t *btree,
                                             btcache_t *cache)
{
    btcache_node_t *node;

    if ((node = (btcache_node_t *) malloc(sizeof(btcache_node_t))) == NULL)
        return(NULL);

    node->hash = NULL;
    node->block = NULL;

    if (cache->head != NULL) {
        node->next = cache->head;
        node->prev = cache->head->prev;
        cache->head->prev = node;
        node->prev->next = node;
    } else {
        node->next = node;
        node->prev = node;
    }

    cache->head = node;
    cache->used++;

    return(node);
}

static void __btcache_unhash (btree_t *btree,
                              btcache_t *cache,
                              btcache_node_t *node)
{
    btcache_node_t *p;
    uint32_t index;

    index = __btcache_hash(btree, cache, node->blocknr);
    if ((p = cache->table[index]) == node) {
        cache->table[index] = NULL;
    } else {
        for (; p != NULL; p = p->hash) {
            if (p->hash == node) {
                p->hash = node->hash;
                break;
            }
        }
    }
}

static void __btcache_eject (btree_t *btree,
                             btcache_t *cache,
                             btcache_node_t *node)
{
    /* Unlink hash */
    __btcache_unhash(btree, cache, node);

    if (cache->head->prev != node) {
        btcache_node_t *tail;

        node->prev->next = node->next;
        node->next->prev = node->prev;

        if (cache->head == node)
            cache->head = node->next;

        tail = cache->head->prev;
        tail->next = node;
        node->next = cache->head;
        node->prev = tail;
        cache->head->prev = node;
    }

    /* Mark block as free */
    node->block = NULL;
}

static int __btcache_open (btree_t *btree,
                           btcache_t *cache,
                           uint32_t size)
{
    if (size == 0) {
        cache->size = 0;
        return(1);
    }

    if (!(cache->table = (btcache_node_t **) malloc(size * sizeof(btcache_node_t *))))
        return(2);

    memset(cache->table, 0, size * sizeof(btcache_node_t *));
    cache->head = NULL;
    cache->size = size;
    cache->used = 0U;

    __btcache_add_node(btree, cache);

    return(0);
}

static void __btcache_close (btree_t *btree,
                             btcache_t *cache)
{
    btcache_node_t *next;
    btcache_node_t *p;

    if (cache->size == 0)
        return;

    if (cache->head != NULL) {
        p = cache->head;
        do {
            next = p->next;

            if (p->block)
                free(p->block);
            free(p);

            p = next;
        } while (p != cache->head);
    }

    free(cache->table);
}

static int __btcache_add (btree_t *btree,
                          btcache_t *cache,
                          uint64_t blocknr,
                          uint8_t *block)
{
    btcache_node_t *node;
    uint32_t index;

    node = cache->head->prev;
    if (cache->head->block == NULL || cache->used >= cache->size) {
        cache->head = node;

        if (node->block != NULL) {
            __btcache_unhash(btree, cache, node);
            free(node->block);
        }
    } else {
        if ((node = __btcache_add_node(btree, cache)) == NULL)
            return(1);
    }

    /* Setup Node */
    node->blocknr = blocknr;
    node->block = block;

    /* Insert node into hashtable */
    index = __btcache_hash(btree, cache, blocknr);
    node->hash = cache->table[index];
    cache->table[index] = node;

    return(0);
}

uint8_t *__btcache_lookup (btree_t *btree,
                           btcache_t *cache,
                           uint64_t blocknr)
{
    btcache_node_t *p;
    uint8_t *block;

    if (cache->size == 0)
        return(NULL);

    p = cache->table[__btcache_hash(btree, cache, blocknr)];
    while (p != NULL) {
        if (p->blocknr == blocknr) {
            block = p->block;
            __btcache_eject(btree, cache, p);
            return(block);
        }
        p = p->hash;
    }

    return(NULL);
}

/* ===========================================================================
 *  PRIVATE Operations (Node)
 */
#define __btree_leaf_node_alloc(btree)                                      \
    __btnode_alloc(btree, LEAF_NODE_LEVEL)

#define __btnode_level_alloc(btree, level)                                  \
    (level == LEAF_NODE_LEVEL) ?                                            \
        __btnode_leaf_alloc(btree) :                                        \
        __btnode_internal_alloc(btree)

static btnode_t *__btnode_internal_alloc (btree_t *btree) {
    btnode_t *node;
    uint32_t size;

    size = __btree_fanout(btree) * sizeof(btnode_t *);
    if ((node = (btnode_t *) malloc(sizeof(btnode_t) + size)) == NULL)
        return(NULL);

    node->pointers = (btnode_t **)(((uint8_t *)node) + sizeof(btnode_t));
    memset(node->pointers, 0, size);

    node->data = NULL;
    node->blocknr = 0;
    node->flags = 0;
    node->size = 0;
    node->refs = 1;

    return(node);
}

static btnode_t *__btnode_leaf_alloc (btree_t *btree) {
    btnode_t *node;

    if ((node = (btnode_t *) malloc(sizeof(btnode_t))) == NULL)
        return(NULL);

    node->pointers = NULL;
    node->data = NULL;
    node->blocknr = 0;
    node->flags = 0;
    node->size = 0;
    node->refs = 1;

    return(node);
}

static btnode_t *__btnode_alloc (btree_t *btree,
                                 uint8_t level)
{
    struct node_head *header;
    btnode_t *node;

    if ((node = __btnode_level_alloc(btree, level)) == NULL)
        return(NULL);

    /* Allocate Node Data */
    if ((node->data = (uint8_t *) malloc(__btree_block_size(btree))) == NULL) {
        free(node);
        return(NULL);
    }

    /* Initialize node header */
    header = __node_head(node);
    header->nh_crc = 0;
    header->nh_magic = __btree_node_magic(btree);
    header->nh_bmagic = 0; /* TODO & Check */
    header->nh_level = level;
    header->nh_items = 0;
    header->nh_free = __btree_node_space(btree);

    /* Update Super-Block with one more node */
    __btree_super(btree)->sb_node_count++;
    return(node);
}

static void __btnode_release (btree_t *btree,
                              btnode_t *node)
{
    if (node->refs <= 0) {
        fprintf(stderr, "assert: Node Release: %p %d\n", node, node->refs);
        abort();
    }

    if (--(node->refs) > 0)
        return;

    if (!__node_is_dirty(node) && __btcache_can_add(btree, &(btree->cache))) {
        __btcache_add(btree, &(btree->cache), node->blocknr, node->data);
        node->data = NULL;
    }
}

static int __btnode_free (btree_t *btree,
                          btnode_t *node)
{
    if (node->data != NULL)
        __btnode_release(btree, node);
    free(node);
    return(0);
}

static int __btnode_remove (btree_t *btree,
                            btnode_t *node)
{
    if (__node_is_on_disk(node))
        __btdisk_erase(btree, node->blocknr, node->size);

    if (node->data != NULL)
        free(node->data);

    free(node);
    return(0);
}

static btnode_t *__btnode_read (btree_t *btree,
                                btnode_t *node,
                                struct node_pointer *pointer)
{
    uint8_t *block;
    uint32_t crc;

    if (node->data != NULL)
        return(node);

    if ((node->data = __btcache_lookup(btree, &(btree->cache), pointer->np_blocknr)) != NULL)
        return(node);

    node->blocknr = pointer->np_blocknr;
    node->size = pointer->np_size;
    node->flags = 0;

    if ((node->data = (uint8_t *) malloc(__btree_block_size(btree))) == NULL) {
        perror("malloc()");
        return(NULL);
    }

    block = node->data;
    if (!__btdisk_read(btree, pointer->np_blocknr, block, pointer->np_size)) {
        fprintf(stderr, "assert: btree disk read\n");
        goto _read_error;
    }

    crc = __btdisk_crc_pointer(btree, block, pointer->np_size);
    if (pointer->np_crc != crc) {
        fprintf(stderr, "assert: pointer->crc failed\n");
        goto _read_error;
    }

    crc = __btdisk_crc_block(btree, NODE_CRC_OFFSET, block, pointer->np_size);
    if (__node_crc(node) != crc) {
        fprintf(stderr, "assert: node_crc() failed\n");
        goto _read_error;
    }

    return(node);

_read_error:
    node->data = NULL;
    free(block);
    return(NULL);
}

static btnode_t *__btnode_fetch (btree_t *btree,
                                 uint8_t level,
                                 struct node_pointer *pointer)
{
    btnode_t *node;

    if ((node = __btnode_level_alloc(btree, level)) == NULL)
        return(NULL);

    if (__btnode_read(btree, node, pointer) == NULL) {
        free(node);
        return(NULL);
    }

    return(node);
}

static btnode_t *__btnode_fetch_twig (btree_t *btree,
                                      btnode_t *node,
                                      uint32_t index)
{
    btnode_t *child;
    uint8_t level;

    if ((child = node->pointers[index]) != NULL) {
        child->refs++;
        return(__btnode_read(btree, child, __twig_pointer(btree, node, index)));
    }

    level = __node_level(node) + 1;
    child = __btnode_fetch(btree, level, __twig_pointer(btree, node, index));
    node->pointers[index] = child;
    return(child);
}

static btnode_t *__btree_fetch_root (btree_t *btree)
{
    uint8_t level;

    if (btree->root != NULL) {
        btree->root->refs++;
        return(__btnode_read(btree, btree->root, __btree_root_pointer(btree)));
    }

    level = __btree_height(btree);
    btree->root = __btnode_fetch(btree, level, __btree_root_pointer(btree));
    return(btree->root);
}

/* ===========================================================================
 *  PRIVATE Operations (Search)
 */
#define __btree_lookup_leaf(btree, key, place)                              \
    __btree_lookup_level(btree, key, LEAF_NODE_LEVEL, place)

#define __btree_lookup_twig(btree, key, place)                              \
    __btree_lookup_level(btree, key, TWIG_NODE_LEVEL, place)

static btnode_t *__btree_lookup_level (btree_t *btree,
                                       const void *key,
                                       uint32_t level,
                                       btnode_place_t *place)
{
    btnode_t *under_node;
    btnode_t *node;

    node = __btree_fetch_root(btree);
    while (node != NULL) {
        if (__node_is_internal(node))
            __twig_search(btree, node, key, place);
        else
            __leaf_search(btree, node, key, place);

        /* We've reach our level */
        if (__node_level(node) == level)
            return(node);

        /* If this is a leaf, there's no more hope */
        if (__node_is_leaf(node)) {
            __btnode_release(btree, node);
            return(NULL);
        }

        /* If isn't in... assert !found */
        if (place->index >= __node_items(node))
            place->index--;

        /* Read the underline node */
        under_node = __btnode_fetch_twig(btree, node, place->index);
        __btnode_release(btree, node);
        node = under_node;
    }

    return(NULL);
}

static btnode_t *__btnode_fetch_right (btree_t *btree,
                                       btnode_t *node)
{
    return(NULL);
}

static btnode_t *__btnode_fetch_left (btree_t *btree,
                                      btnode_t *node)
{
    return(NULL);
}

/* ===========================================================================
 *  PRIVATE Operations (Node Internal Update)
 */
static void __btree_twigs_update (btree_t *btree,
                                  btnode_t *node,
                                  const void *old_key,
                                  const void *new_key)
{
    struct node_pointer *pointer;
    btnode_place_t place;
    uint32_t level;

    level = __node_level(node);
    while (1) {
        if (!(node = __btree_lookup_level(btree, old_key, ++level, &place)))
            break;

        __node_set_to_update(node);
        if (place.found) {
            pointer = __twig_pointer(btree, node, place.index);
            __twig_replace(btree, node, place.index, new_key, pointer);
        }

        __btnode_release(btree, node);
    }
}

static void __btree_twigs_insert_update (btree_t *btree,
                                         btnode_t *node,
                                         uint32_t index,
                                         const void *new_key)
{
    const void *last_key;

    if (__node_is_leaf(node))
        last_key = __leaf_last_key(btree, node);
    else
        last_key = __twig_last_key(btree, node);

    if (index == __node_items(node))
        __btree_twigs_update(btree, node, last_key, new_key);
    else
        __btree_twigs_update(btree, node, last_key, last_key);
}

static void __btree_twigs_remove_update (btree_t *btree,
                                         btnode_t *node,
                                         uint32_t index)
{
    const void *last_key;
    const void *new_key;

    if (__node_is_leaf(node)) {
        last_key = __leaf_last_key(btree, node);
        new_key = __leaf_last_prev_key(btree, node);
    } else {
        last_key = __twig_last_key(btree, node);
        new_key = __twig_last_prev_key(btree, node);
    }

    if (index == (__node_items(node) - 1))
        __btree_twigs_update(btree, node, last_key, new_key);
    else
        __btree_twigs_update(btree, node, last_key, last_key);
}


/* ===========================================================================
 *  PRIVATE Operations (Node Insert)
 */
static btnode_t *__btree_grow (btree_t *btree) {
    struct node_pointer pointer = {0, 0, 0};
    btnode_t *new_root;
    btnode_t *old_root;
    const void *key;
    uint32_t height;

    height = __btree_height(btree);
    if ((new_root = __btnode_alloc(btree, height)) == NULL)
        return(NULL);

    if (height > 1) {
        old_root = __btree_fetch_root(btree);

        if (__node_is_leaf(old_root))
            key = __leaf_last_key(btree, old_root);
        else
            key = __twig_last_key(btree, old_root);

        /* Insert old root as child of new root */
        __twig_insert(btree, new_root, 0, key, &pointer);
        new_root->pointers[0] = old_root;

        __btnode_release(btree, old_root);
    }

    /* Set new root */
    btree->root = new_root;
    __btree_super(btree)->sb_height++;

    return(new_root);
}

static int __btree_insert_node (btree_t *btree,
                                btnode_t *node)
{
    struct node_pointer value = {0, 0, 0};
    btnode_place_t place;
    btnode_t *twig_right;
    btnode_t *twig_node;
    btnode_t *target;
    const void *last_key;
    const void *key;
    uint32_t index;

    if (__node_is_leaf(node))
        key = __leaf_last_key(btree, node);
    else
        key = __twig_last_key(btree, node);

    if (!(twig_node = __btree_lookup_level(btree, key,
                                           __node_level(node) + 1,
                                           &place)))
    {
        if ((twig_node = __btree_grow(btree)) == NULL)
            return(1);

        __twig_index_search(btree, twig_node, key, &index);
        __twig_insert(btree, twig_node, index, key, &value);
        twig_node->pointers[index] = node;

        __btnode_release(btree, twig_node);
        return(0);
    }

    /* Ok, this twig has space */
    if (__node_free(twig_node) >= __twig_needed(btree)) {
        __twig_index_search(btree, twig_node, key, &index);
        __btree_twigs_insert_update(btree, twig_node, index, key);
        __twig_insert(btree, twig_node, index, key, &value);

        twig_node->pointers[index] = node;
        __btnode_release(btree, twig_node);
        return(0);
    }

    /* if place.index < node.nitems - split twig */
    if (place.index < __node_items(twig_node)) {
        if (!(twig_right = __btnode_alloc(btree, __node_level(twig_node)))) {
            __btnode_release(btree, twig_node);
            return(2);
        }

        __twig_split(btree, twig_node, twig_right);
        __btree_twigs_update(btree, twig_node,
                            __twig_last_key(btree, twig_right),
                            __twig_last_key(btree, twig_node));

        /* Insert Key/Value into target node */
        target = __twig_split_target(btree, key, twig_node, twig_right);
        last_key = __twig_last_key(btree, target);

        __twig_index_search(btree, target, key, &index);
        __btree_twigs_insert_update(btree, target, index, key);
        __twig_insert(btree, target, index, key, &value);
        target->pointers[index] = node;

        if (__btree_insert_node(btree, twig_right)) {
            __twig_remove(btree, target, index);
            __twig_merge(btree, twig_node, twig_right);
            __btnode_release(btree, twig_node);
            __btnode_free(btree, twig_right);
            return(3);
        }

        __btnode_release(btree, twig_right);
    } else {
        if (!(twig_right = __btnode_alloc(btree, __node_level(twig_node)))) {
            __btnode_release(btree, twig_node);
            return(4);
        }

        __twig_insert(btree, twig_right, 0, key, &value);
        twig_right->pointers[0] = node;

        if (__btree_insert_node(btree, twig_right)) {
            __btnode_release(btree, twig_node);
            __btnode_free(btree, twig_right);
            return(5);
        }

        __btnode_release(btree, twig_right);
    }

    __btnode_release(btree, twig_node);

    return(0);
}

static int __btree_leaf_split_insert (btree_t *btree,
                                      btnode_t *node_left,
                                      const void *key,
                                      const void *value,
                                      uint32_t size)
{
    btnode_t *node_right;
    uint32_t size_needed;
    const void *last_key;
    btnode_t *target;

    /* Create the new right node */
    if ((node_right = __btree_leaf_node_alloc(btree)) == NULL)
        return(1);

    /* Split original node */
    __leaf_split(btree, node_left, node_right);
    __btree_twigs_update(btree, node_left,
                         __leaf_last_key(btree, node_right),
                         __leaf_last_key(btree, node_left));

    /* Insert right node into the tree or rollback */
    if (__btree_insert_node(btree, node_right)) {
        __leaf_merge(btree, node_left, node_right);
        __btnode_free(btree, node_right);
        return(2);
    }

    /* Calculate Item needed size */
    size_needed = __item_needed_size(btree, size);

    /* Check key node */
    target = __leaf_split_target(btree, key, node_left, node_right);
    last_key = __leaf_last_key(btree, target);

    if (__node_free(target) < size_needed) {
        /* TODO - Do better
         * Allocate an inner node between left and right.
         */
         if ((target = __btree_leaf_node_alloc(btree)) == NULL) {
            __btnode_release(btree, node_right);
            return(3);
         }

        __btree_twigs_insert_update(btree, target, 0, key);
        __leaf_insert(btree, target, 0, key, value, size);

        if (__btree_insert_node(btree, target)) {
            __btnode_release(btree, node_right);
            __btnode_free(btree, target);
            return(4);
        }
    } else {
        uint32_t index;

        __leaf_index_search(btree, target, key, &index);
        __btree_twigs_insert_update(btree, target, 0, key);
        __leaf_insert(btree, target, index, key, value, size);
    }

    __btnode_release(btree, node_right);
    return(0);
}

/* ===========================================================================
 *  PRIVATE Operations (Node Remove)
 */
static int __btree_remove_node (btree_t *btree,
                                btnode_t *node)
{
    btnode_place_t place;
    const void *last_key;
    btnode_t *parent;
    uint32_t level;
    int node_freed;

    if (__node_is_leaf(node))
        last_key = __leaf_last_key(btree, node);
    else
        last_key = __twig_last_key(btree, node);

    level = __node_level(node) + 1;
    if ((parent = __btree_lookup_level(btree, last_key, level, &place))) {
        if (__node_items(parent) == 1) {
            node_freed = __btree_remove_node(btree, parent);
        } else {
            node_freed = 0;

            __btree_twigs_remove_update(btree, parent, place.index);
            __twig_remove(btree, parent, place.index);
        }

        if (__node_is_leaf(node))
            __leaf_remove(btree, node, 0);
        else
            __twig_remove(btree, node, 0);

        __btree_super(btree)->sb_node_count--;
        __btnode_remove(btree, node);

        if (!node_freed)
            __btnode_release(btree, parent);
        return(1);
    } else if (__node_is_internal(node)) {
        __btree_twigs_remove_update(btree, node, place.index);
        __twig_remove(btree, node, place.index);
    }
    return(0);
}

/* ===========================================================================
 *  PRIVATE Operations (Node Sync)
 */
static int __btree_sync (btree_t *btree,
                         btnode_t *node,
                         struct node_pointer *pointer)
{
    uint32_t block_size;
    uint8_t *block;

    if (node->refs != 1) {
        fprintf(stderr, "node %p refs %d\n", node, node->refs);
        abort();
    }

    if (__node_is_internal(node)) {
        btnode_t *child;
        uint32_t i;
        int errno;

        if (!__node_need_update(node) && !__node_is_dirty(node))
            return(1);

        for (i = 0; i < __node_items(node); ++i) {
            if ((child = __btnode_fetch_twig(btree, node, i)) == NULL)
                return(-1);

            errno = __btree_sync(btree, child, pointer);
            __btnode_release(btree, child);

            if (errno < 0)
                return(errno);

            if (errno > 0)
                continue;

            __twig_inline_replace(btree, node, i, pointer);
        }
    }

    if (!__node_is_dirty(node))
        return(2);

    /* TODO: compress node->data
     */
    block = node->data;
    block_size = __btree_block_size(btree);

    /* Setup Node Flags */
    __node_set_on_disk(node);
    __node_set_clean(node);
    __node_head(node)->nh_crc = __btdisk_crc_block(btree, NODE_CRC_OFFSET, block, block_size);

    /* Setup Internal Node Pointer */
    pointer->np_crc = __btdisk_crc_pointer(btree, block, block_size);
    pointer->np_size = block_size;
    pointer->np_blocknr = __btdisk_write(btree, node->blocknr, node->size,
                                         block, block_size);

    /* Setup Blocknr to in-memory node */
    node->blocknr = pointer->np_blocknr;
    node->size = pointer->np_size;

    return(0);
}

/* ===========================================================================
 *  PRIVATE Operations (In-Memory Node Release)
 */
static void __btree_mem_nodes_release (btree_t *btree,
                                       btnode_t *node)
{
    if (node->refs != 0) {
        fprintf(stderr, "assert: __btree_mem_nodes_release refs != 1 %p %d\n", node, node->refs);
        abort();
    }

    if (node->pointers != NULL) {
        btnode_t *child;
        uint32_t i;

        for (i = 0; i < __btree_fanout(btree); ++i) {
            if ((child = node->pointers[i]) == NULL)
                continue;

            __btree_mem_nodes_release(btree, child);
        }
    }

    if (node->data != NULL) {
        printf("NOT FREED %p %d\n", node, node->refs);
    }

    __btnode_free(btree, node);
}

/* ===========================================================================
 *  PUBLIC Operations
 */
int btree_create_prefix (btree_t *btree,
                         btdisk_t *disk,
                         uint32_t cache_size,
                         uint64_t super_offset,
                         uint32_t block_size,
                         uint8_t format,
                         uint32_t prefix_size,
                         uint32_t key_size,
                         uint32_t btree_magic,
                         uint16_t node_magic,
                         memcopy_t prefix_keycpy,
                         compare_t prefix_keycmp,
                         compare_t keycmp,
                         void *user_data)
{
    struct super_block *super;

    /* Block size must be larger than:
     *      NODE_HEAD + (ITEM_HEAD + KEY + VALUE) * n
     */
    if (block_size < (NODE_HEAD_SIZE + 4 * (ITEM_HEAD_SIZE + (key_size << 2))))
        return(1);

    /* Initialize In-Memory Btree */
    btree->prefix_keycpy = prefix_keycpy;
    btree->prefix_keycmp = prefix_keycmp;
    btree->keycmp = keycmp;
    btree->user_data = user_data;
    btree->super_offset = super_offset;
    btree->root = NULL;
    btree->disk = disk;

    /* Initialize B*Tree Super-Block */
    super = __btree_super(btree);
    super->sb_magic = btree_magic;
    super->sb_node_magic = node_magic;

    super->sb_block_size = block_size;
    super->sb_prefix_size = prefix_size;
    super->sb_key_size = key_size;
    super->sb_height = LEAF_NODE_LEVEL;
    super->sb_format = format;

    super->sb_item_count = 0U;
    super->sb_node_count = 0U;
    super->sb_stored_data = 0U;

    super->sb_root = 0U;
    super->sb_root_size = 0U;
    super->sb_root_crc = 0U;

    /* Calculate Fanout */
    super->sb_fanout = __btree_node_space(btree) /
                       (__btree_prefix_size(btree)+NODE_POINTER_SIZE);

    /* Initialize block cache */
    __btcache_open(btree, &(btree->cache), cache_size);

    return(0);
}

int btree_create (btree_t *btree,
                  btdisk_t *disk,
                  uint32_t cache_size,
                  uint64_t super_offset,
                  uint32_t block_size,
                  uint8_t format,
                  uint32_t key_size,
                  uint32_t btree_magic,
                  uint16_t node_magic,
                  compare_t keycmp,
                  void *user_data)
{
    return(btree_create_prefix(btree, disk, cache_size,
                               super_offset,
                               block_size, format,
                               key_size, key_size,
                               btree_magic, node_magic,
                               NULL, keycmp, keycmp,
                               user_data));
}

int btree_prefix_open (btree_t *btree,
                       btdisk_t *disk,
                       uint32_t cache_size,
                       uint64_t super_offset,
                       uint32_t btree_magic,
                       uint16_t node_magic,
                       memcopy_t prefix_keycpy,
                       compare_t prefix_keycmp,
                       compare_t keycmp,
                       void *user_data)
{
    /* Initialize In-Memory Btree */
    btree->prefix_keycpy = prefix_keycpy;
    btree->prefix_keycmp = prefix_keycmp;
    btree->keycmp = keycmp;
    btree->user_data = user_data;
    btree->super_offset = super_offset;
    btree->root = NULL;
    btree->disk = disk;

    /* Initialize block cache */
    __btcache_open(btree, &(btree->cache), cache_size);

    if (!__btdisk_read(btree, super_offset, btree->super, SUPER_BLOCK_SIZE))
        return(1);

    if (__btree_magic(btree) != btree_magic) {
        fprintf(stderr, "assert: __btree_magic() failed.\n");
        return(3);
    }

    if (__btree_node_magic(btree) != node_magic) {
        fprintf(stderr, "assert: __btree_node_magic() failed.\n");
        return(4);
    }

    return(0);
}

int btree_open (btree_t *btree,
                btdisk_t *disk,
                uint32_t cache_size,
                uint64_t super_offset,
                uint32_t btree_magic,
                uint16_t node_magic,
                compare_t keycmp,
                void *user_data)
{
    return(btree_prefix_open(btree, disk, cache_size,
                             super_offset, btree_magic, node_magic,
                             NULL, keycmp,
                             keycmp, user_data));
}

int btree_close (btree_t *btree) {
    if (btree_sync(btree) < 0)
        return(-1);
    __btcache_close(btree, &(btree->cache));
    return(0);
}

int btree_sync (btree_t *btree) {
    struct node_pointer pointer;
    struct super_block *super;
    btnode_t *node;
    int errno;

    if (btree->root == NULL)
        return(0);

    node = __btree_fetch_root(btree);
    errno = __btree_sync(btree, node, &pointer);
    __btnode_release(btree, node);

    if (errno < 0)
        return(-1);

    if (errno > 0)
        return(1);

    super = __btree_super(btree);
    super->sb_root = pointer.np_blocknr;
    super->sb_root_size = pointer.np_size;
    super->sb_root_crc = pointer.np_crc;

    btree->super_offset = __btdisk_write(btree,
                                         btree->super_offset, SUPER_BLOCK_SIZE,
                                         super, SUPER_BLOCK_SIZE);

    /* Free In-Memory Nodes */
    __btree_mem_nodes_release(btree, btree->root);
    btree->root = NULL;

    return(0);
}

int btree_insert (btree_t *btree,
                    const void *key,
                    const void *value,
                    uint32_t size)
{
    btnode_place_t place;
    btnode_t *node_right;
    uint32_t size_needed;
    uint32_t free_space;
    btnode_t *node;

    /* If there's no root, add a new one and add this first key/value */
    if (btree->root == NULL) {
        if ((node = __btree_grow(btree)) == NULL)
            return(1);

        __leaf_insert(btree, node, 0, key, value, size);
        __btnode_release(btree, node);
        return(0);
    }

    /* Lookup Leaf node */
    node = __btree_lookup_leaf(btree, key, &place);

    if (node == NULL) {
        printf("ASSERT: __leaf_lookup must be not null\n");
        abort();
        return(-1);
    }

    if (!__node_is_leaf(node)) {
        printf("ASSERT: node is not leaf!\n");
        abort();
        return(-2);
    }

    /* We consider node free space without this key, that we've to replace */
    size_needed = __item_needed_size(btree, size);
    free_space  = __node_free(node);
    free_space += (place.found) ? __item_needed_size(btree, place.size) : 0U;

    if (node != NULL && place.size == size) {
        /* Inline Replace: Same size for old and new value */
        __leaf_inline_replace(btree, node, place.index, value, size);
    } else if (free_space >= size_needed) {
        /* Node has enough space to contains key/value */
        if (place.found) {
            __leaf_replace(btree, node, place.index, value, size);
        } else {
            __btree_twigs_insert_update(btree, node, place.index, key);
            __leaf_insert(btree, node, place.index, key, value, size);
        }
    } else if (place.index < __node_items(node)) {
        /* There's no enough space for key/value
         * but we've to insert between x and y item. Split node!
         */
        if (place.found)
            __leaf_remove(btree, node, place.index);   /* TODO: DO BETTER! */

        if (__btree_leaf_split_insert(btree, node, key, value, size)) {
            __btnode_release(btree, node);
            return(2);
        }
    }
#if 0
    else if ((node_right = __btnode_fetch_right(btree, node)) != NULL &&
               __node_free(node_right) >= size_needed)
    {
        /* Hei, right node has space for us */
        if (place.found)
            __leaf_remove(btree, node, place.index);

        __leaf_insert(btree, node_right, 0U, key, value, size);
    }
#endif
    else {
        /* There's no enough space for key/value,
         * but I can append key on a new node.
         */
        if (place.found)
            __leaf_remove(btree, node, place.index);

        /* Alloc new Node */
        if ((node_right = __btree_leaf_node_alloc(btree)) == NULL) {
            __btnode_release(btree, node);
            return(3);
        }

        /* Add item to New Node */
        __leaf_insert(btree, node_right, 0U, key, value, size);

        /* Insert Node into the tree */
        if (__btree_insert_node(btree, node_right)) {
            __btnode_release(btree, node);
            __btnode_free(btree, node_right);
            return(4);
        }

        __btnode_release(btree, node_right);
    }

    __btnode_release(btree, node);

    return(0);
}

int btree_remove (btree_t *btree,
                    const void *key)
{
    btnode_place_t place;
    btnode_t *node;

    if ((node = __btree_lookup_leaf(btree, key, &place)) == NULL) {
        return(1);
    }

    if (!place.found) {
        __btnode_release(btree, node);
        return(2);
    }

    if (!__node_is_leaf(node)) {
        fprintf(stderr, "ASSERT: Node is not leaf\n");
        abort();
        return(3);
    }

    if (__node_items(node) == 1) {
        if (__btree_node_count(btree) > 1) {
            btnode_t *root;

            if (!__btree_remove_node(btree, node)) {
                fprintf(stderr, "assert: node not removed\n");
                abort();
                return(4);
            }

            /* Decrease BTree level */
            root = __btree_fetch_root(btree);
            if (__node_items(root) == 1) {
                btnode_t *child;

                child = __btnode_fetch_twig(btree, root, 0);
                btree->root = child;
                __btree_super(btree)->sb_height--;
                __btree_super(btree)->sb_node_count--;
                __btnode_remove(btree, root);
                __btnode_release(btree, child);
            } else {
                __btnode_release(btree, root);
            }
        } else {
            __leaf_remove(btree, node, 0);
            __btnode_release(btree, node);
        }

        return(0);
    }

    /* TODO ALL THE TWIG UPDATES ARE WRONG
     * - leaf_last_key() before remove is not a good idea.
     */
    __btree_twigs_remove_update(btree, node, place.index);
    __leaf_remove(btree, node, place.index);

    __btnode_release(btree, node);

    return(0);
}

uint32_t btree_lookup (btree_t *btree,
                         const void *key,
                         void *buffer,
                         uint32_t size)
{
    btnode_place_t place;
    btnode_t *node;
    uint32_t x;

    if ((node = __btree_lookup_leaf(btree, key, &place)) == NULL)
        return(0);

    if (!place.found) {
        __btnode_release(btree, node);
        return(0);
    }

    x = (size > place.size) ? place.size : size;
    memcpy(buffer, place.value, x);

    __btnode_release(btree, node);
    return(x);
}

#ifdef __BTREE_DEBUG
static void __btree_debug (btree_t *btree,
                           btnode_t *node,
                           btree_key_debug_t key_debug,
                           btree_data_debug_t data_debug)
{
    if (node->refs != 1) {
        fprintf(stderr, "assert: node refs != 1 %d\n", node->refs);
        abort();
        return;
    }

    if (__node_is_leaf(node)) {
        __leaf_debug(btree, node, key_debug, data_debug);
    } else {
        btnode_t *child;
        uint32_t i;

        __twig_debug(btree, node, key_debug);
        for (i = 0; i < __node_items(node); ++i) {
            child = __btnode_fetch_twig(btree, node, i);
            __btree_debug(btree, child, key_debug, data_debug);
            __btnode_release(btree, child);
        }
    }
}

void btree_debug (btree_t *btree,
                    btree_key_debug_t key_debug,
                    btree_data_debug_t data_debug)
{
    btnode_t *node;

    printf("Debug B*Tree\n");
    printf(" - Height: %u\n", __btree_height(btree));
    printf(" - Fanout: %u\n", __btree_fanout(btree));
    printf(" - Items:  %"PRIu64"\n", __btree_item_count(btree));
    printf(" - Nodes:  %"PRIu64"\n", __btree_node_count(btree));
    printf(" - Stored: %"PRIu64"\n", __btree_stored_size(btree));

    node = __btree_fetch_root(btree);
    __btree_debug(btree, node, key_debug, data_debug);
    __btnode_release(btree, node);
}
#endif /* !__BTREE_DEBUG */
