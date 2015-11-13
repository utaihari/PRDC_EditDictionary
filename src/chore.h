#ifndef INCLUDE_CHORE
#define INCLUDE_CHORE


#include"lzw.h"
#include"ds.h"
#include"cv-mst.h"
#include"k-means.h"

patch_graph* finalize_pgr(patch_graph *pgr,int n);
lzw_node** finalize_dics(lzw_node** dics,int dnum);
void Out_Features(patch_graph *data, int n,int d,dict *, char* filename);

#endif
