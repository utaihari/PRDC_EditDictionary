#ifndef INCLUDE_IMAGE2TEXT
#define INCLUDE_IMAGE2TEXT

#include<opencv2/opencv.hpp>
#include"ds.h"

/* 画像保存のデータ構造 */
struct imagedata{
	char character;	/* 符号語 */
	int x;	/* x座標 */
	int y;	/* y座標 */
	CvScalar clr;	/* 色情報 */
};

void breakLink(node** graph,edge* elist,int edge_num,edge* mstlist, int mst_num);
int compColor(CvScalar color1,CvScalar color2);
void convert2text_horizon(FILE* txt,node* graph,mode nmode);
node* get_que(que* first,int* w);
void add_que(que* first, node* data,int way);
void ltsearch(FILE* txt,node* graph,int way,mode nmode);
void dfsearch(FILE* txt,node* graph,int way,mode nmode);
void bfsearch(FILE* txt,node* graph,int way,mode nmode);
FILE* graph2text(node** graph,edge* elist,int node_num,FILE* txt,mode nmode);
void makeLink(node** graph,edge* elist,int edge_num,edge* mstlist, int mst_num);
void setWeight(edge* nedge,mode nmode);
void quantizationRGB4d(node** graph,int width,int height);
void image2graph(IplImage* img, node** graph, edge* elist,mode nmode);
FILE* image2text_horizon(IplImage* img,char* text,mode nmode);
FILE* image2text_mst(IplImage* img,char* text,mode nmode);
FILE* image2text_lbp(IplImage* img,char* text,mode nmode);
void convert2text_mst(FILE* txt,node* graph,int way,mode nmode);

#endif
