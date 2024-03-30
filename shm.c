#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include "kfifo.h"

#define BUFFER_SIZE 10
#define SHM_KEY 0x1050

typedef struct kfifo FIFO_T;

typedef struct {
    int sport;
    int midx;
    int dport;
    unsigned char buff[1024];
    int len;
} FIFO_ENT_T;

KFIFO_ARRAY(kfifo, FIFO_ENT_T);

FIFO_T* s_fifo;

int main() {
    int shm_id;
    int size = 0x10000;
    printf("mem size:%d, sizeof(FIFO_ENT_T):%ld\n", size, sizeof(FIFO_ENT_T));

    shm_id = shmget(SHM_KEY, size, IPC_CREAT | 0666);
    if(shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory segment to process address space
    s_fifo = shmat(shm_id, NULL, 0);
    if(s_fifo == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    KFIFO_ARRAY_INIT(s_fifo, 8);

    unsigned int count = 1;
    while(1) {
        if(KFIFO_FULL(s_fifo)) {
            sleep(1);
            continue;
        }
        FIFO_ENT_T ent = {
            .sport = 1,
            .midx = count++,
            .dport = 5,
            .len = 111,
        };
        sprintf(ent.buff, "%s%.8x", "abcdeeeeaabbzz", count);

        printf("s_fifo->tail:%x, s_fifo->head:%x, "
               "s_fifo->array[(s_fifo)->tail]:0x%p\n",
        s_fifo->tail, s_fifo->head, &s_fifo->array[(s_fifo)->tail]);

        KFIFO_ENQUEUE(s_fifo, ent);
    }
}