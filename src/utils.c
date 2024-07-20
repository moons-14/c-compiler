#include "9cc.h"

// エラーを報告するための関数
// printfと同じ引数を取る
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool strEqual(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}