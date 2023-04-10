all: piper tests

piper: piper.c
	gcc -Wall $< -o $@

tests: tests.c
	gcc -Wall $< -o $@


clean:
	rm -f piper tests

.PHONY: all clean