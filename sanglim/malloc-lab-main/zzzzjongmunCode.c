/*
 * < mm-implicit.c >
 * 이 방법은 묵시적 가용 리스트(Implicit Free List)를 사용하여 메모리 할당을 관리하는 malloc 패키지를 구현하였다.
 * 할당/가용 블록들은 헤더와 풋터를 통해 묵시적으로 연결되어 있다.
 * 이 방법에서는 더블 워드 정렬 조건을 사용하며, 모든 블록의 헤더와 풋터에 크기와 할당 비트를 저장한다. (1 Word = 4 Byte)
 * 최소 블록 크기는 4 Word (16 Byte)이며, 헤더와 풋터는 각각 1 Word이다.
 *
 * 사용 가능한 메모리 할당 정책은 다음과 같다.
 * 1. First Fit: 가용 블록 리스트에서 처음으로 적합한 블록을 찾는다.
 * 2. Best Fit: 가용 블록 리스트에서 가장 작은 적합한 블록을 찾는다.
 * 3. Next Fit: 마지막으로 할당된 블록에서부터 가용 블록을 찾는다.
 *
 */

/*
 * < 묵시적 가용 리스트 Implicit Free List >
 *
 *                                   heap_listp                                              last_bp                        mem_brk
 *                                       |                                                      |                              |
 *                                       |                                                      |                              |
 *                                       |                                                      |                              |
 *                                       V                                                      V                              V
 * -----------------------------------------------------------------------------------------------------------------------------
 * | Alignment Padding | Prologue Header | Prologue Footer | Free Block | ... | Free Block | ...| Free Block | Epilogue Header |
 * -----------------------------------------------------------------------------------------------------------------------------
 *
 * Alignment Padding: 8의 배수로 맞추기 위한 패딩 (값 = 0)
 * Prologue Header: 가용 블록의 시작을 나타내는 헤더 (값 = 8, 할당 비트 = 1)
 * Prologue Footer: 가용 블록의 끝을 나타내는 풋터 (값 = 8, 할당 비트 = 1)
 * Epilogue Header: 힙의 끝을 나타내는 헤더 (값 = 0, 할당 비트 = 1)
 *
 *
 * < 블록 구조 >
 * 31 . . . . . . . . . . . . . . . . . .  0            alloc bit = 001 : 할당 상태
 * -----------------------------------------            alloc bit = 000 : 가용 상태
 * | Block size                | alloc bit | Header
 * -----------------------------------------
 * |                Payload                |
 * |        (Allocated Block Only)         |
 * -----------------------------------------
 * |           Padding(Optional)           |
 * -----------------------------------------
 * | Block size                | alloc bit | Footer
 * -----------------------------------------
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/* 힙 메모리 할당 정책 */
// #define FIRST_FIT // 48 + 13 = 61
// #define BEST_FIT // 50 + 13 = 63
#define NEXT_FIT // 46 + 40 = 86

team_t team = {
    "호둘치",
    "정종문",
    "whdans4005@gmail.com",
    "백강민",
    "xxxxxxxxxx@gmail.com",
    "연선애",
    "xxxxxxxxxx@gmail.com",
};

// 단일 워드 (4) 또는 더블 워드 (8) 정렬
#define ALIGNMENT 8

// size를 8의 배수로 올림
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

// 기본 상수와 매크로들
#define WSIZE 4             // Word와 헤더와 풋터의 크기 4Byte
#define DSIZE 8             // 더블 워드의 크기 8Byte
#define CHUNKSIZE (1 << 12) // 초기 가용 블록과 힙 확장을 위한 기본 크기 4096Byte

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// 크기와 할당 비트를 통합해서 헤더와 풋터에 저장할 수 있는 워드를 반환
#define PACK(size, alloc) ((size) | (alloc))

// 주소 p에서 워드를 읽기 (p는 void *)
#define GET(p) (*(unsigned int *)(p))
// 주소 p에 val을 쓰기
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// 주소 p에서 크기를 읽기
#define GET_SIZE(p) (GET(p) & ~0x7)
// 주소 p에서 할당 비트를 읽기
#define GET_ALLOC(p) (GET(p) & 0x1)

// 블록 포인터의 헤더 주소를 반환
#define HDRP(bp) ((char *)(bp)-WSIZE)
// 블록 포인터의 풋터 주소를 반환
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// 다음 블록 포인터를 반환
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
// 이전 블록 포인터를 반환
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

/* 전역 변수 */
static void *heap_listp; // 힙의 최초 블록을 가리키는 포인터
static void *last_bp;    // 다음 삽입 위치를 가리키는 포인터 (next_fit()을 위한 변수)
/* 함수 프로토타입 */
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/*
 * mm_init - malloc 패키지 초기화. Prologue block과 Epilogue block을 생성하고 초기 힙을 구성한다.
 */
int mm_init(void)
{
    // 4 워드가 필요함
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                            // Alignment padding:
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // Epilogue header
    heap_listp += (2 * WSIZE);

    // CHUNKSIZE만큼의 가용 블록으로 초기 힙을 확장한다.
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    last_bp = heap_listp;
    return 0;
}

