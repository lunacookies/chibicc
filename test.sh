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

echo OK
