#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum
{
    TK_RESERVED, // 記号
    TK_NUM,      // 数字
    TK_EOF,      // 入力の終わり
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // トークンの種類
    Token *next;    // 次のトークン
    int val;        // kindがTK_NUMの場合のみ、その数字
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

// 注目しているトークン
Token *token;

// ASTのノードの種類
typedef enum
{
    ND_ADD,   // +
    ND_SUB,   // -
    ND_MUL,   // *
    ND_DIV,   // /
    ND_SETE,  // ==
    ND_SETNE, // !=
    ND_SETLE, // <=
    ND_SETL,  // <
    ND_NUM,   // 整数
} NodeKind;

typedef struct Node Node;

// ASTのノードの型
struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺 (left-hand side)
    Node *rhs;     // 右辺 (right-hand side)
    int val;       // kindがND_NUMの場合のみ、その数字
};

// 入力プログラム
char *user_input;

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

bool at_eof()
{
    return token->kind == TK_EOF;
}

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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

/*
生成規則
expr = equality
equality = relational ("==" relational | "!=" relational)*
relational = add ("<" add | ">" add | "<=" add | ">=" add)*
add = num ("+" mul | "-" mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-")? primary
primary = num | "(" expr ")"
*/

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr()
{
    return equality();
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_SETE, node, relational());
        else if (consume("!="))
            node = new_node(ND_SETNE, node, relational());
        else
            return node;
    }
}

Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
            node = new_node(ND_SETL, node, add());
        else if (consume(">"))
            node = new_node(ND_SETL, add(), node);
        else if (consume("<="))
            node = new_node(ND_SETLE, node, add());
        else if (consume(">="))
            node = new_node(ND_SETLE, add(), node);
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    if (consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }

    return primary();
}

Node *primary()
{
    // 次のトークンが"("なら、"("+expr+")"のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    // それ以外は数字のはず
    return new_node_num(expect_number());
}



void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_SETE:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_SETNE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_SETLE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_SETL:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

bool strEqual(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // 入力プログラムを記憶する
    user_input = argv[1];

    // トークナイズする
    token = tokenize(argv[1]);

    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}