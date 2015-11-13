#ifndef INCLUDE_CVMST
#define INCLUDE_CVMST


#include"ds.h"


/*CVMST*/
struct patch_graph{
	int node_num;
	/*パッチナンバ*/
	double *cv_num;
	/*圧縮率ベクトル*/
	int c_patch[4];
	/*関連パッチナンバー(下右上左)*/
	struct patch_graph_edge* pgredge[4];
	/*下右上左のエッジ*/
	int root;
	/*根ノード保存*/
	int cl_num;
	/*所属クラスタ番号*/
	double *color_ave;
	/*色情報(RGBそれぞれの平均値)*/
	/*ファイル名*/
	char id[64];

};
/*ノード(パッチ)情報構造体*/

struct dict{
	double self_rate; //自己圧縮率?
	/*ファイル名*/
	char id[128];
	char dic_id[128];
	char img_id[128];
	char txt_id[128];
	char class_name[64];
	int class_id;
	double sum_rate;
	int flag;
	double dis;
	int *exmpl_id;
	int id2;
	double rate;
};

struct knn_result{
	double *dis;
	int *id;
	int disc_id;
};

struct knn_temp{
	int index;
	double dis;
};

struct patch_graph_edge{
	int b_pgr;
	/*始点パッチ*/
	int a_pgr;
	/*終点パッチ*/
	double weight;
	/*重み*/
	int mst_hrg;
	/*MSTフラグ*/
	int shroud_hrg;
	/*被覆エッジ*/
	int cutted_hrg;
	/*切断エッジ*/
};
/*エッジ情報構造体*/


int Rootbreaker_Wurm(patch_graph *pgr,patch_graph_edge *pgr_edge,int n);
void Mana_Severance(patch_graph *pgr,patch_graph_edge *pgre,double per,int n,mode m);
void Hypergenesis(patch_graph *pgr,int cpr_num,int root_num);
void Beacon_of_Destiny(patch_graph *pgr,patch_graph_edge *pgre);
int compare_cv(const void* _a,const void* _b);
void cvmst(patch_graph *pgr,patch_graph_edge *pgr_edge,int d,int n);
void Cultivate(patch_graph *pgr,patch_graph_edge *pgr_edge,mode m,int d,int n);
void Leveler(patch_graph *pgr,int d,int w,int h);


#endif
