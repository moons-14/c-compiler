CFLAGS=-std=c11 -g -static
SRCS=$(wildcard ./src/*.c)
OBJS=$(patsubst ./src/%.c,./build/%.o,$(SRCS))

9cc: $(OBJS)
		$(CC) -o ./build/9cc $(OBJS) $(LDFLAGS)

$(OBJS): ./src/9cc.h

./build/%.o: ./src/%.c
		$(CC) $(CFLAGS) -c $< -o $@

test: ./build/9cc
		./scripts/test.sh

clean:
		rm -f ./build/9cc ./build/*.o ./build/*~ ./build/tmp*

.PHONY: test clean