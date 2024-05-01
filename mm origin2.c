/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"
#include "macros.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "TEAM 7",
    /* First member's full name */
    "JAEHYEOK CHOI",
    /* First member's email address */
    "digi7442@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// #define NEXTFIT

void *next_fitp;
void *heap_listp;

static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* CASE 1 */
    if (prev_alloc && next_alloc) return bp;

    /* CASE 2 */
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    /* CASE 3 */
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp)           , PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /* CASE 4 */
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    next_fitp = bp;
    return bp;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size = WSIZE * ALIGN(words);

    // size = (words & 0x1) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1) 
        return NULL;

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // padding, prologue, epilogue -> 4 Words.
    // error handling
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1) return -1;

    PUT(heap_listp , 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));
    
    heap_listp += (2 * WSIZE);

    next_fitp = heap_listp;

    // extending heapsize
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        // place code 0
        PUT(HDRP(NEXT_BLKP(bp)), PACK(csize - asize, 0));    // set Header, 0
        PUT(FTRP(NEXT_BLKP(bp)), PACK(csize - asize, 0));    // set Footer, 0        

        // place code 1 - 10% faster than place 0
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));    // set Header, 0
        PUT(FTRP(bp), PACK(csize - asize, 0));    // set Footer, 0
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void *find_fit3(size_t asize)
{
    void *bp, *best_fit_bp = NULL;
    size_t min_size = SIZE_MAX;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))) && min_size > GET_SIZE(HDRP(bp))){
            best_fit_bp = bp;
            // printf("%d vs %d \n", GET_SIZE(HDRP(best_fit_bp)), GET_SIZE(HDRP(bp)));
        }
    }

    if (best_fit_bp == NULL) {
        return NULL;
    }
    return best_fit_bp;
}

#define NEXTFIT
// #define FIRSTFIT    1
#define MINE

// first fit / next fit
static void *find_fit(size_t asize)
{
#ifdef  NEXTFIT
#ifdef  MINE
    void *bp;
    for (bp = next_fitp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            next_fitp = bp;
            return bp;
        }
    }
    // for (bp = PREV_BLKP(next_fitp); bp != heap_listp; bp = PREV_BLKP(bp))
    // {
    //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
    //         next_fitp = bp;
    //         return bp;
    //     }
    // }
    
    return NULL;
#else
    void *bp;
    void *old_pointp = next_fitp;
    for (bp = next_fitp; GET_SIZE(HDRP(bp)); bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            next_fitp = NEXT_BLKP(bp);
            return bp;
        }
    }
    for (bp = heap_listp; bp < old_pointp; bp = NEXT_BLKP(bp))
    {
        if ((!GET_ALLOC(HDRP(bp))) && GET_SIZE(HDRP(bp)) >= asize)
        {
            next_fitp = NEXT_BLKP(bp);
            return bp;
        }
    }
    return NULL;
#endif
#elif   FIRSTFIT
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) ; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
#else   // BESTFIT
    void *bp, *best_fit_bp = NULL;
    size_t min_size = SIZE_MAX;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))) && min_size > GET_SIZE(HDRP(bp))){
            best_fit_bp = bp;
            // printf("%d vs %d \n", GET_SIZE(HDRP(best_fit_bp)), GET_SIZE(HDRP(bp)));
        }
    }

    if (best_fit_bp == NULL) {
        return NULL;
    }
    return best_fit_bp;
