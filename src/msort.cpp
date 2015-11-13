/*
 * Copyright (C) 2013 ys7010 All Rights Reserved.
 * 
 * Merge sort Version 1.10
 */

/* header file */
#include <stdlib.h>
#include <string.h>
#include "msort.h"

/* constant definition */
#define THRESHOLD 16

/* global variable */
static size_t gSiz;
static int (*gCmp)(const void *, const void *);
static void *gTmp;

/* function prototype */
static void mergesort(char *base, size_t n);
static void merge(char *base, size_t m, size_t n);
static size_t searchkey(const char *base, const char *key, size_t n);
static void rotate(char *base, size_t move, size_t n);
static unsigned int gcd(unsigned int x, unsigned int y);
static void insertionsort(char *base, size_t n);

/**
 * in-place merge sort.
 * 
 * @param base  start of target array.
 * @param n     array size in elements.
 * @param size  element size in bytes.
 * @param cmp   comparison function.
 * @return      nonzero on success, zero on failure.
 */
int msort(void *base, size_t n, size_t size,
    int (*cmp)(const void *, const void *)) {
    if ((gTmp = malloc(size)) != NULL) {
        gSiz = size;
        gCmp = cmp;
        mergesort((char *)base, n);
        if(gTmp != NULL){
        	free(gTmp);
        	gTmp = NULL;
        }
        return !0;
    }
    return 0;
}

/**
 * bottom up merge sort.
 * 
 * @param base  start of target array.
 * @param n     array size in elements.
 */
static void mergesort(char *base, size_t n) {
    size_t i, j, m = n - n % THRESHOLD;
    for (i = 0; i < m; i += THRESHOLD) {
        insertionsort(base + i * gSiz, THRESHOLD);
    }
    if (n - m > 1) {
        insertionsort(base + i * gSiz, n - m);
    }
    for (i = THRESHOLD; i < n; i <<= 1) {
        m = n - n % (i << 1);
        for (j = 0; j < m; j += i << 1) {
            merge(base + j * gSiz, i, i << 1);
        }
        if (n - m > i) {
            merge(base + j * gSiz, i, n - m);
        }
    }
}

/**
 * merge left and right.
 * 
 * @param base  start of target array.
 * @param m     left subarray size in elements.
 * @param n     array size in elements.
 */
static void merge(char *base, size_t m, size_t n) {
    char *k, *pm = base + m * gSiz;
    size_t lm, rm;
    if (gCmp(pm, pm - gSiz) < 0) {
        if (n > THRESHOLD) {
            k = base + (m >> 1) * gSiz;
            if (gCmp(k, pm) < 0 || !gCmp(k, pm - gSiz)) {
                k = pm;
            }
            lm = searchkey(base, k, m);
            rm = searchkey(pm, k, n - m) + m;
            m = lm + rm - m;
            rotate(base + lm * gSiz, rm - m, rm - lm);
            if (lm > 0) {
                merge(base, lm, m);
            }
            if (rm < n) {
                merge(base + m * gSiz, rm - m, n - m);
            }
        } else {
            insertionsort(base, n);
        }
    }
}

/**
 * binary search.
 * 
 * @param base  start of target array.
 * @param key   search key.
 * @param n     array size in elements.
 * @return      minimum index satisfies base[return value] > key.
 */
static size_t searchkey(const char *base, const char *key, size_t n) {
    size_t l = 0, m;
    while (l < n) {
        m = (l + n) >> 1;
        if (gCmp(key, base + m * gSiz) < 0) {
            n = m;
        } else {
            l = m + 1;
        }
    }
    return l;
}

/**
 * rotate array to left.
 * 
 * @param base  start of target array.
 * @param move  amount of movement.
 * @param n     array size in elements.
 */
static void rotate(char *base, size_t move, size_t n) {
    size_t g, i, j, k;
    n *= gSiz; move *= gSiz;
    g = gcd(n, move);
    for (i = 0; i < g; i += gSiz) {
        memcpy(gTmp, base + i, gSiz);
        j = i;
        for (;;) {
            k = j;
            j += move;
            if (j >= n) {
                j -= n;
                if (j == i) {
                    break;
                }
            }
            memcpy(base + k, base + j, gSiz);
        }
        memcpy(base + k, gTmp, gSiz);
    }
}

/**
 * greatest common divisor.
 * 
 * @param x  integer x.
 * @param y  integer y.
 * @return   greatest common divisor of x and y.
 */
static unsigned int gcd(unsigned int x, unsigned int y) {
    unsigned int z;
    while (y) {
        z = x % y;
        x = y;
        y = z;
    }
    return x;
}

/**
 * insertion sort.
 * 
 * @param base  start of target array.
 * @param n     array size in elements.
 */
static void insertionsort(char *base, size_t n) {
    char *i, *j, *k = base + n * gSiz;
    for (i = base + gSiz; i < k; i += gSiz) {
        memcpy(gTmp, i, gSiz);
        if (gCmp(i - gSiz, gTmp) > 0) {
            j = i;
            do {
                memcpy(j, j - gSiz, gSiz);
                j -= gSiz;
            } while (j > base && gCmp(j - gSiz, gTmp) > 0);
            memcpy(j, gTmp, gSiz);
        }
    }
}
