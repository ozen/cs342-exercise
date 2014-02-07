CS 342 - Operating Systems 2013-2014 Fall Semester

## wordcount.c

This is an exercise of multi thread programming in C.

#### Question:

Write a multi-threaded C program that will count the number of occurrences of words in 
an input set of text files. There will be M input text files (indexed as 0..M-1) containing 
words of alphanumerical characters (M can be at most 100). The names of the files are 
taken from the command line. Each file will be processed by a parser-thread that will 
read and parse the file content and partition and emit the words in it into R intermediate 
temporary files (R can be at most 50). The partition rule is the following: a word w 
coming from an input file j (0<=j<=M-1) will go to an intermediate file “tempj-i” where i 
= hash(w) mod R. Here 0<=i<=R-1. The hash(w) will be found by summing the bytes of 
w. If w is “the”, for example, the three characters, casted to integer, will be summed. In 
this way the words in an input file are partitioned into R files (at most). As a result, the 
same word may appear many times in an intermediate file. Each word appears in a 
separate line of an intermediate file. Since each of M parser threads can produce at most 
R files, there can be at most M*R intermediate files produced. 
There will be R counter-threads. An counter thread will read M partitions (M 
intermediate files), sort the content read according to strcmp(word), find the count of 
occurances of each unique word, and will emit to a temporary output file the unique 
words and their counts. For example, counter thread 0 will read intermediate files 
(partitions) temp0-0, temp1-0, temp2-0, …, temp(M-1)-0. The counter thread 3, for 
example, will read the files temp0-3, temp1-3, …, temp(M-1)-3. A counter thread may 
emit <word, count> pairs as follows (one pair per line). 

an 10 
school 15 
university 8 
zero 12 

At the end, R temporary output files will be produced. A merger thread will merge them 
and will produce a single final file that contains all unique words and their counts in 
sorted order (according to words – use strcmp for comparing two words). The name of 
the final file will be taken from the command line as the last argument of the program. 
Name your program as wordcount. The parameters are: 
wordcount <M> <R> <infile1> … <infileM> <finalfile>

An example invocation is: 

wordcount 3 2 infile1.txt infile2.txt infile3.txt final.txt


## pc.c

This is an example of mutex lock solution to producer-consumer synchronization problem.

#### Question:

Implement the following producer-consumer program. There are two producer threads 
and one consumer thread. There is a single shared linked list (shared buffer) between 
producers and consumer that can hold at most 100 integers. There are two input files, 
each containing sorted positive integers. Each input file will be processed by a separate 
producer. Each producer will read its file and pass the integers to the consumer through 
the shared buffer. It will do this while reading: read one integer, try to put one integer. An 
integer will be put into the list in a structure having two fields. One field will keep the 
integer value, the other field will keep the ID (0 or 1) of the producer putting the integer. 
In this way consumer can understand from which producer the integer is coming. The 
consumer will read the integers from the shared buffer and will merge them and write 
them in sorted order to an output file. At the end, output file is in sorted order. Program is 
called pc and has the following parameters: pc <infile1> <infile2> <outfile>.

An example invocation is: 

pc in1.txt in2.txt out.txt

Use POSIX semaphores for synchronization. 
