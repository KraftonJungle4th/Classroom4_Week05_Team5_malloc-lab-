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

/* Implicit NextFit */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "JGTeam",
    /* First member's full name */
    "DW_K",
    /* First member's email address */
    "Test1",
    /* Second member's full name (leave blank if none) */
    "SJ_L",
    /* Second member's email address (leave blank if none) */
    "Test2",
    /* Third member's full name */
    "SL_L",
    /* Third member's email address */
    "Test3",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 커스텀 */
#define WSIZE 4 /* Word and header/footer size (Bytes) */
#define DSIZE 8 /* Double Word size (Bytes) */
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* size와 할당비트 */
#define PACK(size, alloc) ((size) | (alloc))

/* word를 p에서 읽음 */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* size, field를 포인터로 읽음 */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* 블록포인터로부터, 이 블록의 header와 footer 계산 */
#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* 블록포인터로부터, 다음과 이전 블록의 주소 계산 */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/* 커스텀 변수 */
static char *heap_listp = NULL; // 얘는 따로 해야하나?
static void *NF_pointer = NULL;

/* 함수 프로토타입 선언 */
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static size_t get_adjusted_size(old_size);
int mm_init(void);
void mm_free(void *bp);
void *mm_malloc(size_t size);
void *mm_realloc(void *ptr, size_t size);

/*
 * mm_init - 특정 포인터로 시작하는 배열을 malloc패키지 형태로 포맷. initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                            // 정렬 패딩
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // 프롤로그, 헤더
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); //  프롤로그, 푸터
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // 에필로그 헤더
    heap_listp += (2 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    NF_pointer = heap_listp;
    return 0;
}

/* extend_heap - 할당이 추가로 필요할 때, 전체 배열을 늘리는 함수 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* 더블워드 정렬을 위해, 홀수word일 경우, 추가로 size를 부여 */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* 확장된 힙에 대해, header, footer, 새로운 epilogue header를 부여 및 초기화 */
    PUT(HDRP(bp), PACK(size, 0));         // header
    PUT(FTRP(bp), PACK(size, 0));         // footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 새 eplg header

    /* 힙에 추가 할당 시, 이전할당블록이 가용상태(free)면 합체 */
    return coalesce(bp);
}

/* mm_free - 메모리는 이제 자유에요 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/* 가용상태로 바뀐 힙블록을 받아와, 주변 블록들의 상태에 따라 합체시키는 함수                   *
 * 모든 가용상태 : 힙이 늘어날 때 (extend), place시 반으로 분절할 때 (place), free할 때 (free)*/
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /* CASE 1 : 이전과 다음 힙블록이 할당중일때 */
    if (prev_alloc && next_alloc)
    {
    }

    /* CASE 2 : 다음 힙블록이 가용상태(state : free) */
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    /* CASE 3 : 이전 힙블록이 가용상태*/
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /* CASE 4 : 이전힙블록, 다음힙블록 둘 다 가용상태*/
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    NF_pointer = bp;
    return bp;
}

/* mm_malloc - brk 포인터가 증가함에 따라, 힙블록을 할당.                    *
 *                     항상 정렬조건의 배수로 블록을 할당하라.                        *
 *     Allocate a block by incrementing the brk pointer.                         *
 *     Always allocate a block whose size is a multiple of the alignment. */
void *mm_malloc(size_t size)
{
    size_t asize;      //  정렬조건에 맞게 포맷 될, 조정된 블록사이즈
    size_t extendsize; // malloc할 사이즈와 가용사이즈가 맞지않을경우 늘려서, 늘려진 전체사이즈
    char *bp;

    /* 비논리적 요청무시 */
    if (size == 0)
        return NULL;

    /* size를 adjusted size로 조정 (정렬조건 + 각종메타데이터를 포함) */
    asize = get_adjusted_size(size);

    /* 조정된 사이즈에 맞는 가용상태의 리스트 탐색 */
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        NF_pointer = bp; //  placee하고 나서도 NF 배치가 필요하다
        return bp;
    }

    /* 알맞은 크기가 없을경우, 힙을 더 늘려야함. 더 많은 메모리를 요청 */
    extendsize = MAX(asize, CHUNKSIZE);

    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize); //  placee하고 나서도 NF 배치가 필요하다
    NF_pointer = bp;  //  placee하고 나서도 NF 배치가 필요하다
    return bp;
}

static void *find_fit(size_t asize)
{
    void *bp = NF_pointer;

    /* NextFit search1 - 저장된 지점부터 찾음 */
    while (GET_SIZE(HDRP(bp)) > 0) // size가 0인 epilogue만나면 나가짐
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) // 할당가능조건
        {
            NF_pointer = NEXT_BLKP(bp); // 찾은 후 NF_pointer 업데이트
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }

    /* NextFit search2 - heap의 처음부터 찾음 */
    bp = heap_listp;
    while (bp < NF_pointer)
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) // 할당가능조건
        {
            NF_pointer = NEXT_BLKP(bp); // 찾은 후 NF_pointer 업데이트
            return bp;
        }
        bp = NEXT_BLKP(bp);
    }

    return NULL;
}

static void place(void *bp, size_t asize)
{
    /* 답은 요청한 블록을 가용 블록의 시작부분에 배치해야 하며, 나머지   *
     * 부분의 크기가 최소 블록 크기와 같거나 큰 경우에만 분할해야 한다. */

    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2 * DSIZE))
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        // coalesce(bp); // 이거 빼도된다는데 왜? 가용+가용이 일어나는 경우가 없다 (free 시에 coalesce가 연달아 일어나므로)
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        bp = NEXT_BLKP(bp);
    }
}

/* mm_realloc - ptr를 반환하고 새로운 size의 힙메모리 할당.                    *
 *     Implemented simply in terms of mm_malloc and mm_free.            */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;

    if (oldptr == NULL) // 포인터가 NULL인 경우 할당만 함
        return mm_malloc(size);

    void *newptr = mm_malloc(size);
    if (newptr == NULL) // 할당실패.
        return NULL;

    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    size_t asize = get_adjusted_size(size);
    size_t copySize = GET_SIZE(HDRP(oldptr)) - DSIZE; //  header에서 payload사이즈 추출
    if (size < copySize)
        copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    return newptr;
}

static size_t get_adjusted_size(old_size)
{
    size_t new_size;
    /* size를 adjusted size로 조정 (정렬조건 + 각종메타데이터를 포함) */
    if (old_size <= DSIZE)
        new_size = 2 * DSIZE;
    else
        new_size = ALIGN(old_size + DSIZE); // ALIGN사용
                                            // asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    return new_size;
}
