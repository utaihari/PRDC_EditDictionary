#ifndef INCLUDE_BITIO
#define INCLUDE_BITIO

int get_bit(FILE* fp);
int get_bits(FILE* fp,int n);
void flush_bits(FILE *fp);
int put_bit(FILE* fp,int bit);
void put_bits(FILE* fp,int byte, int n);

#endif