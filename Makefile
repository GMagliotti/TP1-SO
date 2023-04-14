all: piper tests

piper: piper.c
	gcc -Wall $< -o $@ -std=c99 -lm

tests: tests.c
	gcc -Wall $< -o $@ -std=c99 -lm

clean:
	rm -f piper tests

.PHONY: all clean