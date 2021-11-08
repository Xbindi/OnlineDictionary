#include "dict.h"

#define DATABASE "dict.db"
#define WOEDTXT "word.txt"
int main(int argc, char* argv[])
{
    // 数据库指针
    sqlite3* dictDB = NULL;
    if(sqlite3_open(DATABASE,&dictDB) != SQLITE_OK)
    {
        fprintf(stderr, "%s\n", sqlite3_errmsg(dictDB));
        sqlite3_close(dictDB);
        return -1;
    }
    printf("open dict.db succeed\n");

    // 以只读方式打开word.txt文本文件
    FILE* fp = fopen(WOEDTXT, "r");
    if(NULL == fp)
    {
        perror("fopen");
        sqlite3_close(dictDB);
        return -1;
    }
    printf("open word.txt succeed\n");

    insert_dict(dictDB, fp);

    fclose(fp);
    if(sqlite3_close(dictDB) != SQLITE_OK)
    {
        fprintf(stderr, "%s\n", sqlite3_errmsg(dictDB));
    }
    return 0;
}

/*
 * Function   : insert_dict
 * Description: 将txt文件数据插入数据库
 * Input      : dictDB  数据库指针
 *              fp      文件指针
 * Output     :
 * Return     : OK      0
 *              ERROE   -1
*/
int insert_dict(sqlite3* dictDB, FILE* fp)
{
    // 调用sqlite3接口时的错误消息
    char* errmsg = NULL;
    // 创建dict表
    char* createTable = "create table if not exists Word(word char, phonetic char, translate char)";
    if(sqlite3_exec(dictDB, createTable, NULL, NULL, &errmsg) != SQLITE_OK)
    {
        fprintf(stderr, "%s\n", errmsg);
        return -1;
    }
    printf("create dict table succeed\n");

    char wordBuf[MAX_WORD]   = "";
    char word[30]            = "";
    char phonetic[30]        = "";
    char translate[MAX_WORD] = "";
    char insertTable[BUFSIZ] = "";
    // 循环从word.txt中读取一行数据后，写入数据库中
    while(1)
    {
        bzero(wordBuf, sizeof(wordBuf));
        bzero(word, sizeof(word));
        bzero(phonetic, sizeof(phonetic));
        bzero(translate, sizeof(translate));
        // 读取一行
        if(NULL == fgets(wordBuf, sizeof(wordBuf), fp))
        {  
            printf("fgets all word and insert word succeed\n");
            break;
        }
        //printf("===0---->%s\n", wordBuf);
        // 获取单词  sscanf格式化“%s”时遇到空格结束
        sscanf(wordBuf, "%s", word);
        //printf("===1---->%s\n", word);
        // 获取音标
        get_phonetic(wordBuf, phonetic);
        // 获取翻译
        get_translate(wordBuf, translate);

        //将一行对应的 单词，音标，翻译 写入数据库
        bzero(insertTable, sizeof(insertTable));
        sprintf(insertTable, "insert into Word values(\"%s\",\"%s\",\"%s\")", word, phonetic, translate);
        if(sqlite3_exec(dictDB, insertTable, NULL, NULL, &errmsg) != SQLITE_OK)
        {
            fprintf(stderr, "%s:%s\n", word, errmsg);
            break;
        }
    }
    return 0;
}
/*
 * Function   : get_phonetic
 * Description: 从buf中截取音标部分
 * Input      : buf       存了一行数据
 *              phonetic  将截取音标部分保存
 * Return     : 
*/ 
void get_phonetic(char* buf, char* phonetic)
{
    while(*buf)
    {
        if(*buf == '[')
        {
            while(*buf != ']')
            {
                *phonetic = *buf;
                phonetic++;
                buf++;
            }
            *phonetic = *buf;
            break;
        }
        buf++;
    }
}
/*
 * Function   : get_phonetic
 * Description: 从buf中截取翻译部分
 * Input      : buf       存了一行数据
 *              translate 将截取翻译部分保存
 * Return     : 
*/ 
void get_translate(char* buf, char* translate)
{
    while(*buf)
    {
        if(*buf == ']')
        {
            buf += 2;
            strcpy(translate, buf);
            break;
        }
        buf++;
    }
}