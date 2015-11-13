﻿#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"lzw.h"
#include"bitIO.h"

/* 作業用変数 */
FILE* in_file; /* 入力ファイルのポインタ */
FILE* out_file; /* 出力ファイルのポインタ */
int phrase; /* 現在のフレーズナンバー(フレーズ数）-1 */
int now; /* 現在のノード */

/****************************************************************************
 * 関数名 : revise_flag
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 :
 * 機能   : 2のフラグを0にする
 * 引数   : 辞書:dic
 * 戻り値 :
 ****************************************************************************/
void revise_flag(lzw_node* dic) {

	int j;

	for (j = MAX_PHRASE_NUM - 1; j > 256; j--) {
		if (dic[j].flag == 2) {
			dic[j].flag = 0;
		}
	}
}

/****************************************************************************
 * 関数名 : compensate_phrase
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 :
 * 機能   : 削除したぶんだけ単語を追加
 * 引数   : 辞書:dic
 * 戻り値 : 補完した単語数
 ****************************************************************************/
int compensate_phrase(lzw_node* dic) {

	int i;
	int phrase_num, delete_num, add_num = 0;

	phrase_num = count_phrase(dic, dic);
	delete_num = dic->pre_phrase_num - phrase_num;
	if (delete_num < 0) {
		return 0;
	}

	for (i = PHRASE_NUM; i < MAX_PHRASE_NUM; i++) {
		if (dic[i].flag == 0) {
			//printf("dic[%d].flag = 1\n",i);
			dic[i].flag = 1;
			add_num++;
		}
		if (add_num == delete_num) {
			break;
		}
	}
	printf("added %d phrase (delete_num = %d)\n", add_num, delete_num);
	phrase_num = count_phrase(dic, dic);
	printf("phrase_num = %d\n", phrase_num + 256);
	return add_num;

}

/****************************************************************************
 * 関数名 : search_child
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 指定した辞書の子ノードの節点検索
 * 引数   : 辞書:dic, 検索したい要素:symbol
 * 戻り値 : フレーズナンバー:n
 ****************************************************************************/
int search_child(lzw_node* dic, int symbol) {
	int n;
	/* 現在ノードがルートなら初期辞書の最初を対象にする */
	if (now == ROOT) {
		n = 0;
	} else {
		n = dic[now].child; /* 現在のノードの子を代入 */
	}
	/* 兄弟を見ていく */
	while (n != NUL) {
		/* 記号が一致すればそのフレーズナンバーを返す */
		if (dic[n].alphabet == symbol) {
			return n;
		}
		n = dic[n].brother; /* 次の兄弟へ */
	}
	/* 一致がなかったらNULLを返す */
	return NUL;
}

/****************************************************************************
 * 関数名 : output_phrase
 * 作成日 :
 * 作成者 : Nakajima.Y
 * 更新日 :
 * 機能   : 辞書の単語を画面に出力
 * 引数   : 辞書
 * 戻り値 :
 ****************************************************************************/
void output_phrase(lzw_node* dic1) {

	int j;
	int *pre_n, now_n, plength;
	char *nphrase;
	int parent;
	int i;

	nphrase = (char *) malloc(sizeof(char) * (MAX_PHRASE_NUM));
	pre_n = (int *) malloc(sizeof(int));

	for (j = MAX_PHRASE_NUM - 1; j > 256; j--) {
		/* 子クラスタ1の処理 */
		if (dic1[j].flag == 1) {
			plength = dic1[j].depth; /* フレーズの長さを取得 */
			nphrase[dic1[j].depth - 1] = dic1[j].alphabet;
			nphrase[dic1[j].depth] = EOF; /* 終端記号の登録 */
			parent = dic1[j].parent;
			while (parent != ROOT) {
				nphrase[dic1[parent].depth - 1] = dic1[parent].alphabet;
				parent = dic1[parent].parent; /* 書き変え */
			}
			printf("phrase%d:", j);
			for (i = 0; i < dic1[j].depth; i++) {
				printf("%c", nphrase[i]);
			}
			printf("\n");
		}

	}

	free(nphrase);
	free(pre_n);

}

/****************************************************************************
 * 関数名 : search_child
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 指定した辞書の子ノードの節点検索
 * 引数   : 辞書:dic, 検索したい要素:symbol, 現在のフレーズナンバー:pre
 * 戻り値 : フレーズナンバー:n
 ****************************************************************************/
int search_child2(lzw_node* dic, int symbol, int* pre) {
	int n;
	/* 現在ノードがルートなら初期辞書の最初を対象にする */
	if (*pre == ROOT) {
		n = 0;
	} else {
		n = dic[*pre].child; /* 現在のノードの子を代入 */
	}
	/* 兄弟を見ていく */
	while (n != NUL) {
		/* 記号が一致すればそのフレーズナンバーを返す */
		if (dic[n].alphabet == symbol) {
			return n;
		}
		*pre = n;
		n = dic[n].brother; /* 次の兄弟へ */
	}
	/* 一致がなかったらNULLを返す */
	return NUL;
}

