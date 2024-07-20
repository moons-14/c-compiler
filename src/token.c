#include "9cc.h"

// 新しいトークンを作成してcurのnextに入れる
Token *new_token(TokenKind kind, Token *cur, char *str, int stl)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = stl;
    cur->next = tok;
    return tok;
}

// 次のトークンが期待している記号の場合のとき、注目しているトークンを一つ進めてtrue。それ以外はfalse。
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, strlen(op)))
    {
        return false;
    }
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときはトークンを一つ進める。それ以外はエラーを出す
bool expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, strlen(op)))
    {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが数字の場合、トークンを一つ進めてその数字を返す。それ以外はエラーを出す
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

// 文字列*pをトークナイズして返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // スペースはスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (strEqual(p, "==") || strEqual(p, "!=") || strEqual(p, "<=") || strEqual(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()><>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}