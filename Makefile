all: wordcount pc
wordcount: wordcount.c
	gcc wordcount.c -o wordcount -lpthread
pc: pc.c
	gcc pc.c -o pc -lpthread
