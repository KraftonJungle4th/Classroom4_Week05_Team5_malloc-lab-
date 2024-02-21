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

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
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
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
#define MAX(x,y) ((x) > (y) ? (x):(y))
#define PACK(size,alloc) ((size) | (alloc)) //크기와 할당 여부 4바이트로 한번에 표현,비트 계산으로 해야 속도가 빠르다
#define GET(p) (*(unsigned int*)(p)) // void형 포인터라 직접 역참조가 불가능해 타입 캐스팅해줌
#define PUT(p,val) (*(unsigned int*)(p) = (val)) //p에 val 저장함
#define GET_SIZE(p) (GET(p) & ~0x7) // 뒤에 3비트개 없애줌
#define GET_ALLOC(p) (GET(p) & 0x1) // a/f bit 빼고 없애줌
#define HDRP(bp) ((char*)(bp)) - WSIZE // bp는 payload의 시작을 가리킴
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //payload 시작점 + 해당 블록 크기 - 2Word = footer
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))// bp에서 자기 블록 크기 더하기
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE))) // bp에서 이전블록 크기 빼기,헤더 다음 포인팅

// 전방 선언해줌
static void *heap_listp;//할당기
static void *extend_heap(size_t words);
static void *coalesce(void* ptr);
static void *find_fit(size_t asize);
static void place(void*ptr,size_t asize);
static void* next_fit(size_t asize);
static void* temp = NULL;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) //padding + prog_header + prog_footer + epil_header 4words 필요
{   
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1) // (void*) -1은 포인터가 가리키는 값이 유효 하지 않음
        return -1;
    PUT(heap_listp,0); //alignment padding
    PUT(heap_listp + (1*WSIZE),PACK(DSIZE,1)); //Prologue header
    PUT(heap_listp + (2*WSIZE),PACK(DSIZE,1)); //Prologue footer -> 헤더를 복사했기 때문에 값이 같다
    PUT(heap_listp + (3*WSIZE),PACK(0,1)); //epilogue header
    heap_listp += (2*WSIZE); //after prologue block의 끝 포인팅

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) //실패시
        return -1;

    return 0;
}

static void* extend_heap(size_t words){
    char* bp;
    size_t size;

    size = (words%2) ? (words +1) * WSIZE : words * WSIZE; //추가 사이즈가 홀이면 짝수로 만들어줌-> double words align
    if((long)(bp = mem_sbrk(size)) == -1)// 늘리기 실패시 ////why long?
        return NULL;

    PUT(HDRP(bp),PACK(size,0)); //가용블록 헤더
    PUT(FTRP(bp),PACK(size,0)); //가용 블록 푸터
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));//끝 삽입

    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      // 조정된 블록 사이즈
    size_t extendsize; // 확장할 사이즈
    char *bp;

    // 잘못된 요청 분기
    if (size == 0)
        return NULL;

        /* 사이즈 조정 2word 단위*/
    if (size <= DSIZE)     // 8바이트 이하이면
        asize = 2 * DSIZE; // 최소 블록 크기 16바이트 할당 (헤더 4 + 푸터 4 + 저장공간 8)
    else // 8바이트 초과
        asize = DSIZE * ((size + DSIZE + DSIZE - 1) / DSIZE); // 8배수로 올림 처리

        /* 가용 블록 검색 */
    if (bp = next_fit((asize)) != NULL)
    {
        place(bp, asize); // 할당
        return bp;        // 새로 할당된 블록의 포인터 리턴
    }

    /* 적합한 블록이 없을 경우 확장 요청*/
    extendsize = MAX(asize, CHUNKSIZE); //요청받은 사이즈(조정된)와 기본 단위중 더 큰 사이즈(외부 단편화 방지)
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);//할당
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr),PACK(size,0)); //0으로 할당
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);//반환후 주위 블록 검사
}


