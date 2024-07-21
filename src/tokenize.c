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
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

// 次のトークンがidentの時はトークンを一つ進めてそのidentを返す。それ以外はfalseを返す
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }
    Token *ident = token;
    token = token->next;
    return ident;
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

// 変数名の始めの文字として有効な文字であるかを判定する 数字は不可
bool is_ident_start(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// 変数名の始め以外の文字として有効な文字であるかを判定する
bool is_ident_others(char c)
{
    return is_ident_start(c) || ('0' <= c && c <= '9');
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

        if (strchr("+-*/()><>=;", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // 変数を検出した場合
        if (is_ident_start(*p))
        {
            char *q = p++;
            while (is_ident_others(*p))
            {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
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