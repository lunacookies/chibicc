#!/bin/sh

assert() {
	expected="$1"
	input="$2"

	./out/chibicc "$input" > out/tmp.s || exit
	as -arch x86_64 -o out/tmp.o out/tmp.s
	ld \
		-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
		-lSystem \
		-o out/tmp \
		out/tmp.o

	./out/tmp
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
assert 21 '5+20-4'
assert 41 ' 12 + 34 - 5 '
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 4  '(3+5)/2'
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'

echo OK
