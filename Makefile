all: piper testf tests

piper: piper.c
	gcc -Wall $< -o $@

testf: testf.c
	gcc -Wall $< -o $@

tests: tests.c
	gcc -Wall $< -o $@


clean:
	rm -f piper testf tests

.PHONY: all clean