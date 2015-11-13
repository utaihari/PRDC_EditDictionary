﻿#define DATASET "/home/uchinosub/workspace_copy/UCMerced_LandUse/dataset"
#define DATASET_LBP "/home/uchinosub/workspace_copy/UCMerced_LandUse/dataset_lbp"
#define TEXT "/home/uchinosub/workspace_copy/UCMerced_LandUse/text"
#define TEXT_LBP "/home/uchinosub/workspace_copy/UCMerced_LandUse/text_lbp"
#define DICT "/home/uchinosub/workspace_copy/UCMerced_LandUse/dictionary"
#define DICT_LBP "/home/uchinosub/workspace_copy/UCMerced_LandUse/dictionary_lbp"
#define BASIS "/home/uchinosub/workspace_copy/UCMerced_LandUse/basis"
#define BASIS_LBP "/home/uchinosub/workspace_copy/UCMerced_LandUse/basis_lbp"
#define OUTPUT "/home/uchinosub/workspace_copy/UCMerced_LandUse/result"
#define EDT_BASIS "/home/uchinosub/workspace_copy/UCMerced_LandUse/edit-basis"
#define EDT_BASIS_LBP "/home/uchinosub/workspace_copy/UCMerced_LandUse/edit-basis_lbp"

#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<float.h>
#include<dirent.h>
#include"lzw.h"
#include"image2text.h"
#include"bitIO.h"
#include"chore.h"
#include"cv-mst.h"
#include"k-means.h"
#include"ds.h"

#include<iostream>
#include<stdint.h>

void Extract(mode, int, dict *, bool);
void CreateLbpImage(mode m, int d, dict *candi_lbp);
void SelectProposal(int, dict *, dict *, mode, int, bool);
void SelectBaseline(int, dict *, mode, dict *, bool);
void str_eraser(char *, char *);
int listComp_dsc(const void*, const void*);
int listComp_asc(const void* a, const void* b);
int listComp3(const void*, const void*);
void Labeling(int, dict *);
void CompressExmpl(mode, dict *, dict *, dict *, int, patch_graph *);
float KnnSearch(int, mode, patch_graph *, dict *, knn_result *);

/****************************************************************************
 * 関数名 : main
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 : 2015/02/06
 * 機能   : main
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
int main(int argc, char* argv[]) {

	int d; /* データセットの要素数 */

	patch_graph *exmpl_pgr = NULL;
	FILE *fp;
	char buf[10];
	dict *candi, *sel_dict, *candi_lbp, *lbp_sel_dict;
	knn_result *result;
	char fname[128];
	float ave_rate;
	float retrn;

	/*実験パラメータ*/
	mode m;
	m.num_basis = 10;	//基底辞書の個数
	m.select = 1;		//辞書の選び方： 1==類似度、2==kmeans
	m.criteria = 2;		//非類似度 : 1==合計最大化、2==最小最大化
	m.text_cnvrt = 0; 	//データセット画像のテキスト変換処理の省略(0で省略する)
	m.del_dic = 1; 		//辞書の単語削除の有無
	m.add = 0; 			//削除後の単語補完の有無
	m.first_select = 0; //最初の辞書の選択方法 0:ランダム、1:(自己圧縮率が)最大、2:中間、3:最小
	m.lbp = 0;			//LBP特徴を用いるかの有無
	m.k = 6;

	/* 乱数のシード */
	int t;
	//t = 1;
	t = atoi(argv[1]);	//複数試行をおこなう実験の際に用いる
	srand(t);
	printf("seed=%d \n", t);

	puts("----------------Step 1. Extract basis-dictionary----------------");
	//基底辞書の抽出？
	/* (準備)　データセットのデータ数d取得 */
	fp = popen("ls -1F "DATASET" | grep -v / | wc -l ", "r");
	fgets(buf, 9, fp);
	d = atoi(buf);
	pclose(fp);
	candi = (dict *) calloc(d, sizeof(dict));
	candi_lbp = (dict *) calloc(d, sizeof(dict));

	if (m.text_cnvrt == 1) {
		system("rm "TEXT"/*");
		system("rm "DICT"/*");
		system("rm "TEXT_LBP"/*");
		if (m.lbp == 1) {
			CreateLbpImage(m, d, candi_lbp);
		}
	}
	Extract(m, d, candi, 0);
	if (m.lbp == 1) {
		Extract(m, d, candi_lbp, 1);
	}

	puts("----------------Step 1. End----------------");

	sel_dict = (struct dict *) calloc(m.num_basis, sizeof(struct dict));
	lbp_sel_dict = (struct dict *) calloc(m.num_basis, sizeof(struct dict));
	if (m.select == 1) {
		puts(
				"----------------Step 2. Select basis-dictionary(based on dissimilarity)----------------");
		//相違性に基づく基底辞書の選択？
		system("rm "BASIS"/*");
		system("rm "EDT_BASIS"/*");
		SelectProposal(d, candi, sel_dict, m, t, 0);
		if (m.lbp == 1) {
			system("rm "BASIS_LBP"/*");
			system("rm "EDT_BASIS_LBP"/*");
			SelectProposal(d, candi_lbp, lbp_sel_dict, m, t, 1);
		}
	} else if (m.select == 2) {
		puts(
				"----------------Step 2. Select basis-dictionary(based on k-means centroid)----------------");
		system("rm "BASIS"/*");
		system("rm "EDT_BASIS"/*");
		SelectBaseline(d, candi, m, sel_dict, 0);
		if (m.lbp == 1) {
			system("rm "BASIS_LBP"/*");
			system("rm "EDT_BASIS_LBP"/*");
			SelectBaseline(d, candi_lbp, m, lbp_sel_dict, 1);
		}
	}
	puts("----------------Step 2. End----------------");

	puts(
			"----------------Step3. Convert Data to Compression Vector.----------------");

	exmpl_pgr = (patch_graph *) malloc(sizeof(patch_graph) * d);
	if (exmpl_pgr == NULL) {
		puts("memory error!");
		exit(1);
	}
	CompressExmpl(m, candi, sel_dict, lbp_sel_dict, d, exmpl_pgr);
	Labeling(d, candi);

	puts("----------------Step 3. End----------------");

	puts(
			"----------------Step 4. Classify query using k-NN search and leave-one-out validation.----------------");
	result = (knn_result *) malloc(sizeof(knn_result) * d);
	retrn = KnnSearch(d, m, exmpl_pgr, candi, result);
	puts("----------------Step 4. End----------------");

	puts("----------------Result----------------");
	ave_rate = retrn;
	sprintf(fname, OUTPUT"/rate/num%d.dat", m.num_basis);
	if ((fp = fopen(fname, "a+")) == NULL) {
		printf("file name miss\n");
		exit(1);
	}
	fprintf(fp, "%d %f\n", t, ave_rate);
	fclose(fp);

