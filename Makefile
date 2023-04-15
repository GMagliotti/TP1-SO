all: piper tests view

view: view.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g

piper: piper.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g -D_XOPEN_SOURCE=500

tests: tests.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g

clean:
	rm -f piper tests view

.PHONY: all clean