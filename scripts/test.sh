#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./build/9cc "$input" > build/tmp.s
    cc -o build/tmp build/tmp.s
    ./build/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "✅ $input => $actual"
    else
        echo "❌ $input => $expected expected, but got $actual"
        exit 1
    fi
}

# assert 0 0
# assert 42 42

# 足し引きできるように
assert 21 "5+20-4;"

# トークン化
assert 41 " 12 + 34 - 5 ;"

# 四則演算
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

# 単項のプラマイを許可
assert 60 '12*+5;'
assert 75 '-5*-5*3;'

# 比較演算子を使えるように
assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

# アルファベット一文字変数を使えるように
assert 10 'a=2;b=8;a+b;'
assert 23 'a=2-1;b=12-(-2*-3);z=1>=0;t=23;a*(b-3)+20;'

# 複数文字の変数が使えるように
assert 40 'foo=20;bar=4-+2;foo*bar;'

# returnが使えるように
assert 24 'foo=20;bar=4-+2;return foo+bar*2;'
assert 5 'foo=20;bar=4-+2;return foo/4;return foo*bar*2;'

# if,elseが使えるように
assert 10 'if(1==1) return 10; return 20;'
assert 20 'if(1==0) return 10; return 20;'
assert 20 'if(1==0) return 10; else return 20;'
assert 30 'a=20;b=30;if(a>=b) return a; else return b;'

# whileが使えるように
assert 10 'a=0;while(a<10) a=a+1; return a;'
assert 10 'a=20;while(a>10) a=a-1; return a;'

# forが使えるように
assert 10 'a=0;for(i=0;i<10;i=i+1) a=a+1; return a;'
assert 10 'a=0;for(;a<10;) a=a+1; return a;'

# ブロックを使えるように
assert 10 '{10;}'
assert 20 '{a=20;return a;}'
assert 20 'if(1==1) {a=20;return a;} return 10;'

# 複雑な式
# 複雑な条件分岐とループの組み合わせ
assert 38 'a=0; b=1; while(a<6) { if(a<3) b=b*2; else b=b+10; a=a+1; } return b;'
# ネストされたif文と算術演算
assert 140 'x=10; y=20; if(x<y) { if(y>15) { x=x*2; y=y+100; } else { x=x+5; } } else { y=y-10; } return x+y;'
# 複数の変数を使った計算とループ
assert 55 'sum=0; i=1; while(i<=10) { sum=sum+i; i=i+1; } return sum;'
# 条件付きのreturn文を含むループ
assert 128 'a=1; b=1; while(a<100) { if(b>100) return a; a=a+b; b=b*2; } return b;'
# 複雑な算術演算と条件分岐の組み合わせ
assert 35 'x=5; y=3; z=2; if((x+y)*z > 15) { return x*y+z*10; } else { return (x-y)*z+100; }'

echo OK