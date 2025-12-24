#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*Note: Value of LOCK is 0 and value of UNLOCK is 1.*/
#define LOCK 0
#define UNLOCK 1

volatile int a = 0;
volatile int lock = UNLOCK;
pthread_mutex_t mutex;

void spin_lock() {
    asm volatile(
        "loop:\n\t"
        "mov $0, %%eax\n\t"         // move 0 to eax -> get locked initially
        /*YOUR CODE HERE*/
        "xchg %%eax, %[lock]\n\t"   // exchange lock and eax, get locked or unlocked
        "cmpl $1, %%eax\n\t"        // old_lock_value == unlocked ? 
        /****************/
        "jne loop\n\t"              // check flag bit
        :                           // output operands
        : [lock] "m" (lock)         // input operands
        : "eax", "memory"           // clobbers
    );
}

void spin_unlock() {
    asm volatile(
        "mov $1, %%eax\n\t"
        /*YOUR CODE HERE*/
        "xchg %%eax, %[lock]\n\t"   // unclocked the lock
        /****************/
        :                           // output operands
        : [lock] "m" (lock)         // input operands
        : "eax", "memory"           // clobbers
    );
}


void *thread(void *arg) {

    for(int i=0; i<10000; i++){

        spin_lock();
        a = a + 1;
        spin_unlock();
    }
    return NULL;
}

int main() {
    FILE *fptr;
    fptr = fopen("1.txt", "a");
    pthread_t t1, t2;

    pthread_mutex_init(&mutex, 0);
    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&mutex);

    fprintf(fptr, "%d ", a);
    fclose(fptr);
}

