CFLAGS=-std=c11 -g -static

9cc: ./src/9cc.c
		$(CC) -o ./build/9cc ./src/9cc.c $(LDFLAGS)

test: ./src/9cc.c
		./scripts/test.sh

clean:
		rm -f ./build/9cc ./build/*.o ./build/*~ ./build/tmp*

.PHONY: test clean