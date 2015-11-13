#ifndef INCLUDE_KMEANS
#define INCLUDE_KMEANS

#include "image2text.h"

/* 各要素を表す構造体 */
struct point{
	double* cvector;	/* 圧縮率ベクトル */
	int clnum;			/* 所属クラスタ番号 */
	int text_num;		/*データナンバ*/
	double distance;	/* 重心との距離 */
	int nod;			/*データ個数*/
	int hrg;			/*クラスタリング許可フラグ*/
	char id[64];
};

/* クラスタを表す構造体 */
struct classify{
	double* coodinate;	/* 重心座標 */
	int mindisdata;		/*重心最近データ*/
	double mindis;		/*重心最近データ距離*/
	int cnum;			/* 所属データ数 */
	int *data_num;		/*所属データ番号*/
	char id[128];
};

double calc_distance(double *d1,double *d2,int d);
void k_means(point *data,classify *cl,int k,int d,int n,mode m);

#endif
