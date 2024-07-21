#include "9cc.h"

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

int label_count;
void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_RETURN: // return文の場合
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_NUM: // 数字だけの場合
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR: // ローカル変数の場合
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN: // 代入の場合
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_IF: // if文の場合
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%03d\n", node->label);
        gen(node->then);
        printf(".Lend%03d:\n", node->label);
        return;
    case ND_IF_ELSE: // if-else文の場合
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lelse%03d\n", node->label);
        gen(node->then);
        printf("    jmp .Lend%03d\n", node->label);
        printf(".Lelse%03d:\n", node->label);
        gen(node->els);
        printf(".Lend%03d:\n", node->label);
        return;
    case ND_WHILE: // while文の場合
        printf(".Lbegin%03d:\n", node->label);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%03d\n", node->label);
        gen(node->then);
        printf("    jmp .Lbegin%03d\n", node->label);
        printf(".Lend%03d:\n", node->label);
        return;
    case ND_FOR: // for文の場合
        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%03d:\n", node->label);
        if (node->cond)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%03d\n", node->label);
        }
        gen(node->then);
        if (node->inc)
        {
            gen(node->inc);
        }
        printf("    jmp .Lbegin%03d\n", node->label);
        printf(".Lend%03d:\n", node->label);
        return;
    case ND_BLOCK: // ブロックの場合
        for (Node *n = node->next; n; n = n->next)
        {
            gen(n);
            if (n->next)
            {
                printf("    pop rax\n");
            }
        }
        return;
    case ND_CALL_FUNC: // 関数呼び出しの場合
        for (Node *n = node->next; n; n = n->next)
        {
            gen(n);
        }
        // 引数の値をRDI、RSI、RDX、RCX、R8、R9のレジストリに引数をそれぞれ入れる 6以上ある場合は無視 6個に満たない場合のことも考慮
        int i = 0;
        for (Node *n = node->next; n; n = n->next)
        {
            if (i == 0)
            {
                printf("    pop rdi\n");
            }
            else if (i == 1)
            {
                printf("    pop rsi\n");
            }
            else if (i == 2)
            {
                printf("    pop rdx\n");
            }
            else if (i == 3)
            {
                printf("    pop rcx\n");
            }
            else if (i == 4)
            {
                printf("    pop r8\n");
            }
            else if (i == 5)
            {
                printf("    pop r9\n");
            }
            i++;
        }

         // rspを16の倍数にする
        printf("    mov rax, rsp\n");
        printf("    and rax, 15\n");
        printf("    jz .L.call.%d\n", label_count);
        printf("    push rax\n");
        printf(".L.call.%d:\n", label_count);
        
        printf("    call %s\n", node->name);
        printf("    push rax\n");

        // スタックポインタを元に戻す（必要な場合）
        printf("    mov rax, rsp\n");
        printf("    and rax, 15\n");
        printf("    jz .L.end.%d\n", label_count);
        printf("    add rsp, 8\n");
        printf(".L.end.%d:\n", label_count);

        label_count++;
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