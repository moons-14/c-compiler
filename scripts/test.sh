#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./build/9cc "$input" > build/tmp.s
    cc -o build/tmp build/tmp.s
    ./build/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42

# 足し引きできるように
assert 21 "5+20-4"

# トークン化
assert 41 " 12 + 34 - 5 "

# 四則演算
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4 '(3+5)/2'

# 単項のプラマイを許可
assert 60 '12*+5'
assert 75 '-5*-5*3'

# 比較演算子を使えるように
assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'

assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'

assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'

echo OK