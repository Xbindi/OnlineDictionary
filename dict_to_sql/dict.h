#ifndef _DICT_H_
#define _DICT_H_

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_WORD 128

// 插入数据库
int insert_dict(sqlite3* dictDB, FILE* fp);
// 获取音标
void get_phonetic(char* buf, char* phonetic);
// 获取翻译
void get_translate(char* buf, char* translate);

#endif // !_DICT_H_