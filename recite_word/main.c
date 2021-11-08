#include "recite.h"

int main(int argc, const char* argv[])
{
    char* word[MAX_WORD]      = {};
    char* phonetic[MAX_WORD]  = {};
    char* translate[MAX_WORD] = {};
    char input[MAX]           = "";
    int num                   = 0;
    for(int id=0; id<MAX_WORD; id++)
    {
        word[id]      = (char*)malloc(20);
        phonetic[id]  = (char*)malloc(30);
        translate[id] = (char*)malloc(MAX);
    }
    // 将数据库全部保存
    down_load(word, phonetic, translate);
    printf("==========TSET START======>>>>>>\n");
    while(1)
    {
        // 获取随机种子
        srand(time(NULL));
        num = rand() % MAX_WORD;
        switch (num % 2)
        {
        case 1:
            // 中译英
            if(-1 == ch_to_en(num, word, phonetic, translate)) {goto END;}
            break;
        case 0:
            // 英译中
            if(-1 == en_to_ch(num, word, phonetic, translate)) {goto END;}
            break;
        default:
            break;
        }
    }
END:
    // 销毁malloc出来的空间
    delte_all(word, phonetic, translate);
    return 0;
}