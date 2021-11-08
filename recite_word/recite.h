#ifndef _RECITE_H_
#define _RECITE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <malloc.h>
#include <time.h>

#define MAX_WORD  170
#define MAX       128

// 将数据库全部保存到allWord中,此接口遇到段错误问题，先不用
// int down_sql(char* word[], char* phonetic[], char* translate[]);
// 将word.txt种的单词，音标及翻译保存到变量中
int down_load(char* word[], char* phonetic[], char* translate[]);
void delte_all(char* word[], char* phonetic[], char* translate[]);
// 获取音标
void get_phonetic(char* buf, char* phonetic);
// 获取翻译
void get_translate(char* buf, char* translate);

int ch_to_en(int num, char* word[], char* phonetic[], char* translate[]);
int en_to_ch(int num, char* word[], char* phonetic[], char* translate[]);

#endif // !_RECITE_H_