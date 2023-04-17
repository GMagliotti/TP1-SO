all: md5 hashCalculate vista

md5: md5.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g -D_XOPEN_SOURCE=500

hashCalculate: hashCalculate.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g

vista: vista.c
	gcc -Wall $< -o $@ -std=c99 -lm -lrt -pthread -g

clean:
	rm -f md5 hashCalculate vista

.PHONY: all clean