#endif
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // int newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);
    // if (p == (void *)-1)
	// return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }

    if (size == 0)
        return NULL;

    size_t asize;

    /* if data is lower than DSIZE, 
    we have to have at least DSIZE(8 bytes, 2 * WSIZE) of data 
    because of Header and Footer. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        // asize = DSIZE * ALIGN(size);
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    char *bp;
    // find the fit to place the data.
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    size_t extendsize;
    // if couldn't find the fit,
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;

    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc11(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memmove(newptr, oldptr, copySize);
    mm_free(oldptr);
    // next_fitp = oldptr;
    return newptr;
}

void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    size_t csize = GET_SIZE(HDRP(ptr));
    size_t nsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    // size_t asize = size + 2*WSIZE;
    size_t asize;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ALIGN(size);
        // asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    if (asize < csize) {
        return ptr;
    }
    else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) && csize + nsize >= asize && csize + nsize < asize) {
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        return ptr;
    }
    // 분할
    else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) && csize + nsize > asize) {
        // PUT(HDRP(ptr), PACK(csize + nsize, 1));
        // PUT(FTRP(ptr), PACK(csize + nsize, 1));
        
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        void *nblk_ptr = NEXT_BLKP(ptr);
        PUT(HDRP(nblk_ptr), PACK(csize + nsize - asize , 0));
        PUT(FTRP(nblk_ptr), PACK(csize + nsize - asize , 0));
        // next_fitp = ptr; // mem util - 3%
        return ptr;
    }
    // 메모리 끝 부분, 필요한 만큼만 extend
    // else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0) {
    //     extend_heap(asize - csize);
    //     PUT(HDRP(ptr), PACK(asize, 1));
    //     PUT(FTRP(ptr), PACK(asize, 1));
    //     return ptr;
    // }
    // 메모리 끝 전전 부분, csize + nsize가 asize 보다 작고 nsize가 epilogue 직전 블록인 경우
    // else if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) && GET_SIZE(HDRP(NEXT_BLKP(NEXT_BLKP(ptr)))) == 0 && csize + nsize < asize) {
    //     extend_heap(CHUNKSIZE/WSIZE);
    //     PUT(HDRP(ptr), PACK(asize, 1));
    //     PUT(FTRP(ptr), PACK(asize, 1));
    //     return ptr;
    // }
    else {
        void *newptr = mm_malloc(size + DSIZE);
        if (newptr == NULL) return NULL;
        memmove(newptr, ptr, size + DSIZE);
        // next_fitp = newptr;
        mm_free(ptr);
        return newptr;
    }
}

void *mm_realloc33(void *ptr, size_t size) // 재할당
{
    if (ptr == NULL) // 입력 포인터가 NULL이면, 입력 사이즈만큼 새롭게 할당 (예외처리)
    {
        return mm_malloc(size);
    }
    if (size == 0) // 입력 사이즈가 0이면, 입력 포인터의 블록을 해제 (예외처리)
    {
        mm_free(ptr);
        return NULL;
    }
    void *oldptr = ptr;
    void *newptr;
    size_t copySize = GET_SIZE(HDRP(oldptr)); // 재할당하려는 블록의 사이즈
    if (size + DSIZE <= copySize) // (재할당 하려는 블록 사이즈 + 8 bytes(Header + Footer)) <= 현재 블록 사이즈
    {
        return oldptr; // 현재 블록에 재할당해도 문제 없기 때문에, 포인터만 반환
    }
    else // (재할당 하려는 블록 사이즈 + 8 bytes) > 현재 블록 사이즈
         // 경우에 따라서 인접 Free block을 활용하는 방안과, 새롭게 할당하는 방안을 이용해야 함
    {
        size_t next_size = copySize + GET_SIZE(HDRP(NEXT_BLKP(oldptr))); // 현재 블록 사이즈 + 다음 블록 사이즈 = next_size
        if (!GET_ALLOC(HDRP(NEXT_BLKP(oldptr))) && (size + DSIZE <= next_size))
        // 다음 블록이 Free block이고, (재할당 하려는 블록의 사이즈 + 8 bytes) <= (현재 블록 사이즈 + 다음 블록 사이즈)
        // 현재 블록과 다음 블록을 하나의 블록으로 취급해도 크기의 문제가 발생하지 않음
        // malloc을 하지 않아도 됨 -> 메모리 공간 및 시간적 이득을 얻을 수 있음
        {
            PUT(HDRP(oldptr), PACK(next_size, 1)); // 현재 블록의 Header Block에, (현재 블록 사이즈 + 다음 블록 사이즈) 크기와 Allocated 상태 기입
            PUT(FTRP(oldptr), PACK(next_size, 1)); // 현재 블록의 Footer Block에, (현재 블록 사이즈 + 다음 블록 사이즈) 크기와 Allocated 상태 기입
            next_fitp = oldptr; // next_fit 사용을 위한 포인터 동기화
            return oldptr;
        }
        // else if (!GET_ALLOC(HDRP(PREV_BLKP(oldptr))) && ())
        else // 위 케이스에 모두 해당되지 않아, 결국 malloc을 해야 하는 경우
        {
            newptr = mm_malloc(size + DSIZE); // (할당하려는 크기 + 8 bytes)만큼 새롭게 할당
            if (newptr == NULL) // 새로 할당한 주소가 NULL일 경우 (예외처리)
            {
                return NULL;
            }
            memmove(newptr, oldptr, size + DSIZE); // payload 복사
            next_fitp = newptr; // next_fit 사용을 위한 포인터 동기화
            mm_free(oldptr); // 기존의 블록은 Free block으로 바꾼다
            return newptr; // 새롭게 할당된 주소의 포인터를 반환
        }
    }
}