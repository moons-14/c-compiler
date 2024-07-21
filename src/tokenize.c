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

// トークンを進める
void next_token()
{
    token = token->next;
}

// 次のトークンが期待している記号の場合のとき、注目しているトークンを一つ進めてtrue。それ以外はfalse。
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, strlen(op)))
    {
        return false;
    }
    next_token();
    return true;
}

// 次のトークンが期待している記号のときはトークンを一つ進める。それ以外はエラーを出す
bool expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, strlen(op)))
    {
        error_at(token->str, "'%s'ではありません", op);
    }
    next_token();
}

// 次のトークンがidentの時はトークンを一つ進めてそのidentを返す。それ以外はfalseを返す
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }
    Token *ident = token;
    next_token();
    return ident;
}

// 次のトークンが特定の種類かどうか 特定の種類ならトークンを一つすすめてtrue それ以外はfalseを返す
bool consume_token_kind(TokenKind kind)
{
    if (token->kind != kind)
    {
        return false;
    }
    next_token();
    return true;
}

// 次のトークンが数字の場合、トークンを一つ進めてその数字を返す。それ以外はエラーを出す
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    next_token();
    return val;
}

// 変数名の始めの文字として有効な文字であるかを判定する 数字は不可
bool is_ident_start(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// 変数名の始め以外の文字として有効な文字であるかを判定する
bool is_alnum(char c)
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

        // returnの場合
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        // ifの場合
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        // elseの場合
        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        // whileの場合
        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        // 変数を検出した場合
        if (is_ident_start(*p))
        {
            char *q = p++;
            while (is_alnum(*p))
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