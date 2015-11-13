#include <stdio.h>
#include "bitIO.h"

/****************************************************************************
* 関数名 : get_bit
* 作成日 : 
* 作成者 : S.Kobayashi
* 更新日 : 
* 機能   : ファイルから1bit読み込む関数
* 引数   : 読み込み対象ファイル:fp
* 戻り値 : 1 or 0
****************************************************************************/
int get_bit(FILE* fp){
	static int gcnt = 0,buff = 0;
	gcnt -= 1;
	/* 1byte分読みこむ */
	if(gcnt < 0){
		if((buff = getc(fp)) == EOF){
			/* EOFなら終了 */
			return EOF;
		}
		gcnt = 7;
	}
	/* bitの位置のビットを返す */
	return ((buff >> gcnt)&1);
}

/****************************************************************************
* 関数名 : get_bits
* 作成日 : 
* 作成者 : S.Kobayashi
* 更新日 : 
* 機能   : ファイルからn bit読み込む関数
* 引数   : 対象ファイル:fp, 読み込むbit数:n
* 戻り値 : int型の値:value
****************************************************************************/
int get_bits(FILE* fp,int n){
	int i,v,value = 0;
	for(i = n-1; i>=0; i--){
		/* byteのi番目を入力 */
		v = get_bit(fp);
		if(v == EOF){
			return value;
		}
		else if(v > 0){
		value |= (1 << i);	/* ループが終わると値が格納されている */
		}
	}
	return value;
}

/****************************************************************************
* 関数名 : flush_bits
* 作成日 : 
* 作成者 : S.Kobayashi
* 更新日 : 
* 機能   : 残ってしまったbitを0で埋めて出力する関数
* 引数   : 対象ファイル:fp
* 戻り値 : なし
****************************************************************************/
void flush_bits(FILE *fp){
	while(put_bit(fp,1) != 8);
}

/****************************************************************************
* 関数名 : put_bit
* 作成日 : 
* 作成者 : S.Kobayashi
* 更新日 : 
* 機能   : 8bit溜まったらファイルに書きだす関数
* 引数   : 対象ファイル:fp, bitの値:bit
* 戻り値 : ビットカウンタ:pcnt
****************************************************************************/
int put_bit(FILE* fp,int bit){
	static int pcnt = 8;
	static unsigned char buff = 0;
	pcnt -= 1;
	if(bit > 0){
		buff |= (1 << pcnt);	/* buffのpcntの位置に1をセット */
	}
	/* バッファに8bit貯まったのでファイルに出力 */
	if(pcnt == 0){
		putc(buff,fp);
		/* カウンタとバッファの初期化 */
		pcnt = 8;
		buff = 0;
	}
	return pcnt;
}

/****************************************************************************
* 関数名 : put_bits
* 作成日 : 
* 作成者 : S.Kobayashi
* 更新日 : 
* 機能   : 指定された値byte をn bitだけ出力する関数
* 引数   : 対象ファイル:fp, 出力対象:byte, ビット数:n
* 戻り値 : なし
****************************************************************************/
void put_bits(FILE* fp,int byte, int n){
	int i;
	for(i = n-1; i>=0; i--){
		/* byteのi番目を出力 */
		put_bit(fp,((byte >> i)&1));
	}
}
