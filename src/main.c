#include "9cc.h"

// 入力プログラム
char *user_input;
// 注目しているトークン
Token *token;

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

    // ローカル変数を初期化する
    init_lvar();
    // パースする。結果はcodeに入る
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // 先頭の行からコードを生成
    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);

        // 式の結果としてスタックに値が残っているのでポップする
        printf("    pop rax\n");
    }

    // エピローグ
    // 最後の式の結果RAXに残っているのでそれを返り値とする
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}