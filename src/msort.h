#ifndef INCLUDED_MSORT
#define INCLUDED_MSORT
/*
 * Copyright (C) 2013 ys7010 All Rights Reserved.
 * 
 * Merge sort Version 1.10
 */

/* header file */
#include <stddef.h>

/**
 * in-place merge sort.
 * 
 * @param base  start of target array.
 * @param n     array size in elements.
 * @param size  element size in bytes.
 * @param cmp   comparison function.
 * @return      nonzero on success, zero on failure.
 */
int msort(
    void *base,
    size_t n,
    size_t size,
    int (*cmp)(const void *, const void *)
);
#endif /* INCLUDED_MSORT */