/****************************************************************************
 * 関数名 : search_phrase
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 指定した辞書に目的のフレーズが登録されているかみる
 * 引数   : 辞書:dic, 検索したいフレーズ:phrase, フレーズの長さ:pnum, 1個前のフレーズナンバー:pre_n
 * 戻り値 : フレーズナンバー:n
 ****************************************************************************/
int search_phrase(lzw_node* dic, char* phrase, int pnum, int* pre_n) {
	int i, now_n;
	now_n = ROOT; /* 初期化 */
	for (i = 0; i < pnum; i++) {
		*pre_n = now_n;
		now_n = search_child2(dic, phrase[i], pre_n);
		if (now_n == NUL) {
			break; /* 途中で無いとわかったので */
		}
	}
	if (i != pnum) {
		return NUL; /* 一致無し */
	}
	return now_n;
}

/****************************************************************************
 * 関数名 : search_pre_node
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 1つ前のノードを探す関数
 * 引数   : 辞書:dic, ノード番号:now
 * 戻り値 : 1つ前のノードの番号
 ****************************************************************************/
int search_pre_node(lzw_node* dic, int nnode) {
	int pnode;
	pnode = dic[nnode].parent;
	/* まず親を見る */
	if (dic[pnode].child == nnode) {
		return pnode;
	}
	pnode = dic[pnode].child;
	while (dic[pnode].brother != nnode) {
		pnode = dic[pnode].brother;
	}
	return pnode;
}

/****************************************************************************
 * 関数名 : delete_phrase
 * 作成日 :
 * 作成者 : Yuji Nakajima
 * 更新日 :
 * 機能   : 指定したフレーズを辞書から削除(フラグ処理)する関数
 * 引数   : 辞書:dic, 削除したいフレーズの番号:now_n
 * 戻り値 : セットした値に応じて1か0を返す
 ****************************************************************************/
int delete_phrase(lzw_node* dic, int now_n) {
	if (dic[now_n].flag == 1) {
		dic[now_n].flag = 0;
		return 1;
	} else {
		dic[now_n].flag = 2; //flagが0だが、共通してた場合
		return 0;
	}
}

/****************************************************************************
 * 関数名 : count_phrase
 * 作成日 :
 * 作成者 : Nakajima.Y
 * 更新日 :
 * 機能   : 辞書同士の共通単語を数え上げる
 * 引数   : 辞書１、辞書２
 * 戻り値 : 一致した単語数
 ****************************************************************************/
int count_phrase(lzw_node* dic1, lzw_node* dic2) {

	int j;
	int *pre_n, now_n, plength;
	char *nphrase;
	int parent;
	int count = 0;

	nphrase = (char *) malloc(sizeof(char) * (MAX_PHRASE_NUM));
	pre_n = (int *) malloc(sizeof(int));

	for (j = MAX_PHRASE_NUM - 1; j > 256; j--) {
		/* 子クラスタ1の処理 */
		if (dic1[j].flag == 1) {
			plength = dic1[j].depth; /* フレーズの長さを取得 */
			nphrase[dic1[j].depth - 1] = dic1[j].alphabet;
			nphrase[dic1[j].depth] = EOF; /* 終端記号の登録 */
			parent = dic1[j].parent;
			while (parent != ROOT) {
				nphrase[dic1[parent].depth - 1] = dic1[parent].alphabet;
				parent = dic1[parent].parent; /* 書き変え */
			}
			/* c1のフレーズが取れたので辞書登録 */
			*pre_n = ROOT;
			now_n = search_phrase(dic2, nphrase, plength, pre_n);
			//printf("j = %d\n",j);
			if (now_n != NUL && dic2[now_n].flag == 1) {
				//puts("hit!!!!!");
				count++;
			}
		}

	}

	free(nphrase);
	free(pre_n);

	return count;

}

/****************************************************************************
 * 関数名 : delete_cophrase
 * 作成日 :
 * 作成者 : Nakajima.Y
 * 更新日 :
 * 機能   : 辞書同士の共通単語を削除
 * 引数   : 辞書１、辞書２（削除される側）
 * 戻り値 :
 ****************************************************************************/
