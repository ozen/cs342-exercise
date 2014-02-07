#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>

#define MAXM 100
#define MAXR 50
#define MAX_WORD_SIZE 100
#define INIT_COUNT_SIZE 10
#define INIT_MERGE_SIZE 100

int M, R;
char** input_files;
char* final_file;

typedef struct {
    char word[MAX_WORD_SIZE];
    int count;
} Pair;

int hash(char word[]) {
    int hash = 0;
    int i;
    for(i=0; word[i]; i++) {
        hash += (int) word[i];
    }
    return hash;
}

int pair_compare(const void *a, const void *b) {
    return strcmp((*(Pair*)a).word, (*(Pair*)b).word);
}


void parse(int id) {
    printf("Parsing %d\n", id+1);

    int i;
    FILE *in = fopen(input_files[id], "r");
    if(!in) {
        printf("Cannot open file: %s, errno:%d\n", input_files[id], errno);
        return;
    }

    char word[MAX_WORD_SIZE];

    FILE* out[R];
    for(i=0; i<R; i++) {
        char name[12];
        sprintf(name, "temp%d-%d", id, i);
        out[i] = fopen(name, "w");
    }

    while(fscanf(in, "%s", word) == 1) {
        fprintf(out[hash(word) % R], "%s\n", word);
    }

    for(i=0; i<R; i++) {
        fclose(out[i]);
    }

    fclose(in);
}

void count(int id) {
    printf("Counting %d\n", id+1);

    int i, j, count = 0, size = INIT_COUNT_SIZE;
    Pair* pairs = (Pair*) malloc(size * sizeof(Pair));

    for(i=0; i<M; i++) {
        char name[12];
        sprintf(name, "temp%d-%d", i, id);
        FILE *in = fopen(name, "r");
        if(!in) {
            printf("Cannot open file: %s, errno:%d\n", name, errno);
            return;
        }

        char word[MAX_WORD_SIZE];

        while(fscanf(in, "%s", word) == 1) {
            int found = 0;
            for(j=0; j<count; j++) {
                if(strcmp(pairs[j].word, word) == 0) {
                    pairs[j].count = pairs[j].count + 1;
                    found = 1;
                    break;
                }
            }

            if(!found) {
                if(count >= size) {
                    pairs = (Pair*) realloc(pairs, 2 * size * sizeof(Pair));
                    size = 2 * size;
                }

                strcpy(pairs[count].word, word);
                pairs[count].count = 1;
                count++;
            }
        }

        fclose(in);
    }

    qsort(pairs, count, sizeof(Pair), pair_compare);

    char name[6];
    sprintf(name, "out%d", id);
    FILE *out = fopen(name, "w");
    if(!out) {
        printf("Cannot open file: %s, errno:%d\n", name, errno);
        return;
    }

    for(i=0; i<count; i++) {
        fprintf(out, "%s\t\t%d\n", pairs[i].word, pairs[i].count);
    }

    free(pairs);
    fclose(out);
}

void merge() {
    printf("Merging\n");
    int i, count = 0, size = INIT_MERGE_SIZE;
    Pair* pairs = (Pair*) malloc(size * sizeof(Pair));

    for(i=0; i<R; i++) {
        char name[6];
        sprintf(name, "out%d", i);
        FILE *in = fopen(name, "r");
        if(!in) {
            printf("Cannot open file: %s, errno:%d\n", name, errno);
            return;
        }

        char word[MAX_WORD_SIZE];
        int num;

        while(fscanf(in, "%s%d", word, &num) == 2) {
            if(count >= size) {
                pairs = (Pair*) realloc(pairs, 2 * size * sizeof(Pair));
                size = 2 * size;
            }

            strcpy(pairs[count].word, word);
            pairs[count].count = num;
            count++;
        }

        fclose(in);
    }

    qsort(pairs, count, sizeof(Pair), pair_compare);

    FILE *out = fopen(final_file, "w");
    if(!out) {
        printf("Cannot open file: %s, errno:%d\n", final_file, errno);
        return;
    }

    for(i=0; i<count; i++) {
        fprintf(out, "%s\t\t%d\n", pairs[i].word, pairs[i].count);
    }

    free(pairs);
    fclose(out);
}

int main(int argc, char** argv) {
    int i, err;
    system("rm temp* out*");

    /*
    *   Evaluate command line parameters
    */

    if(argc < 5) {
        printf("Usage: wordcount <M> <R> <infile1> ... <infileM> <finalfile>\n");
        return 0;
    }

    M = atoi(argv[1]);
    R = atoi(argv[2]);

    if(argc != 4+M) {
        printf("Number of parameters does not match with M.\nUsage: wordcount <M> <R> <infile1> ... <infileM> <finalfile>\n");
        return 0;
    }

    printf("M:%d R:%d\n", M, R);

    input_files = (char**) malloc(M * sizeof(char*));

    for(i=0; i<M; i++) {
        input_files[i] = argv[i+3];
    }

    final_file = argv[argc-1];

    /*
    *   Create Parser Threads
    */

    pthread_t parser_threads[M];

    for(i=0; i<M; i++) {
        pthread_create(&parser_threads[i], NULL, (void*)&parse, (void*) i);
    }

    for(i=0; i<M; i++) {
        err = pthread_join(parser_threads[i], NULL);
        if (err)
            printf("error joining thread %d, error=%d\n", i, err);
    }

    /*
    *   Create Counter Threads
    */

    pthread_t counter_threads[R];

    for(i=0; i<R; i++) {
        pthread_create(&counter_threads[i], NULL, (void*)&count, (void*) i);
    }

    for(i=0; i<R; i++) {
        err = pthread_join(counter_threads[i], NULL);
        if (err)
            printf("error joining thread %d, error=%d\n", i, err);
    }

    /*
    *   Create Merger Thread
    */

    pthread_t merger_thread;

    pthread_create(&merger_thread, NULL, (void*)&merge, NULL);

    err = pthread_join(merger_thread, NULL);
    if (err)
        printf("error joining merger thread, error=%d\n", err);

    printf("Word counting has been completed.\n");
    free(input_files);
    return 0;
}
