#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>

#define LISTMAX 100
#define END_OF_STREAM -1

typedef struct Element {
    int value;
    int producer;
    struct Element* next;
} Element;

typedef struct {
    int count;
    Element* head;
    Element* tail;
} List;

List buffer;
sem_t mutex, empty, full;
char* infile[2];

void list_init(List* li) {
    li->count = 0;
    li->head = NULL;
    li->tail = NULL;
}

void list_insert(List* li, Element* el) {
    if(li->count == 0) {
        li->head = el;
        li->tail = el;
    }
    else {
        li->tail->next = el;
        li->tail = el;
    }

    li->count++;
}

Element* list_retrieve(List* li) {
    if(li->count == 0) {
        return NULL;
    }

    Element* el = li->head;
    li->head = li->head->next;
    li->count--;

    return el;
}

void list_dump(List* li) {
    if(!li->head) return;
    Element* el = li->head;
    do {
        printf("%d ", el->value);
    } while(el = el->next);
    printf("\n\n");
}

void produce(int id) {

    FILE *in = fopen(infile[id], "r");
    if(!in) return;

    int num;
    while(fscanf(in, "%d", &num) == 1) {
        //list_dump(&buffer);

        Element* ele = (Element*) malloc(sizeof(Element));
        ele->value = num;
        ele->producer = id;
        ele->next = NULL;

        sem_wait(&empty);
        sem_wait(&mutex);
        list_insert(&buffer, ele);
        sem_post(&mutex);
        sem_post(&full);
    }

    Element* ele = (Element*) malloc(sizeof(Element));
    ele->value = END_OF_STREAM;
    ele->producer = id;
    ele->next = NULL;

    sem_wait(&empty);
    sem_wait(&mutex);
    list_insert(&buffer, ele);
    sem_post(&mutex);
    sem_post(&full);

    fclose(in);
}

void consume(char* outfile) {
    int open = 2;
    List list[2];

    list_init(&list[0]);
    list_init(&list[1]);

    while(open) {

            sem_wait(&full);
            sem_wait(&mutex);
            Element* ele = list_retrieve(&buffer);
            sem_post(&mutex);
            sem_post(&empty);

            if(ele->value > 0) {
                list_insert(&list[ele->producer], ele);
            }
            else {
                --open;
            }
    }

    FILE *out = fopen(outfile, "a");
    if(!out) return;
    Element* ele;

    while(list[0].count > 0 && list[1].count > 0) {
        if(list[0].head->value < list[1].head->value) {
            ele = list_retrieve(&list[0]);
        }
        else {
            ele = list_retrieve(&list[1]);
        }

        fprintf(out, "%d\n", ele->value);
        free(ele);
    }

    while(list[0].count > 0) {
        ele = list_retrieve(&list[0]);
        fprintf(out, "%d\n", ele->value);
        free(ele);
    }

    while(list[1].count > 0) {
        ele = list_retrieve(&list[1]);
        fprintf(out, "%d\n", ele->value);
        free(ele);
    }

    fclose(out);
}

int main(int argc, char** argv) {
    if(argc != 4) {
        printf("Usage: pc <infile1> <infile2> <outfile>\n");
        return 0;
    }

    infile[0] = argv[1];
    infile[1] = argv[2];
    char* outfile = argv[3];

    remove(outfile);

    list_init(&buffer);

    pthread_t producer1, producer2, consumer;

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, LISTMAX);
    sem_init(&full, 0, 0);

    pthread_create(&producer1, NULL, (void*)&produce, (void*) 0);
    pthread_create(&producer2, NULL, (void*)&produce, (void*) 1);
    pthread_create(&consumer, NULL, (void*)&consume, (void*) outfile);

    pthread_join(producer1, NULL);
    pthread_join(producer2, NULL);
    pthread_join(consumer, NULL);

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