void delete_cophrase(lzw_node* dic1, lzw_node* dic2) {

	int j;
	int *pre_n, now_n, plength;
	char *nphrase;
	int parent;
	int del_num = 0, del;

	nphrase = (char *) malloc(sizeof(char) * (MAX_PHRASE_NUM));
	pre_n = (int *) malloc(sizeof(int));

	for (j = MAX_PHRASE_NUM - 1; j > 256; j--) {
		/* 子クラスタ1の処理 */
		if (dic1[j].flag == 1) {
			plength = dic1[j].depth; /* フレーズの長さを取得 */
			nphrase[dic1[j].depth - 1] = dic1[j].alphabet;
			nphrase[dic1[j].depth] = EOF; /* 終端記号の登録 */
			parent = dic1[j].parent;
			while (parent != ROOT) {
				nphrase[dic1[parent].depth - 1] = dic1[parent].alphabet;
				parent = dic1[parent].parent; /* 書き変え */
			}
			/* c1のフレーズが取れたので辞書登録 */
			*pre_n = ROOT;
			now_n = search_phrase(dic2, nphrase, plength, pre_n);
			if (now_n != NUL) {
				del = delete_phrase(dic2, now_n);
				del_num += del;
			}
		}
	}

	free(nphrase);
	free(pre_n);
	//printf("deleted %d phrase\n",del_num);
}

/****************************************************************************
 * 関数名 : encode_fix2
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 固定辞書での符号化関数（辞書登録がないだけ）。フラグも見る
 * 引数   : 辞書:dic
 * 戻り値 : なし
 ****************************************************************************/
void encode_fix2(lzw_node* dic) {
	int symbol, n, i, j;
	int queue[500];

	for (i = 0; i < 500; i++) {
		queue[i] = 0;
	}
	i = 0;
	/* 親フレーズの初期設定 */
	now = search_child(dic, getc(in_file)); /* 1文字目を初期辞書から検索 */
	queue[i++] = now;
	//printf("queue[0] = %c\n",now);
	while (1) {
		symbol = getc(in_file); /* 入力文字 */
		queue[i] = symbol;
		//printf("queue[%d] = %c\n",i,symbol);
		if (symbol == EOF) {
			//puts("aaaa");
			/* フラグがおｋだったら */
			if (dic[now].flag == 1) {
				/* 符号化して終了 */
				put_bits(out_file, now, BIT_SIZE);
				put_bits(out_file, 256, BIT_SIZE); /* EOFの出力 */
				flush_bits(out_file); /* 出力し切れなかったものを出力 */
				break;
			}
			/* フラグがダメだったら */
			else {
#if 1
				for (j = 0; j < dic[now].depth; j++) {
					put_bits(out_file, queue[j], BIT_SIZE);
				}
				put_bits(out_file, 256, BIT_SIZE); /* EOFの出力 */
				flush_bits(out_file); /* 出力し切れなかったものを出力 */
				break;
#else
				while(!dic[now].flag) {
					/* ノードを一つ戻す */
					i--;
					symbol = now;
					now = dic[now].parent;
					fseek(in_file,-1L,SEEK_CUR); /* ファイルも1文字戻す */
				}
				//put_bits(out_file,now,BIT_SIZE);
				now = dic[symbol].alphabet;
				fseek(in_file,1L,SEEK_CUR);
				continue;
#endif
			}
		}
		n = search_child(dic, symbol); /* (新フレーズ⊂辞書)? */
		/* 入力文字が子ノードに含まれるなら節点移動し探索続行 */
		if (n != NUL) {
			now = n;
			i++;
		}
		/* 子ノードに含まれないなら最長一致なので符号化 */
		else {
			//puts("bbbb");
			/* フラグが立っていなかったら符号化可能なところまで戻って符号化 */
			if (dic[now].flag == 0) {
				for (j = 0; j < dic[now].depth; j++) {
					put_bits(out_file, queue[j], BIT_SIZE);
					//printf("invalid!!! \n queue[%d] = %c\n",j,queue[j]);
				}
				now = dic[symbol].alphabet;
				i = 0;
				queue[i++] = symbol;


				/* ノードを一つ戻す */
//				symbol = now;
//				now = dic[now].parent;
//				//printf("%c \n",now);
//				fseek(in_file,-1L,SEEK_CUR);	/* ファイルも1文字戻す */
			} else {
				//printf("%c\n",symbol);
				put_bits(out_file, now, BIT_SIZE);
				now = dic[symbol].alphabet;
				i = 0;
				queue[i] = symbol;
				i++;
//				queue[i++] = symbol;
			}
		}
	}
}

/****************************************************************************
 * 関数名 : finalize_file
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 入出力ファイルの後始末
 * 引数   : なし
 * 戻り値 : なし
 ****************************************************************************/
void finalize_file() {
	fclose(in_file);
	fclose(out_file);
}

/****************************************************************************
 * 関数名 : new_node
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 指定した辞書に新しいフレーズを登録する
 * 引数   : 辞書:dic, 加える要素:symbol, 最大フレーズ数:pnum
 * 戻り値 : 辞書登録できたか: 0 or 1
 ****************************************************************************/
