static void *NF_pointer;

static void *find_fit(size_t asize)
{
    /* 답은 묵시적 가용 리스트에서  FirstFit 검색을 수행해야 함 */

    /* NextFit search */
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }

    return NULL;
}