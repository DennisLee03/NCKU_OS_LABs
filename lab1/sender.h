#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <mqueue.h>
#include "constants.h"
#include <sys/mman.h>

#define MSG_PASSING 1
#define SHARED_MEM 2

typedef struct {
    int flag;      // 1 for message passing, 2 for shared memory
    union{
        mqd_t mq_descriptor;
        char* shm_addr;
    } storage;
} mailbox_t;


typedef struct {
    long mType;
    int len;
    char msgText[PAYLOAD_SIZE];
} message_t;

void send(message_t message, mailbox_t* mailbox_ptr);