int new_node(lzw_node* dic, int symbol, int pnum, int max_pnum) {
	/* 最大フレーズ数以下なら追加 */
	if (phrase < max_pnum) {
		dic[phrase].alphabet = symbol;
		dic[phrase].brother = dic[now].child;
		dic[phrase].child = NUL;
		dic[now].child = phrase;
		dic[phrase].parent = now;
		if (phrase < pnum) {
			dic[phrase].flag = 1;
		} else {
			dic[phrase].flag = 0;
		}
		dic[phrase].depth = dic[now].depth + 1;
		phrase++; /* フレーズ追加したので */
		return 1;
	} else {
		return 0;
	}
}

/****************************************************************************
 * 関数名 : encode
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 符号化関数（作る辞書を指定）
 * 引数   : 辞書:dic
 * 戻り値 : なし
 ****************************************************************************/
void encode(lzw_node* dic) {
	int symbol, n;
	/* 親フレーズの初期設定 */
	symbol = getc(in_file);
	now = search_child(dic, symbol);
	while (1) {
		symbol = getc(in_file); /* 入力文字 */
		if (symbol == EOF) {
			/* 符号化して終了 */
			put_bits(out_file, now, BIT_SIZE);
			put_bits(out_file, 256, BIT_SIZE); /* EOFの出力 */
			flush_bits(out_file); /* 出力し切れなかったものを出力 */
			break;
		}
		n = search_child(dic, symbol); /* (新フレーズ⊂辞書)? */
		/* 入力文字が子ノードに含まれるなら節点移動し探索続行 */
		if (n != NUL) {
			now = n;
		}
		/* 子ノードに含まれないなら最長一致なので符号化 */
		else {
			put_bits(out_file, now, BIT_SIZE); /* フレーズナンバーがそのまま符号語 */
			new_node(dic, symbol, PHRASE_NUM, MAX_PHRASE_NUM); /* 新たなフレーズを辞書に追加 */
			now = symbol; /* 入力文字を親フレーズとする */
		}
	}
}

/****************************************************************************
 * 関数名 : initialize_work
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 作業用の変数の初期化（グローバル変数だから。なくしたい）
 * 引数   : なし
 * 戻り値 : なし
 ****************************************************************************/
void initilalize_work() {
	/* 作業用変数の初期化 */
	phrase = 257;
	now = ROOT;
}

/****************************************************************************
 * 関数名 : initialize_file
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 入出力ファイルの初期化
 * 引数   : 入力ファイル名:argv1, 出力ファイル名:argv2
 * 戻り値 : なし
 ****************************************************************************/
void initialize_file(char* argv1, char* argv2) {
	/* 入力ファイル読み込み */
	if ((in_file = fopen(argv1, "rb")) == NULL) {
		printf("input file miss 1 %s\n", argv1);
		exit(1);
	}

	/* 出力ファイル読み込み */
	if ((out_file = fopen(argv2, "wb")) == NULL) {
		printf("output file miss 1\n");
		exit(1);
	}
}

/****************************************************************************
 * 関数名 : initialize_dics
 * 作成日 :
 * 作成者 : S.Kobayashi
 * 更新日 :
 * 機能   : 辞書群の初期化関数
 * 引数   : 辞書数:dnum, 最大フレーズ数:pnum
 * 戻り値 : 辞書dics
 ****************************************************************************/
lzw_node** initialize_dics(int dnum, int pnum) {
	int i, j;
	lzw_node** dics;
	dics = (lzw_node **) malloc(sizeof(lzw_node) * dnum);
	if (dics == NULL) {
		printf("memory securing miss (dics in [initialize_dics])\n");
		exit(1);
	}
	for (i = 0; i < dnum; i++) {
		//printf("i=%d \n",i);
		dics[i] = (lzw_node *) malloc(sizeof(lzw_node) * pnum);
		//puts("bbc");
		if (dics[i] == NULL) {
			printf("memory securing miss (graph[%d] in [initilalize_dics])\n",
					i);
			exit(1);
		}
		//puts("bbc");
		for (j = 0; j < 256; j++) {
			dics[i][j].alphabet = j;
			dics[i][j].brother = j + 1;
			dics[i][j].child = NUL;
			dics[i][j].parent = ROOT;
			dics[i][j].flag = 1;
			dics[i][j].depth = 1;
		}
		//puts("ccc");
		/* EOFの登録 */
		dics[i][256].alphabet = EOF;
		dics[i][256].brother = NUL;
		dics[i][256].child = NUL;
		dics[i][256].parent = ROOT;
		dics[i][256].flag = 1;
		dics[i][256].depth = NUL;
	}
	return dics;
}