# ifdef AB
#endif

	free(candi);
	free(candi_lbp);
	free(sel_dict);
	free(lbp_sel_dict);
	free(exmpl_pgr);
	free(result);

	puts("----------------Finished----------------");

	exit(0);
}

/****************************************************************************
 * 関数名 : Extract
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 : 2015/02/22
 * 機能   :
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
void Extract(mode m, int d, dict *candi, bool lbp_flag) {
	int i, j;
	FILE *fp, *fp2;
	IplImage* simage;
	char imgname[128];
	char textname[128];
	char dicname[128];
	char filename[128];
	fpos_t tsize, dsize;
	char split[] = ".tif";

	lzw_node** dics;
	dics = initialize_dics(d, MAX_PHRASE_NUM);

	DIR *dir;
	struct dirent *dp;
	dir = opendir(DATASET);

	i = 0;

	for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		} else {
			//辞書作成
			memset(imgname, 0, sizeof(imgname));
			sprintf(candi[i].id, "%s", dp->d_name);
			str_eraser(candi[i].id, split);
			if (lbp_flag == 0) {
				sprintf(imgname, DATASET"/%s.tif", candi[i].id);
				sprintf(textname, TEXT"/%s.txt", candi[i].id);
			} else {
				sprintf(imgname, DATASET_LBP"/%s.tif", candi[i].id);
				sprintf(textname, TEXT_LBP"/%s.txt", candi[i].id);
			}

			if (m.text_cnvrt == 1) {
				//画像を読み込む
				if (lbp_flag == 0) {
					simage = cvLoadImage(imgname,
							CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
				} else {
					simage = cvLoadImage(imgname,
							CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_GRAYSCALE);
				}

				if (!simage) {
					/* エラー処理 */
					printf("error: no image file[%s]\n", imgname);
					exit(1);
				}
				//色空間の変換
				if (lbp_flag == 0) {
					cvCvtColor(simage, simage, CV_BGR2Lab);
					if (!simage) {
						//エラー処理
						printf("error: no image file[%s]\n", imgname);
						exit(1);
					}
				}
				if (lbp_flag == 0) {
					image2text_horizon(simage, textname, m);
				} else {
					image2text_lbp(simage, textname, m);
				}

				if ((fp = fopen(textname, "rb")) == NULL) {
					printf("file name miss\n");
					exit(1);
				}
				fseek(fp, 0, SEEK_END);
				fgetpos(fp, &tsize);
				//printf("(double)tsize.__pos=%f\n",(double)tsize.__pos);
				fclose(fp);
				sprintf(dicname, OUTPUT"/%s.dic", candi[i].id); //書き出さない
				initialize_file(textname, dicname);
				initilalize_work();
				encode(dics[i]);
				finalize_file();
				if (remove(dicname) != 0) {
					printf("error");
				}
				//辞書の書き出し
				memset(dicname, 0, sizeof(dicname));
				if (lbp_flag == 0) {
					sprintf(dicname, DICT"/%s.dic", candi[i].id);
				} else {
					sprintf(dicname, DICT_LBP"/%s.dic", candi[i].id);
				}

				if ((fp = fopen(dicname, "wb")) == NULL) {
					printf("file name miss\n");
					exit(1);
				}

				for (j = 0; j < MAX_PHRASE_NUM; j++) {
					put_bits(fp, dics[i][j].alphabet, DICW_SIZE);
					put_bits(fp, dics[i][j].brother, DICW_SIZE);
					put_bits(fp, dics[i][j].child, DICW_SIZE);
					put_bits(fp, dics[i][j].parent, DICW_SIZE);
					put_bits(fp, dics[i][j].flag, DICW_SIZE);
					put_bits(fp, dics[i][j].depth, DICW_SIZE);
				}
				// 		strcpy(candi[i].id,dp->d_name);
				fclose(fp);
			}
			i++;
		}
	}

	if (m.text_cnvrt == 1) {
		sprintf(filename, OUTPUT"/srate.dat");
		fp2 = fopen(filename, "w");

		//自己圧縮率の計算
		for (i = 0; i < d; i++) {
			memset(dicname, 0, sizeof(dicname));
			sprintf(dicname, OUTPUT"/%d.dic", i); //書き出さない
			memset(textname, 0, sizeof(textname));
			sprintf(textname, TEXT"/%s.txt", candi[i].id);
			initialize_file(textname, dicname);
			initilalize_work();
			encode_fix2(dics[i]); //フラグ有の圧縮
			finalize_file();
			//圧縮後のサイズ取得
			fp = fopen(dicname, "rb");
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &dsize);
			fclose(fp);
			if (remove(dicname) != 0) {
				printf("error\n");
			}
			candi[i].self_rate = (double) dsize.__pos / (double) tsize.__pos;
			fprintf(fp2, "%s %f \n", candi[i].id, candi[i].self_rate);
			//printf("candi[%d].self_rate=%f/%f \n",i,(double)dsize.__pos,(double)tsize.__pos);
		}

		fclose(fp2);
	}

	closedir(dir);
	dics = finalize_dics(dics, d);
	free(dics);

}

