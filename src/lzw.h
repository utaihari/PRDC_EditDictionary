#ifndef INCLUDE_LZW
#define INCLUDE_LZW
/*
*	LZWで圧縮する関数群
*/
#define BIT_SIZE		13
#define DICW_SIZE		32
#define PHRASE			32
//#define AAA	(1 << 12)	/* 最大フレーズ数 2^12 */
#define PHRASE_NUM	 4096
#define MAX_PHRASE_NUM	 4096	/* 最大フレーズ数 2^12 */
#define ROOT			-1
#define NUL				-2

/* ノードを表す構造体 */
struct lzw_node{
	int alphabet;	/* 節点の記号 */
	int brother;	/* 節点の次の兄弟ノード */
	int child;		/* 節点の子ノード */
	int parent;		/* 節点の親ノード */
	int flag;		/* ここで圧縮していいかのフラグ 1(ok) 0(no) */
	int depth;		/* 節点の深さ */
	int pre_phrase_num; /*削除前の有効な単語数 */
};

void encode_fix2(lzw_node* dic);
void finalize_file();
int new_node(lzw_node* dic,int symbol,int pnum);
int search_child(lzw_node* dic,int symbol);
void encode(lzw_node* dic);
void initilalize_work();
void initialize_file(char* argv1,char* argv2);
lzw_node** initialize_dics(int dnum,int pnum);
int count_phrase(lzw_node*,lzw_node*);
void delete_cophrase(lzw_node*,lzw_node*);
void output_phrase(lzw_node*);
int compensate_phrase(lzw_node*);
void revise_flag(lzw_node* );

#endif
