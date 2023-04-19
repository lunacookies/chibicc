CC=clang

CFLAGS=\
	-std=c11 \
	-fsanitize=address \
	-fsanitize=undefined \
	-g \
	-W \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wimplicit-fallthrough \
	-Wimplicit-int-conversion \
	-Wshadow \
	-Wstrict-prototypes \
	-Wmissing-prototypes

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix out/, $(SOURCES:.c=.o))

all: out/chibicc tidy

out/chibicc: $(OBJECTS)
	@ mkdir -p out
	@ $(CC) $(CFLAGS) $^ -o $@

out/%.o: %.c $(HEADERS)
	@ mkdir -p out
	@ $(CC) $(CFLAGS) -c -o $@ $<

test: out/chibicc
	@ ./test.sh

tidy: $(HEADERS) $(SOURCES)
	@ clang-format -i $^

clean:
	@ rm -r out

.PHONY: all test tidy clean
