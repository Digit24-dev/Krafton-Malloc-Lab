/* Basic constants and macros */

#define WSIZE 4                 //  Word
#define DSIZE 8                 //  Double Word
#define CHUNKSIZE (1 << 12)     //  Least amount of extending heap size

#define MAX(x, y)           ((x > y) ? (x) : (y))
#define MIN(x, y)           ((x < y) ? (x) : (y))

// PACK
#define PACK(size, aloc)    ((size) | (aloc))

// typedef unsigned int size_pointer;

// Read & Write a word at address p
#define GET(p)              (*(unsigned int*)(p))
#define PUT(p, val)         (*(unsigned int*)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p)         (GET(p) & ~0x7)
#define GET_ALLOC(p)        (GET(p) &  0x1)

// Get Header and Footer address
#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// Get Next and Prev block size
#define NEXT_BLKP(bp)       ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)       ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// #define WSIZE           4
// #define DSIZE           8
// #define CHUNKSIZE       (1<<12)
// #define MAX(x, y)       ((x) > (y) ? (x) : (y))
// #define PACK(size, alloc)   ((size) | (alloc))
// #define GET(p)          (*(unsigned int *)(p))
// #define PUT(p, val)     (*(unsigned int *)(p) = (val))
// #define GET_SIZE(p)     (GET(p) & ~0x7)
// #define GET_ALLOC(p)    (GET(p) & 0x1)
// #define HDRP(bp)        ((char *)(bp) - WSIZE)
// #define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
// #define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
// #define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))