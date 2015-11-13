
/*
 *	k-means.cpp
 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<float.h>
#include "k-means.h"

/****************************************************************************
 * 関数名 : calc_distance
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 2点間の距離を返す関数
 * 引数   : 点1:d1, 点2:d2, 次元数:d
 * 戻り値 : 2点間の距離:sum
 ****************************************************************************/
double calc_distance(double *d1, double *d2, int d) {
	int i;
	double sum = 0;
	//puts("----sum----");
	for (i = 0; i < d; i++) {
		//printf(" %f %f",d1[i],d2[i]);
		sum += (d1[i] - d2[i]) * (d1[i] - d2[i]);
		//printf(" %f ",sum);
	}
	//printf("\n");
	return sum;
}

/****************************************************************************
 * 関数名 : k_means2
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : k-means法でクラスタリングする関数
 * 引数   : データ集合:data, クラスタ集合:cl, クラスタ数:k, 次元数:d, 要素数:n
 * 戻り値 : なし
 ****************************************************************************/
void k_means(point *data, classify *cl, int k, int d, int n, mode m) {
	int i, j, l, d1, d2, save_tn;
	double t;
	double p_dis; /* 仮の距離 */
	int cn; /* 割り当てが変わった数 */
	char temp[64];
	int count;

	/***** 1.クラスタの初期重心を割り当てる *****/

	for (i = 0; i < 2 * n; i++) {
		d1 = rand() % n;
		do {
			d2 = rand() % n;
		} while (d1 == d2);
		for (j = 0; j < d; j++) {
			t = data[d1].cvector[j];
			data[d1].cvector[j] = data[d2].cvector[j];
			data[d2].cvector[j] = t;
		}
		memset(temp, 0, sizeof(temp));
		strcpy(temp, data[d1].id);
		strcpy(data[d1].id, data[d2].id);
		strcpy(data[d2].id, temp);
		save_tn = data[d1].text_num;
		data[d1].text_num = data[d2].text_num;
		data[d2].text_num = save_tn;
		save_tn = data[d1].nod;
		data[d1].nod = data[d2].nod;
		data[d2].nod = save_tn;
		save_tn = data[d1].hrg;
		data[d1].hrg = data[d2].hrg;
		data[d2].hrg = save_tn;
	}

	for (i = 0, l = 0; l < k; i++) {
		if (data[i].hrg) {
			for (j = 0; j < d; j++) {
				cl[l].coodinate[j] = data[i].cvector[j];
			}
			l++;
		}
	}
	/* 初期重心の割り当て(ランダム) */

	/*	
	printf("各クラスタの重心：\n");
	for (i = 0; i < k; i++) {
		for (j = 0; j < d; j++) {
			printf(" %f ", cl[i].coodinate[j]);
		}
		printf("\n");
	}
	*/

	double e;
	bool fin;
	count = 0;

	do {
		fin = 0;
		cn = 0;

		for (i = 0; i < n; i++) data[i].distance = DBL_MAX;
		/***** 2.各点を近接するクラスタに割り当てる *****/
		/* すべてのデータに対して */
		for (i = 0; i < n; i++) {
			if (data[i].hrg) {
				/* すべてのクラスタと比較 */
				for (j = 0; j < k; j++) {
					//printf("%f %f\n",data[i].cvector[0],cl[j].coodinate[0]);
					p_dis = calc_distance(data[i].cvector, cl[j].coodinate, d); /* クラスタ重心との距離計算 */
					//printf("%f \n",p_dis);
					/* 現在の重心間距離より短ければ所属クラスタを更新 */
					if (p_dis < data[i].distance) {
						data[i].distance = p_dis;
						data[i].clnum = j;
						cn++;
					}
				}
			}
			//printf("%d \n",data[i].clnum);
		}

		/***** 3.クラスタ重心を計算 *****/
		for (i = 0; i < k; i++) {
			/* 重心の値を初期化 */
			for (j = 0; j < d; j++) {
				cl[i].coodinate[j] = 0;
			}
			cl[i].cnum = 0; /* 所属データ数初期化 */
			/* すべてのデータを調べる */
			for (j = 0; j < n; j++) {
				if (data[i].hrg) {
					/* 所属クラスタだったら値を足し合わせる */
					if (data[j].clnum == i) {
						cl[i].cnum++;
						for (l = 0; l < d; l++) {
							cl[i].coodinate[l] += data[j].cvector[l];
						}
					}
				}
			}
			//printf("%d %d \n",i,cl[i].cnum);
			for (j = 0; j < d; j++) {
				/* 所属クラスタ数で割って重心の値を決定する */
				cl[i].coodinate[j] = cl[i].coodinate[j] / cl[i].cnum;
		//		printf("%f %d\n", cl[i].coodinate[j], cl[i].cnum);
			}
			if (cl[i].cnum == 0) {
				cn++;
				//puts("aaaa");
			}
		}
		/***** 4.クラスタの割り当てに変化が無ければ終了 *****/
		count++;
		//puts("aaa");
	} while (cn < 20);
	//while(!fin);

	/*
	printf("各クラスタの重心：\n");
	for (i = 0; i < k; i++) {
		for (j = 0; j < d; j++) {
			printf(" %f ", cl[i].coodinate[j]);
		}
		printf("\n");
	}
	*/

	for (i = 0; i < k; i++) {
		if (cl[i].cnum == 0) {
			printf("%d k-means miss\n", cl[i].cnum);
		}
	}
#ifdef kmean
#endif
}

