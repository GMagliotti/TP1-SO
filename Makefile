all: piper tests

piper: piper.c
	gcc -Wall $< -o $@ -std=c99 -lm -pthread -g

tests: tests.c
	gcc -Wall $< -o $@ -std=c99 -lm -pthread -g

clean:
	rm -f piper tests

.PHONY: all clean