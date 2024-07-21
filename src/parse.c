#include "9cc.h"

int nodeLabelCount = 0;
Node *new_node_by_kind(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->label = nodeLabelCount++;
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node_by_kind(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = new_node_by_kind(ND_NUM);
    node->val = val;
    return node;
}

Node *new_node_if(Node *cond, Node *then)
{
    Node *node = new_node_by_kind(ND_IF);
    node->cond = cond;
    node->then = then;
    return node;
}

Node *new_node_if_else(Node *cond, Node *then, Node *els)
{
    Node *node = new_node_by_kind(ND_IF_ELSE);
    node->cond = cond;
    node->then = then;
    node->els = els;
    return node;
}

// ローカル変数
LVar *locals;
void init_lvar()
{
    locals = NULL;
}

// 変数を名前で検索する。見つからない場合はNULL
LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

/*
生成規則
program = stmt*
stmt = expr ";"
        | "return" expr ";"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "while" "(" expr ")" stmt
expr = assign
assign = equality ("=" assign)?
equality = relational ("==" relational | "!=" relational)*
relational = add ("<" add | ">" add | "<=" add | ">=" add)*
add = num ("+" mul | "-" mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-")? primary
primary = num | ident | "(" expr ")"
*/

Node *code[100];

// program = stmt*
void *program()
{
    int i = 0;
    while (!at_eof())
    {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

// stmt = expr ";"
//         | "return" expr ";"
//         | "if" "(" expr ")" stmt ("else" stmt)?
//         | "while" "(" expr ")" stmt
Node *stmt()
{
    Node *node;
    if (consume_token_kind(TK_RETURN))
    {
        node = new_node_by_kind(ND_RETURN);
        node->lhs = expr();
    }
    else if (consume_token_kind(TK_IF))
    {
        expect("(");
        Node *cond = expr();
        expect(")");
        Node *then = stmt();
        Node *els = NULL;
        if (consume_token_kind(TK_ELSE))
        {
            node = new_node_if_else(cond, then, stmt());
        }
        else
        {
            node = new_node_if(cond, then);
        }
        return node;
    }
    else if(consume_token_kind(TK_WHILE))
    {
        expect("(");
        Node *cond = expr();
        expect(")");
        Node *body = stmt();
        node = new_node_by_kind(ND_WHILE);
        node->cond = cond;
        node->then = body;
        return node;
    }
    else
    {
        node = expr();
    }

    if (!consume(";"))
        error("';'ではないトークンです");
    return node;
}

// expr = assign
Node *expr()
{
    return assign();
}

// assign = equality ("=" assign)?
Node *assign()
{
    Node *node = equality();
    for (;;)
    {
        if (consume("="))
            node = new_node(ND_ASSIGN, node, assign());
        else
            return node;
    }
}

// equality = relational ("==" relational | "!=" relational)*
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

// relational = add ("<" add | ">" add | "<=" add | ">=" add)*
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

// add = num ("+" mul | "-" mul)*
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

// mul = unary ("*" unary | "/" unary)*
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

// unary = ("+" | "-")? primary
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

// primary = num | ident | "(" expr ")"
Node *primary()
{
    // 次のトークンが"("なら、"("+expr+")"のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok)
    {
        Node *node = new_node_by_kind(ND_LVAR);

        LVar *lvar = find_lvar(tok);

        if (lvar)
        {
            node->offset = lvar->offset;
        }
        else
        {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            if (locals)
                lvar->offset = locals->offset + 8;
            else
                lvar->offset = 8;
            node->offset = lvar->offset;
            locals = lvar;
        }

        return node;
    }

    // それ以外は数字のはず
    return new_node_num(expect_number());
}