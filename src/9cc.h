#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === Token ===

// トークンの種類
typedef enum
{
    TK_RESERVED,    // 記号
    TK_IDENT,       // 識別子
    TK_NUM,         // 数字
    TK_EOF,         // 入力の終わり
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

extern Token *token;

// === ノード ===
// ASTのノードの種類
typedef enum
{
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_ASSIGN,  // =
    ND_SETE,    // ==
    ND_SETNE,   // !=
    ND_SETLE,   // <=
    ND_SETL,    // <
    ND_NUM,     // 整数
    ND_LVAR,    // ローカル変数
} NodeKind;

typedef struct Node Node;

// ASTのノードの型
struct Node
{
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺 (left-hand side)
    Node *rhs;      // 右辺 (right-hand side)
    int val;        // kindがND_NUMの場合のみ、その数字
    int offset;     // kindがND_LVARの場合のみ、RBPとの差異
};

// コード
extern Node *code[100];

// === utils.c ===
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool strEqual(char *p, char *q);
bool at_eof();

// === main.c ===
int main(int argc, char **argv);

extern char *user_input;

// === codegen.c ===
void gen(Node *node);

// === tokenize.c ===
Token *new_token(TokenKind kind, Token *cur, char *str, int stl);
bool consume(char *op);
bool expect(char *op);
Token *consume_ident();
int expect_number();
Token *tokenize(char *p);

// === parse.c ===
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();