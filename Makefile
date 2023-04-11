all: piper tests

piper: piper.c
	gcc -Wall $< -o $@ -std=c99

tests: tests.c
	gcc -Wall $< -o $@ -std=c99

clean:
	rm -f piper tests

.PHONY: all clean