/****************************************************************************
 * 関数名 : CreateLbpImage
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 :
 * 機能   :
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
void CreateLbpImage(mode m, int d, dict *candi) {
	int i, j;
	IplImage* simage;
	char imgname[128];
	char textname[128];
	char filename[128];
	char split[] = ".tif";
	int x, y;
	node** graph = NULL;
	int center;
	int neighbor[8];

	lzw_node** dics;
	dics = initialize_dics(d, MAX_PHRASE_NUM);

	DIR *dir;
	struct dirent *dp;
	dir = opendir(DATASET);

	i = 0;

	for (dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
			continue;
		} else {
			//辞書作成
			memset(imgname, 0, sizeof(imgname));
			sprintf(candi[i].id, "%s", dp->d_name);
			str_eraser(candi[i].id, split);
			sprintf(imgname, DATASET"/%s.tif", candi[i].id);
			sprintf(textname, TEXT"/%s.txt", candi[i].id);
			//画像を読み込む
			simage = cvLoadImage(imgname,
					CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_GRAYSCALE);
			if (!simage) {
				/* エラー処理 */
				printf("error: no image file[%s]\n", imgname);
				exit(1);
			}

			graph = (node **) malloc(sizeof(node) * simage->width);
			if (!graph) {
				printf("memory securing misssss (graph)\n");
				//return NULL;
			}
			for (j = 0; j < simage->width; j++) {
				graph[j] = (node *) malloc(sizeof(node) * simage->height);
				if (!graph[j]) {
					printf("memory securing miss (graph[%d])\n", i);
					//return NULL;
				}
			}

			for (y = 1; y < simage->height - 1; y++) {
				graph[x][y].clr.val[0] = 0;
				for (x = 1; x < simage->width - 1; x++) {
					/* 画素値を直接操作する一例 */
					center = simage->imageData[simage->widthStep * y + x]; // B
					neighbor[0] =
							(unsigned char) simage->imageData[simage->widthStep
									* y + (x - 1)];
					neighbor[1] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y + 1) + (x - 1)];
					neighbor[2] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y + 1) + (x)];
					neighbor[3] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y + 1) + (x + 1)];
					neighbor[4] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y) + (x + 1)];
					neighbor[5] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y - 1) + (x + 1)];
					neighbor[6] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y - 1) + (x)];
					neighbor[7] =
							(unsigned char) simage->imageData[simage->widthStep
									* (y - 1) + (x - 1)];

					for (j = 0; j < 8; j++) {
						if (center - neighbor[j] >= 0) {
							graph[x][y].clr.val[0] += pow(2, 7 - j);
						}
					}
				}
			}

			for (y = 1; y < simage->height - 1; y++) {
				for (x = 1; x < simage->width - 1; x++) {
					/* 画素値を直接操作する一例 */
					simage->imageData[simage->widthStep * y + x] =
							graph[x][y].clr.val[0]; // B
					//printf("%d \n",(unsigned char)simage->imageData[simage->widthStep * y + x ]);
				}
			}
			memset(filename, 0, sizeof(filename));
			sprintf(filename, DATASET_LBP"/%s.tif", candi[i].id);
			cvSaveImage(filename, simage);
			i++;
		}
	}

	closedir(dir);
	dics = finalize_dics(dics, d);
	free(dics);

}

