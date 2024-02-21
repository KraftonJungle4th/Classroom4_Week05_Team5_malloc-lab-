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

size_t get_adjusted_size(old_size)
{
    size_t new_size;
    /* size를 adjusted size로 조정 (정렬조건 + 각종메타데이터를 포함) */
    if (old_size <= DSIZE)
        new_size = 2 * DSIZE;
    else
        new_size = ALIGN(old_size + DSIZE); // ALIGN사용
                                            // asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    return old_size
}