/* extend_heap - 힙을 확장하고 새로운 가용 블록을 생성한다.
 * 초기화 시에 매개변수 words로 1024이 들어온다면, size는 1024 * 4 = 4096이 된다.
 * 4096은 16진수로 0x1000이다. 따라서 program break의 주소는 0x1000만큼 증가한다.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    // 가용 블록의 크기는 8의 배수로 올림
    size = (words % 2 == 1) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    // 새로운 가용 블록의 헤더와 풋터를 초기화하고 새로운 에필로그 헤더를 설정
    PUT(HDRP(bp), PACK(size, 0));         // 가용 블록 헤더
    PUT(FTRP(bp), PACK(size, 0));         // 가용 블록 풋터
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 새로운 에필로그 헤더

    // 이전 블록이 가용 상태이면 통합
    return coalesce(bp);
}

void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * coalesce - 경계 태그 합치기. 합쳐진 블록의 포인터를 반환한다.
 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // 이전 블록의 할당 상태 (0 or 1)
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // 다음 블록의 할당 상태 (0 or 1)
    size_t size = GET_SIZE(HDRP(bp));                   // 현재 블록의 크기

    if (prev_alloc && next_alloc)
    { // Case 1: 이전 블록과 다음 블록이 모두 할당 상태
        last_bp = bp;
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    { // Case 2: 이전 블록은 할당 상태이고 다음 블록은 가용 상태
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    { // Case 3: 이전 블록은 가용 상태이고 다음 블록은 할당 상태
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    { // Case 4: 이전 블록과 다음 블록이 모두 가용 상태
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    last_bp = bp;
    return bp;
}
/*
 * mm_malloc - 블럭을 할당하고 적합한 블록을 찾지 못했을 때 힙을 확장한다.
 * 블록의 크기는 최소 16바이트 크기의 블록으로 구성
 * 8바이트는 정렬 조건을 만족하기 위해, 8바이트는 헤더와 풋터를 위해 사용
 */
void *mm_malloc(size_t size) // size는 헤더와 풋터를 제외한 블록의 크기
{
    size_t asize;      // 헤더와 풋터를 포함한 조정된 블록의 크기
    size_t extendsize; // 적합한 블록을 찾지 못했을 때 힙을 확장하는 양
    char *bp;

    // 가짜 블록을 할당하지 않음
    if (size == 0)
        return NULL;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size + DSIZE); // 헤더와 풋터를 포함하여 8의 배수로 올림

    // 가용 리스트에서 적합한 블록을 찾음
    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        last_bp = bp;
        return bp;
    }

    // 적합한 블록을 찾지 못했을 때 힙을 확장하고 새로운 블록을 할당
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    last_bp = bp;
    return bp;
}

static void *find_fit(size_t asize)
{
    void *bp = last_bp;

    for (bp = NEXT_BLKP(bp); GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            last_bp = bp;
            return bp;
        }
    }

    // 끝까지 갔는데 할당가능한 가용 블록이 없으면 처음부터 마지막으로 할당한 블록 전까지 탐색
    bp = heap_listp;
    while (bp < last_bp)
    {
        bp = NEXT_BLKP(bp);

        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            last_bp = bp;
            return bp;
        }
    }
    return NULL; // No fit
}

/*
 * place - 가용 블록의 시작 부분에 asize 바이트의 블록을 배치하고 나머지가 최소 블록 크기 이상이면 분할한다.
 */
static void place(void *bp, size_t allocate_size) // allocate_size는 헤더와 풋터를 포함한 블록의 크기
{
    size_t chunk_size = GET_SIZE(HDRP(bp));

    if ((chunk_size - allocate_size) >= (2 * DSIZE))
    { // 가용 블록에 할당을 하고 남은 공간이 최소 블록 크기 이상이면 분할
        PUT(HDRP(bp), PACK(allocate_size, 1));
        PUT(FTRP(bp), PACK(allocate_size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(chunk_size - allocate_size, 0));
        PUT(FTRP(bp), PACK(chunk_size - allocate_size, 0));
        coalesce(bp);
    }
    else
    { // 가용 블록에 할당을 하고 남은 공간이 최소 블록 크기보다 작으면 분할하지 않음
        PUT(HDRP(bp), PACK(chunk_size, 1));
        PUT(FTRP(bp), PACK(chunk_size, 1));
    }
}

/*
 * mm_realloc - 기존 블록을 새로운 크기로 재할당한다.
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr; // 이전 포인터
    void *newptr;       // 새로 메모리 할당할포인터

    size_t originsize = GET_SIZE(HDRP(oldptr)); // 원본 사이즈
    size_t newsize = size + DSIZE;              // 새 사이즈

    // 새로운 사이즈가 원본 사이즈보다 작거나 같으면
    if (newsize <= originsize)
        return oldptr;
    else
    {                                                                    // 기존 블록의 다음 블록이 가용 블록이고, 추가 사이즈가 충분하면
        size_t addSize = originsize + GET_SIZE(HDRP(NEXT_BLKP(oldptr))); // 추가 사이즈 -> 헤더 포함 사이즈
        if (!GET_ALLOC(HDRP(NEXT_BLKP(oldptr))) && (newsize <= addSize))
        {                                        // 가용 블록이고 사이즈 충분
            PUT(HDRP(oldptr), PACK(addSize, 1)); // 새로운 헤더
            PUT(FTRP(oldptr), PACK(addSize, 1)); // 새로운 푸터
            return oldptr;
        }
        else // 새로운 블록 할당
        {
            newptr = mm_malloc(newsize);
            if (newptr == NULL)
                return NULL;
            memcpy(newptr, oldptr, newsize);
            mm_free(oldptr);
            return newptr;
        }
    }
}