/****************************************************************************
 * 関数名 : SelectProposal
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : 候補の中から基底辞書を選ぶ
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
void SelectProposal(int d, dict * candi, dict * sel_dict, mode m, int t,
		bool lbp_flag) {

	int rnd;
	char textname[128];
	char dicname[128];
	lzw_node** dics;
	FILE *fp;
	int i, j, k;
	fpos_t tsize, dsize;
	struct dict *candidate;					//選択辞書候補?
	struct dict *first;
	struct dict **dic_list;
	struct dict *temp_list;
	float min;
	int count;

	candidate = (struct dict *) malloc(sizeof(struct dict) * d);
	first = (struct dict *) malloc(sizeof(struct dict));
	dic_list = (struct dict **) malloc(sizeof(struct dict) * d);
	temp_list = (struct dict *) malloc(sizeof(struct dict) * m.num_basis);

	for (j = 0; j < d; j++) {
		dic_list[j] = (struct dict *) malloc(sizeof(struct dict) * m.num_basis);
		for (i = 0; i < m.num_basis; i++) {
			dic_list[j][i].rate = 10000; //初期値
		}
	}

	for (j = 0; j < d; j++) {
		candidate[j].flag = 0;
		candidate[j].sum_rate = 0;
	}
	dics = initialize_dics(m.num_basis, MAX_PHRASE_NUM);

	i = 0;

	//1回目は辞書をランダムに選択
	if (m.first_select == 0) {
		rnd = rand() % d;
		if (lbp_flag == 0) {
			sprintf(dicname, DICT"/%s.dic", candi[rnd].id);
		} else {
			sprintf(dicname, DICT_LBP"/%s.dic", candi[rnd].id);
		}

		if ((fp = fopen(dicname, "rb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		printf("%d %s selected(first). rnd = %d\n", i, candi[rnd].id, rnd);
		strcpy(first->id, candi[rnd].id);
		//決定的に選ぶ
	} else {
		qsort(candi, d, sizeof(struct dict), listComp3);
		for (j = 0; j < d; j++) {
		}
		if (m.first_select == 1) {
			if (lbp_flag == 0) {
				sprintf(dicname, DICT"/%s.dic", candi[t - 1].id);
			} else {
				sprintf(dicname, DICT_LBP"/%s.dic", candi[t - 1].id);
			}
			strcpy(first->id, candi[t - 1].id);
			printf("%d %s selected(first).\n", i, candi[t - 1].id);
		} else if (m.first_select == 2) {
			if (lbp_flag == 0) {
				sprintf(dicname, DICT"/%s.dic", candi[d / 2].id);
			} else {
				sprintf(dicname, DICT_LBP"/%s.dic", candi[d / 2].id);
			}
			strcpy(first->id, candi[d / 2].id);
			printf("%d %s selected(first).\n", i, candi[d / 2].id);
		} else if (m.first_select == 3) {
			if (lbp_flag == 0) {
				sprintf(dicname, DICT"/%s.dic", candi[d - 1].id);
			} else {
				sprintf(dicname, DICT_LBP"/%s.dic", candi[d - 1].id);
			}
			strcpy(first->id, candi[d - 1].id);
			printf("%d %s selected(first).\n", i, candi[d - 1].id);
		}
		if ((fp = fopen(dicname, "rb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
	}

	//辞書ファイルからデータの読み込み?
	for (j = 0; j < MAX_PHRASE_NUM; j++) {
		dics[i][j].alphabet = get_bits(fp, DICW_SIZE);
		dics[i][j].brother = get_bits(fp, DICW_SIZE);
		dics[i][j].child = get_bits(fp, DICW_SIZE);
		dics[i][j].parent = get_bits(fp, DICW_SIZE);
		dics[i][j].flag = get_bits(fp, DICW_SIZE);
		dics[i][j].depth = get_bits(fp, DICW_SIZE);
	}
	fclose(fp);

	memset(dicname, 0, sizeof(dicname)); //dicname の初期化（すべての値を0で埋める）?
	if (m.first_select == 0) {
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candi[rnd].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candi[rnd].id);
		}
	} else if (m.first_select == 1) {
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candi[t - 1].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candi[t - 1].id);
		}
	} else if (m.first_select == 2) {
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candi[d / 2].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candi[d / 2].id);
		}
	} else if (m.first_select == 3) {
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candi[d - 1].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candi[d - 1].id);
		}
	}

	for (j = 0; j < d; j++) {
		memset(textname, 0, sizeof(textname));
		if (lbp_flag == 0) {
			sprintf(textname, TEXT"/%s.txt", candi[j].id);
		} else {
			sprintf(textname, TEXT_LBP"/%s.txt", candi[j].id);
		}

		fp = fopen(textname, "rb");
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &tsize);
		fclose(fp);
		memset(dicname, 0, sizeof(dicname));
		sprintf(dicname, OUTPUT"/%d.dic", j); //書き出さない
		initialize_file(textname, dicname);
		initilalize_work();
		encode_fix2(dics[i]);
		finalize_file();
		fp = fopen(dicname, "rb");
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &dsize);
		fclose(fp);
		if (remove(dicname) != 0) {
			printf("error");
		}
		strcpy(candidate[j].id, candi[j].id);
		if (m.criteria == 1) {
			candidate[j].sum_rate += (double) dsize.__pos
					/ (double) tsize.__pos;
			candidate[j].rate = candidate[j].sum_rate;
		} else if (m.criteria == 2) {
			dic_list[j][i].rate = (double) dsize.__pos / (double) tsize.__pos;
			for (k = 0; k < m.num_basis; k++) {
				temp_list[k].rate = dic_list[j][k].rate;
				if (k == 0) {
					min = temp_list[k].rate;
				} else {
					if (min > temp_list[k].rate)
						min = temp_list[k].rate;
				}
			}
			candidate[j].rate = min;
		}
	}
	qsort(candidate, d, sizeof(struct dict), listComp_dsc);

	while (i < m.num_basis) {
		for (j = 0; j < d; j++) {
			if (candidate[j].flag == 0) {
				memset(dicname, 0, sizeof(dicname));
				if (lbp_flag == 0) {
					sprintf(dicname, DICT"/%s.dic", candidate[j].id);
				} else {
					sprintf(dicname, DICT_LBP"/%s.dic", candidate[j].id);
				}
				if ((fp = fopen(dicname, "rb")) == NULL) {
					printf("file name miss\n");
					exit(1);
				}
				candidate[j].flag = 1;
				k = j;
				break;
			}
		}
		strcpy(sel_dict[i].id, candidate[k].id);
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			dics[i][j].alphabet = get_bits(fp, DICW_SIZE);
			dics[i][j].brother = get_bits(fp, DICW_SIZE);
			dics[i][j].child = get_bits(fp, DICW_SIZE);
			dics[i][j].parent = get_bits(fp, DICW_SIZE);
			dics[i][j].flag = get_bits(fp, DICW_SIZE);
			dics[i][j].depth = get_bits(fp, DICW_SIZE);
		}
		fclose(fp);

		memset(dicname, 0, sizeof(dicname));
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candidate[k].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candidate[k].id);
		}

		if ((fp = fopen(dicname, "wb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			put_bits(fp, dics[i][j].alphabet, DICW_SIZE);
			put_bits(fp, dics[i][j].brother, DICW_SIZE);
			put_bits(fp, dics[i][j].child, DICW_SIZE);
			put_bits(fp, dics[i][j].parent, DICW_SIZE);
			put_bits(fp, dics[i][j].flag, DICW_SIZE);
			put_bits(fp, dics[i][j].depth, DICW_SIZE);
		}
		fclose(fp);

		for (j = 0; j < d; j++) {
			memset(textname, 0, sizeof(textname));
			if (lbp_flag == 0) {
				sprintf(textname, TEXT"/%s.txt", candi[j].id);
			} else {
				sprintf(textname, TEXT_LBP"/%s.txt", candi[j].id);
			}
			fp = fopen(textname, "rb");
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &tsize);
			fclose(fp);
			memset(dicname, 0, sizeof(dicname));
			sprintf(dicname, OUTPUT"/%d.dic", j); //書き出さない
			initialize_file(textname, dicname);
			initilalize_work();
			encode_fix2(dics[i]);
			finalize_file();
			fp = fopen(dicname, "rb");
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &dsize);
			fclose(fp);
			if (remove(dicname) != 0) {
				printf("error");
			}
			if (m.criteria == 1) {
				candidate[j].sum_rate += (double) dsize.__pos;
				candidate[j].rate = candidate[j].sum_rate;
			} else if (m.criteria == 2) {
				dic_list[j][i].rate = (double) dsize.__pos
						/ (double) tsize.__pos;
				for (k = 0; k < m.num_basis; k++) {
					temp_list[k].rate = dic_list[j][k].rate;
					if (k == 0) {
						min = temp_list[k].rate;
					} else {
						if (min > temp_list[k].rate)
							min = temp_list[k].rate;
					}
				}
				candidate[j].rate = min;
			}
		}
		qsort(candidate, d, sizeof(struct dict), listComp_dsc);
		i++;
	}

#ifdef ANALY
	//基底辞書の相互の共通単語数の表示(解析用)
	for (i = 1; i < m.num_basis; i++) {
		for (k = 0; k < i; k++) {
			count = count_phrase(dics[i], dics[k]);
			printf("match dic[%d],dic[%d] : %d\n",i,k,count+256);
		}
	}

	//基底辞書それぞれの単語数の表示(解析用)
	for (i = 0; i < m.num_basis; i++) {
		count = count_phrase(dics[i], dics[i]);
		printf("match dic[%d],dic[%d] : %d\n",i,i,count+256);
		printf("-------dic%d phrase-------\n",i);
	}
#endif

	//共通単語の削除・補完
	if (m.del_dic == 1) {
		for (i = 1; i < m.num_basis; i++) {
			dics[i]->pre_phrase_num = count_phrase(dics[i], dics[i]);
			for (k = 0; k < i; k++) {
				//	printf("dic[%d], dic[%d]",k,i);
				delete_cophrase(dics[k], dics[i]);
			}
//			printf("dic[%d]: ",i);
			if (m.add == 1) {
				compensate_phrase(dics[i]);
			}
			revise_flag(dics[i]);
		}
	}

#ifdef ANALY2
	//削除・補完後の基底辞書の相互の共通単語数の表示(解析用)
	for (i = 1; i < m.num_basis; i++) {
		for (k = 0; k < i; k++) {
			count = count_phrase(dics[i], dics[k]);
//				printf("match dic[%d],dic[%d] : %d\n",i,k,count);
		}
	}
	//削除・補完後の基底辞書それぞれの単語数の表示(解析用)
	for (i = 0; i < m.num_basis; i++) {
		count = count_phrase(dics[i], dics[i]);
//		printf("match dic[%d],dic[%d] : %d\n",i,i,count+256);
	}
#endif

	if (m.del_dic == 1) {
		for (i = 0; i < m.num_basis; i++) {
			memset(dicname, 0, sizeof(dicname));
			if (lbp_flag == 0) {
				sprintf(dicname, EDT_BASIS"/%s.dic", sel_dict[i].id);
			} else {
				sprintf(dicname, EDT_BASIS_LBP"/%s.dic", sel_dict[i].id);
			}
			if ((fp = fopen(dicname, "wb")) == NULL) {
				printf("file name miss\n");
				exit(1);
			}
			for (j = 0; j < MAX_PHRASE_NUM; j++) {
				put_bits(fp, dics[i][j].alphabet, DICW_SIZE);
				put_bits(fp, dics[i][j].brother, DICW_SIZE);
				put_bits(fp, dics[i][j].child, DICW_SIZE);
				put_bits(fp, dics[i][j].parent, DICW_SIZE);
				put_bits(fp, dics[i][j].flag, DICW_SIZE);
				put_bits(fp, dics[i][j].depth, DICW_SIZE);
			}
			fclose(fp);
		}
	}

//#endif

	for (j = 0; j < d; j++) {
		free(dic_list[j]);
	}

	free(dic_list);

	free(candidate);
	free(first);
	free(temp_list);
	dics = finalize_dics(dics, m.num_basis);
	free(dics);
}

/****************************************************************************
 * 関数名 : SelectBaseline
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 :
 * 機能   : 候補の中から基底辞書を選ぶ
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
void SelectBaseline(int d, dict * candi, mode m, dict *sel_dict,
		bool lbp_flag) {

	int rnd;
	char select_dic[128];
	char textname[128];
	char dicname[128];
	lzw_node** dics;
	fpos_t tsize, dsize;
	FILE *fp;
	int i, j;
	struct dict *select_dict;
	int *selected_table;
	bool flag;

	select_dict = (struct dict *) malloc(sizeof(struct dict) * d);
	for (j = 0; j < d; j++) {
		select_dict[j].flag = 0;
	}
	selected_table = (int *) malloc(sizeof(int) * m.num_basis);
	dics = initialize_dics(m.num_basis, MAX_PHRASE_NUM);

	//1.ランダムに辞書を選択、辞書を読み込む、ファイルに書き出す
	puts("----------Select with random.-----------");

	for (i = 0; i < m.num_basis; i++) {
		if (i == 0) {
			rnd = rand() % d;
			selected_table[0] = rnd;
		} else {
			do {
				flag = 0;
				rnd = rand() % d;
				for (j = 0; j < i; j++) {
					if (rnd == selected_table[j]) {
						flag = 1;
						break;
					}
				}
				if (flag == 0) {
					selected_table[i] = rnd;
				}
			} while (flag == 1);
		}
		if (lbp_flag == 0) {
			sprintf(select_dic, DICT"/%s.dic", candi[rnd].id);
		} else {
			sprintf(select_dic, DICT_LBP"/%s.dic", candi[rnd].id);
		}
		//strcpy(select_dic,candi[rnd].dic_id);
		//printf("%s\n",select_dic);
		if ((fp = fopen(select_dic, "rb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			dics[i][j].alphabet = get_bits(fp, DICW_SIZE);
			dics[i][j].brother = get_bits(fp, DICW_SIZE);
			dics[i][j].child = get_bits(fp, DICW_SIZE);
			dics[i][j].parent = get_bits(fp, DICW_SIZE);
			dics[i][j].flag = get_bits(fp, DICW_SIZE);
			dics[i][j].depth = get_bits(fp, DICW_SIZE);
		}
		fclose(fp);
		//基底辞書書き出し
		memset(dicname, 0, sizeof(dicname));
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", candi[rnd].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", candi[rnd].id);
		}
		if ((fp = fopen(dicname, "wb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			put_bits(fp, dics[i][j].alphabet, DICW_SIZE);
			put_bits(fp, dics[i][j].brother, DICW_SIZE);
			put_bits(fp, dics[i][j].child, DICW_SIZE);
			put_bits(fp, dics[i][j].parent, DICW_SIZE);
			put_bits(fp, dics[i][j].flag, DICW_SIZE);
			put_bits(fp, dics[i][j].depth, DICW_SIZE);
		}
		fclose(fp);
	}

	double rate;
	point *data;
	classify *cl;
	patch_graph *pgr;

	pgr = (patch_graph *) malloc(sizeof(patch_graph) * d);

	//2.候補となる画像集合を特徴量へ変換
	for (i = 0; i < d; i++) {
		pgr[i].cv_num = (double *) malloc(sizeof(double) * m.num_basis);
		memset(textname, 0, sizeof(textname));
		if (lbp_flag == 0) {
			sprintf(textname, TEXT"/%s.txt", candi[i].id);
		} else {
			sprintf(textname, TEXT_LBP"/%s.txt", candi[i].id);
		}
		strcpy(pgr[i].id, candi[i].id);
		fp = fopen(textname, "rb");
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &tsize);
		fclose(fp);
		for (j = 0; j < m.num_basis; j++) {
			memset(dicname, 0, sizeof(dicname));
			sprintf(dicname, OUTPUT"/%d.dic", j); //書き出さない
			initialize_file(textname, dicname);
			initilalize_work();
			encode_fix2(dics[j]); /* フラグ有の圧縮 */
			finalize_file();
			/* 圧縮後のサイズ取得 */
			fp = fopen(dicname, "rb");
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &dsize);
			fclose(fp);
			if (remove(dicname) != 0) {
				printf("error\n");
			}
			rate = (double) dsize.__pos / (double) tsize.__pos;
			pgr[i].cv_num[j] = rate;
		}
	}

	//3.特徴量をk-meansクラスタリング
	data = (point *) malloc(sizeof(point) * d);

	for (i = 0; i < d; i++) {
		data[i].cvector = (double *) malloc(sizeof(double) * m.num_basis);
		data[i].distance = DBL_MAX;/*最大値で初期化*/
		data[i].text_num = i;
		data[i].hrg = 1;
		data[i].nod = 0;
		strcpy(data[i].id, pgr[i].id);
		for (j = 0; j < m.num_basis; j++) {
			data[i].cvector[j] = pgr[i].cv_num[j];
		}
	}

	cl = (classify *) malloc(sizeof(classify) * m.num_basis);
	for (i = 0; i < m.num_basis; i++) {
		cl[i].coodinate = (double *) malloc(sizeof(double) * m.num_basis);
		cl[i].mindis = DBL_MAX;
	}

	k_means(data, cl, m.num_basis, m.num_basis, d, m);

	for (i = 0; i < d; i++) {
		if (data[i].distance < cl[data[i].clnum].mindis) {
			cl[data[i].clnum].mindis = data[i].distance;
			cl[data[i].clnum].mindisdata = data[i].text_num;
			strcpy(cl[data[i].clnum].id, data[i].id);
		}
	}

	for (i = 0; i < m.num_basis; i++) {
		if (cl[i].cnum == 0) {
			cl[i].mindisdata = 0;
		}
	}

	//4.クラスタ中心のデータを基底辞書として書き出す
	puts("----Output Centered Dictionary.---------");

	if (lbp_flag == 0) {
		system("rm "BASIS"/*");
	} else {
		system("rm "BASIS_LBP"/*");
	}

	free(dics);
	dics = initialize_dics(m.num_basis, MAX_PHRASE_NUM);

	for (i = 0; i < m.num_basis; i++) {

		memset(dicname, 0, sizeof(dicname));
		if (lbp_flag == 0) {
			sprintf(dicname, DICT"/%s.dic", cl[i].id);
		} else {
			sprintf(dicname, DICT_LBP"/%s.dic", cl[i].id);
		}
		strcpy(sel_dict[i].id, cl[i].id);
		if ((fp = fopen(dicname, "rb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}

		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			dics[i][j].alphabet = get_bits(fp, DICW_SIZE);
			dics[i][j].brother = get_bits(fp, DICW_SIZE);
			dics[i][j].child = get_bits(fp, DICW_SIZE);
			dics[i][j].parent = get_bits(fp, DICW_SIZE);
			dics[i][j].flag = get_bits(fp, DICW_SIZE);
			dics[i][j].depth = get_bits(fp, DICW_SIZE);
		}
		fclose(fp);

		memset(dicname, 0, sizeof(dicname));
		if (lbp_flag == 0) {
			sprintf(dicname, BASIS"/%s.dic", cl[i].id);
		} else {
			sprintf(dicname, BASIS_LBP"/%s.dic", cl[i].id);
		}
		if ((fp = fopen(dicname, "wb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			put_bits(fp, dics[i][j].alphabet, DICW_SIZE);
			put_bits(fp, dics[i][j].brother, DICW_SIZE);
			put_bits(fp, dics[i][j].child, DICW_SIZE);
			put_bits(fp, dics[i][j].parent, DICW_SIZE);
			put_bits(fp, dics[i][j].flag, DICW_SIZE);
			put_bits(fp, dics[i][j].depth, DICW_SIZE);
		}
		fclose(fp);

	}

	free(select_dict);
	free(selected_table);
	free(pgr);

#ifdef RRR
#endif

}

/****************************************************************************
 * 関数名 : str_eraser
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : 文字列s1に含まれる文字列s2を削除する
 * 引数   : 比較対象1:_a, 比較対象2:_b
 * 戻り値 : 1 or -1 or 0
 ****************************************************************************/
void str_eraser(char *s1, char *s2) {
	char *p = s1;

	p = strstr(s1, s2);
	if (p != NULL) {
		strcpy(p, p + strlen(s2));
		str_eraser(p + 1, s2);
	}
}

/****************************************************************************
 * 関数名 : listComp
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : qsort()でdouble型の要素を比較する関数（降順）
 * 引数   : 比較対象1:_a, 比較対象2:_b
 * 戻り値 : 1 or -1 or 0
 ****************************************************************************/
int listComp_dsc(const void* a, const void* b) {

	const struct dict* x = (const struct dict*) a;
	const struct dict* y = (const struct dict*) b;

	if (x->rate < y->rate)
		return 1;
	if (x->rate > y->rate)
		return -1;
	return 0;
}

/****************************************************************************
 * 関数名 : listComp_asc
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : qsort()でdouble型の要素を比較する関数（昇順）
 * 引数   : 比較対象1:_a, 比較対象2:_b
 * 戻り値 : 1 or -1 or 0
 ****************************************************************************/
int listComp_asc(const void* a, const void* b) {

	const struct dict* x = (const struct dict*) a;
	const struct dict* y = (const struct dict*) b;

	if (x->rate > y->rate)
		return 1;
	if (x->rate < y->rate)
		return -1;
	return 0;
}

/****************************************************************************
 * 関数名 : listComp2
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : knnで距離をソートするために用いる
 * 引数   : 比較対象1:_a, 比較対象2:_b
 * 戻り値 : 1 or -1 or 0
 ****************************************************************************/
int listComp2(const void* a, const void* b) {

	const struct knn_temp* x = (const struct knn_temp*) a;
	const struct knn_temp* y = (const struct knn_temp*) b;

	if (x->dis > y->dis)
		return 1;
	if (x->dis < y->dis)
		return -1;
	return 0;
}

/****************************************************************************
 * 関数名 : listComp3
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : qsort()でdouble型の要素を比較する関数（降順）
 * 引数   : 比較対象1:_a, 比較対象2:_b
 * 戻り値 : 1 or -1 or 0
 ****************************************************************************/
int listComp3(const void* a, const void* b) {

	const struct dict* x = (const struct dict*) a;
	const struct dict* y = (const struct dict*) b;

	if (x->self_rate < y->self_rate)
		return 1;
	if (x->self_rate > y->self_rate)
		return -1;
	return 0;
}

/****************************************************************************
 * 関数名 : Labeling
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : 画像を読み込んで、ファイル名に基づいてクラスラベルをつける。
 * 引数   :
 * 戻り値 :
 ****************************************************************************/
void Labeling(int e, dict* exmpl) {

	int i;

	for (i = 0; i < e; i++) {
		//Labeling...
		if (strstr(exmpl[i].id, "forest") != NULL) {
			strcpy(exmpl[i].class_name, "forest");
			exmpl[i].class_id = 0;
		} else if (strstr(exmpl[i].id, "river") != NULL) {
			strcpy(exmpl[i].class_name, "river");
			exmpl[i].class_id = 1;
		} else if (strstr(exmpl[i].id, "intersection") != NULL) {
			strcpy(exmpl[i].class_name, "intersection");
			exmpl[i].class_id = 2;
		} else if (strstr(exmpl[i].id, "denseresidential") != NULL) {
			strcpy(exmpl[i].class_name, "denseresidential");
			exmpl[i].class_id = 3;
		} else if (strstr(exmpl[i].id, "buildings") != NULL) {
			strcpy(exmpl[i].class_name, "buildings");
			exmpl[i].class_id = 4;
		}
		//printf("%s is %s(class_id = %d).\n",exmpl[i].id,exmpl[i].class_name,exmpl[i].class_id);
	}

}

/****************************************************************************
 * 関数名 : Compress_Exmpl
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   : データを圧縮特徴量に変換。
 * 引数   :
 * 戻り値 : なし
 ****************************************************************************/
void CompressExmpl(mode m, dict *candi, dict *basis, dict *lbp_basis, int d,
		patch_graph *exmpl_pgr) {

	char textname[128];
	char dicname[128];
	lzw_node** dics;
	FILE *fp;
	int i, j;
	fpos_t tsize, dsize;
	double rate;

	if (m.lbp == 0) {
		dics = initialize_dics(m.num_basis, MAX_PHRASE_NUM);
	} else {
		dics = initialize_dics(m.num_basis * 2, MAX_PHRASE_NUM);
	}

	//基底辞書の読み込み
	for (i = 0; i < m.num_basis; i++) {
		memset(dicname, 0, sizeof(dicname));
		if (m.del_dic == 0 || m.select == 2) {
			sprintf(dicname, BASIS"/%s.dic", basis[i].id);
		} else {
			sprintf(dicname, EDT_BASIS"/%s.dic", basis[i].id);
		}
		if ((fp = fopen(dicname, "rb")) == NULL) {
			printf("file name miss\n");
			exit(1);
		}
		for (j = 0; j < MAX_PHRASE_NUM; j++) {
			dics[i][j].alphabet = get_bits(fp, DICW_SIZE);
			dics[i][j].brother = get_bits(fp, DICW_SIZE);
			dics[i][j].child = get_bits(fp, DICW_SIZE);
			dics[i][j].parent = get_bits(fp, DICW_SIZE);
			dics[i][j].flag = get_bits(fp, DICW_SIZE);
			dics[i][j].depth = get_bits(fp, DICW_SIZE);
		}
		fclose(fp);
	}

	//LBP基底辞書の読み込み
	if (m.lbp == 1) {
		for (i = 0; i < m.num_basis; i++) {
			memset(dicname, 0, sizeof(dicname));
			if (m.del_dic == 0 || m.select == 2) {
				sprintf(dicname, BASIS_LBP"/%s.dic", lbp_basis[i].id);
			} else {
				sprintf(dicname, EDT_BASIS_LBP"/%s.dic", lbp_basis[i].id);
			}
			if ((fp = fopen(dicname, "rb")) == NULL) {
				printf("file name miss\n");
				exit(1);
			}
			for (j = 0; j < MAX_PHRASE_NUM; j++) {
				dics[i + m.num_basis][j].alphabet = get_bits(fp, DICW_SIZE);
				dics[i + m.num_basis][j].brother = get_bits(fp, DICW_SIZE);
				dics[i + m.num_basis][j].child = get_bits(fp, DICW_SIZE);
				dics[i + m.num_basis][j].parent = get_bits(fp, DICW_SIZE);
				dics[i + m.num_basis][j].flag = get_bits(fp, DICW_SIZE);
				dics[i + m.num_basis][j].depth = get_bits(fp, DICW_SIZE);
			}
			fclose(fp);
		}
	}

	//圧縮率ベクトルを求める
	for (i = 0; i < d; i++) {

		if (m.lbp == 0) {
			exmpl_pgr[i].cv_num = (double *) malloc(
					sizeof(double) * m.num_basis);
		} else {
			exmpl_pgr[i].cv_num = (double *) malloc(
					sizeof(double) * m.num_basis * 2);
		}
		memset(textname, 0, sizeof(textname));
		sprintf(textname, TEXT"/%s.txt", candi[i].id);
		memset(exmpl_pgr[i].id, 0, sizeof(exmpl_pgr[i].id));
		strcpy(exmpl_pgr[i].id, candi[i].id);
		/* textファイルのサイズ取得 */
		if ((fp = fopen(textname, "rb")) == NULL) {
			puts("error!");
			exit(1);
		}
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &tsize);
		fclose(fp);
		//基底辞書（カラー画像）で圧縮
		for (j = 0; j < m.num_basis; j++) {
			memset(dicname, 0, sizeof(dicname));
			sprintf(dicname, OUTPUT"/img%d-dic%02d.dic", i + 1, j + 1); //書き出さない
			initialize_file(textname, dicname);
			initilalize_work();
			encode_fix2(dics[j]); /* フラグ有の圧縮 */
			finalize_file();
			/* 圧縮後のサイズ取得 */
			fp = fopen(dicname, "rb");
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &dsize);
			fclose(fp);
			if (remove(dicname) != 0) {
				printf("error\n");
			}
			rate = (double) dsize.__pos / (double) tsize.__pos;
			exmpl_pgr[i].cv_num[j] = rate;
		}

		//基底辞書（LBP画像）で圧縮
		if (m.lbp == 1) {
			sprintf(textname, TEXT_LBP"/%s.txt", candi[i].id);
			/* textファイルのサイズ取得 */
			if ((fp = fopen(textname, "rb")) == NULL) {
				printf("file name miss\n");
				exit(1);
			}
			fseek(fp, 0, SEEK_END);
			fgetpos(fp, &tsize);
			fclose(fp);
			for (j = m.num_basis; j < m.num_basis * 2; j++) {
				sprintf(dicname, OUTPUT"/%02d.dic", j); //書き出さない
				initialize_file(textname, dicname);
				initilalize_work();
				encode_fix2(dics[j]); /* フラグ有の圧縮 */
				finalize_file();
				/* 圧縮後のサイズ取得 */
				fp = fopen(dicname, "rb");
				fseek(fp, 0, SEEK_END);
				fgetpos(fp, &dsize);
				//printf("%s : %f\n",dicname,(double)tsize.__pos);
				fclose(fp);
				if (remove(dicname) != 0) {
					printf("error\n");
				}
				rate = (double) dsize.__pos / (double) tsize.__pos;
				//printf("%f\n",rate);
				exmpl_pgr[i].cv_num[j] = rate;
			}

		}

	}

	char filename[] = "example";
	Out_Features(exmpl_pgr, d, m.num_basis, basis, filename);

	//if(m.lbp == 1){
	//	char filename_lbp[] = "example_lbp";
	//	Out_Features(exmpl_pgr, d, m.num_basis, lbp_basis, filename_lbp);
	//}

}

/****************************************************************************
 * 関数名 : K-NNSearch
 * 作成日 :
 * 作成者 : Y.Nakajima
 * 更新日 :
 * 機能   :　(5クラス分類のみ対応)
 * 引数   :
 * 戻り値 : 平均判別率(正解数 / クエリ数)
 * 注釈	：従来研究に使用？
 ****************************************************************************/
float KnnSearch(int d, mode m, patch_graph *exmpl_pgr, dict *candi,
		knn_result *result) {

	int i, j;
	knn_temp* temp;
	int tempo[5];
	int max;
	int idx;
	int max_index[5];
	int conflict;
	int correct = 0;
	char temp_str[5][20] = { "forest", "river", "intersection",
			"denseresidential", "buildings" };

	//距離計算
	temp = (knn_temp *) malloc(sizeof(knn_temp) * d);

	for (i = 0; i < d; i++) {
		result[i].id = (int *) malloc(sizeof(int) * d);
		result[i].dis = (double *) malloc(sizeof(double) * d);

		for (j = 0; j < d; j++) {
			temp[j].index = j;
			if (m.lbp == 0) {
				temp[j].dis = calc_distance(exmpl_pgr[i].cv_num,
						exmpl_pgr[j].cv_num, m.num_basis);
			} else {
				temp[j].dis = calc_distance(exmpl_pgr[i].cv_num,
						exmpl_pgr[j].cv_num, m.num_basis * 2);
			}
		}
		qsort(temp, d, sizeof(struct knn_temp), listComp2);
		for (j = 0; j < d; j++) {
			result[i].id[j] = temp[j].index;
			result[i].dis[j] = temp[j].dis;
		}
	}

	//判別（k近傍）
	for (i = 0; i < d; i++) {
		result[i].disc_id = (uintptr_t) malloc(sizeof(int));
		for (j = 0; j < 5; j++) {
			tempo[j] = 0;
			max_index[j] = -1;
			conflict = 0;
		}
		for (j = 1; j < m.k; j++) {
			tempo[candi[result[i].id[j]].class_id]++;
		}
		max = tempo[0];
		idx = 0;
		//max_index = 0;
		for (j = 1; j < 5; j++) {
			if (max < tempo[j]) {
				max = tempo[j];
				idx = j;
			}
		}
		for (j = 0; j < 5; j++) {
			if (max == tempo[j]) {
				max_index[conflict] = j;
				conflict++;
			}
		}

		if (conflict - 1 != 0) {
			result[i].disc_id = max_index[rand() % conflict];
		} else {
			result[i].disc_id = idx;
		}
		if (result[i].disc_id == 0) {
			//	printf("%s is forest\n",candi[i].id);
		} else if (result[i].disc_id == 1) {
			//	printf("%s is river\n",candi[i].id);
		} else if (result[i].disc_id == 2) {
			//	printf("%s is intersection\n",candi[i].id);
		} else if (result[i].disc_id == 3) {
			//	printf("%s is denseresidential\n",candi[i].id);
		} else if (result[i].disc_id == 4) {
			//	printf("%s is buildings\n",candi[i].id);
		}
	}

	//4.正答率計算
	for (i = 0; i < d; i++) {
		if (result[i].disc_id == candi[i].class_id) {
			correct++;
		}
	}
	printf("correct = %d,d=%d\n", correct, d);
	printf("%f\n", (double) correct / d);

	//クラスごとの正答率(5クラスver)
	int correct_class[5];
	memset(correct_class, 0, sizeof(correct_class));
	for (i = 0; i < d; i++) {
		for (j = 0; j < 5; j++) {
			if (candi[i].class_id == j) {
				if (result[i].disc_id == candi[i].class_id) {
					correct_class[j]++;
				}
			}
		}
	}

	for (i = 0; i < 5; i++) {
		printf("class_id %d(%s) precision = %f \n", i, &temp_str[i][0],
				(double) correct_class[i] / (d / 5));
	}

	free(temp);

	return (double) correct / d;

#ifdef knn
#endif

}
