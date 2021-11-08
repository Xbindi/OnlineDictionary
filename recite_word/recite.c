#include "recite.h"

/*
 * Function   : down_load
 * Description: 将word.txt中的单词，音标及翻译保存导变量中
 * Input      : word       单词
 *              phonetic   音标
 *              translate  翻译
 * Return     : OK      0
 *              ERROE   -1
*/
int down_load(char* word[], char* phonetic[], char* translate[])
{
    int id = 0;
    char wordBuf[MAX] = "";
    char wordTemp[20] = "";
    FILE* fp = fopen("./word.txt", "r");
    if(NULL == fp)
    {
        perror("fopen");
        return -1;
    }
    printf("open word.txt succeed\n");

    while(1)
    {
        memset(wordBuf, 0, sizeof(wordBuf));
        if(NULL == fgets(wordBuf, sizeof(wordBuf), fp))
        {
            // perror("fgets");
            printf("download word succeed\n");
            break;
        }
        // 获取单词
        sscanf(wordBuf, "%s", wordTemp);
        strcpy(word[id], wordTemp);
        // 获取音标
        get_phonetic(wordBuf, phonetic[id]);
        // 获取翻译
        get_translate(wordBuf, translate[id]);
        id++;
    }
    fclose(fp);
    fp = NULL;
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
    // 末尾补'\0'
    phonetic++;
    *phonetic = '\0';
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
            translate[strlen(translate)-1] = '\0';
            break;
        }
        buf++;
    }
}
void delte_all(char* word[], char* phonetic[], char* translate[])
{
    for(int i=0; i<MAX_WORD; i++)
    {
        if(word[i] != NULL || phonetic[i] != NULL || translate[i] != NULL)
        {
            free(word[i]);
            free(phonetic[i]);
            free(translate[i]);
        }
    }
}


int ch_to_en(int num, char* word[], char* phonetic[], char* translate[])
{
    char input[MAX] = "";
    printf("\033[35m(q退出) %s\033[0m\n", translate[num]);//35紫 34蓝 33黄 32绿 31红
    scanf("%s", input);
    while(getchar() != '\n');
    if(0 == strcmp(input, "q") || 0 == strcmp(input, "Q")) {return -1;}
    // 打印正确 单词 音标 翻译
    printf("\033[33m%s %s %s\033[0m\n",word[num], phonetic[num], translate[num]);
    if(0 != strncmp(input, word[num], strlen(word[num])-1))
    {
        // 重复输入英文直到输入正确
        while(1)
        {
            printf("please input right word !!! >_<\n");
            scanf("%s", input);
            while(getchar() != '\n');
            if(0 == strncmp(input, word[num], strlen(word[num]))) 
            {
                printf("Finally, the input is correct , got to next !!! ^_^\n");
                break;
            }
        }
    }
    return 0;
}

int en_to_ch(int num, char* word[], char* phonetic[], char* translate[])
{
    char input[MAX] = "";
    printf("\033[34m(q退出) %s\033[0m\n",word[num]);
    scanf("%s", input);
    while (getchar() != '\n');
    if(0 == strcmp(input, "q") || 0 == strcmp(input, "Q")) {return -1;}
    // 打印正确 单词 音标 翻译
    printf("\033[31m%s %s %s\033[0m\n",word[num], phonetic[num], translate[num]);
    // 输出中文即可返回
    return 0;
}

/*
 * Function   : down_sql 此接口遇到段错误问题，先不用
 * Description: 将数据库全部保存到allWord中
 * Input      : word       单词
 *              phonetic   音标
 *              translate  翻译
 * Return     : OK      0
 *              ERROE   -1
*/
/*
int down_sql(char* word[], char* phonetic[], char* translate[])
{
    sqlite3* db = NULL;
    if(sqlite3_open("./recite_word/include/dict.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    printf("open dict.db succeed\n");

    char sql[] = "select * from dict";
    sqlite3_stmt* stmt = NULL;
    int idx = 0;
    if(sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Can't open table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    // right align output format
    printf("%10s %10s %10s\n", "word", "phonetic", "translate");
    printf("---------- ---------- ----------\n");
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        printf("1\n");
        strcpy(word[idx], sqlite3_column_text(stmt, 0));
        strcpy(phonetic[idx], sqlite3_column_text(stmt, 1));
        strcpy(translate[idx], sqlite3_column_text(stmt, 2));
        printf("%10s %10s %10s\n", word[idx], phonetic[idx], translate[idx]);
        idx++;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
*/