static void *coalesce(void *bp) // 반환시 주변에 합칠 것 있음 합치기
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // 이전 블록 할당 상태,footer 이용
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // 다음 블록 할당 상태,header 이용
    size_t size = GET_SIZE(HDRP(bp));                   // 현재 블록 사이즈

    if (prev_alloc && next_alloc) // 모두 할당된 경우
        return bp;

    else if (prev_alloc && !next_alloc) // 다음 블록만 빈 경우
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));//다음 블록 사이즈만큼 증가
        PUT(HDRP(bp), PACK(size, 0)); // 현재 블록 헤더 재설정
        PUT(FTRP(bp), PACK(size, 0)); // 다음 블록 푸터 재설정 (위에서 헤더를 재설정했으므로, FTRP(bp)는 합쳐질 다음 블록의 푸터가 됨)
    }
    else if (!prev_alloc && next_alloc) // 이전 블록만 빈 경우
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); // 이전 블록 헤더 재설정
        PUT(FTRP(bp), PACK(size, 0));            // 현재 블록 푸터 재설정
        bp = PREV_BLKP(bp);                      // 이전 블록의 시작점으로 포인터 변경
    }
    else // 이전 블록과 다음 블록 모두 빈 경우
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); // 이전 블록 헤더 재설정
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0)); // 다음 블록 푸터 재설정
        bp = PREV_BLKP(bp);                      // 이전 블록의 시작점으로 포인터 변경
    }

    return bp; // 병합된 블록의 포인터 반환
}



static void *find_fit(size_t asize)
{
    void *bp = mem_heap_lo() + 2 * WSIZE; // 첫번째 블록(주소: 힙의 첫 부분 + 8bytes)부터 탐색 시작
    while (GET_SIZE(HDRP(bp)) > 0)//해당 블록 사이즈가 0보다 크다면
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) // 가용 상태이고, 사이즈가 적합하면
            return bp;                                             // 해당 블록 포인터 리턴
        bp = NEXT_BLKP(bp);                                        // 조건에 맞지 않으면 다음 블록으로 이동해서 탐색을 이어감
    }
    return NULL;
}

// static void* next_fit(size_t asize){
//     void* bp;

//     if(temp == NULL){
//         return find_fit(asize);
//     }else{
//         bp = NEXT_BLKP(temp);
//     }

//     while(GET_SIZE(bp) > 0){ // 해당 블록 사이즈 0보다 크고
//         if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){ // free, 현재 사이즈가 클때
//             return bp;
//         }
//         bp = NEXT_BLKP(bp);
//     }

//     return NULL;
// }

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp)); // 현재 블록의 크기

    if ((csize - asize) >= (2 * DSIZE)) // 차이가 최소 블록 크기 16보다 같거나 크면 분할,외부 단편화 방지
    {
        PUT(HDRP(bp), PACK(asize, 1)); // 현재 블록에는 필요한 만큼만 할당
        PUT(FTRP(bp), PACK(asize, 1));

        PUT(HDRP(NEXT_BLKP(bp)), PACK((csize - asize), 0)); // 남은 크기를 다음 블록에 할당(가용 블록)
        PUT(FTRP(NEXT_BLKP(bp)), PACK((csize - asize), 0));
    }
    else //남는 공간이 4워드 보다 작을 때(header+footer + 2words payload)
    {
        PUT(HDRP(bp), PACK(csize, 1)); // 해당 블록 전부 사용
        PUT(FTRP(bp), PACK(csize, 1));
    }
}


// 기존에 할당된 메모리 블록의 크기 변경
// `기존 메모리 블록의 포인터`, `새로운 크기`
void *mm_realloc(void *ptr, size_t size)
{
        /* 예외 처리 */
    if (ptr == NULL) // 포인터가 NULL인 경우 할당만 수행
        return mm_malloc(size);

    if (size <= 0) // size가 0인 경우 메모리 반환만 수행
    {
        mm_free(ptr);
        return 0;
    }

        /* 새 블록에 할당 */
    void *newptr = mm_malloc(size); // 새로 할당한 블록의 포인터
    if (newptr == NULL)
        return NULL; // 할당 실패

        /* 데이터 복사 */
    size_t copySize = GET_SIZE(HDRP(ptr)) - DSIZE; // payload만큼 복사
    if (size < copySize)                           // 기존 사이즈가 새 크기보다 더 크면
        copySize = size;                           // size로 크기 변경 (기존 메모리 블록보다 작은 크기에 할당하면, 일부 데이터만 복사)

    memcpy(newptr, ptr, copySize); // 새 블록으로 데이터 복사, malloc은 header 다음부터 포인팅

        /* 기존 블록 반환 */
    mm_free(ptr);

    return newptr;
}





