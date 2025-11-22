#include "sender.h"

void send(message_t message, mailbox_t* mailbox_ptr){
    if(mailbox_ptr->flag == 1) {
        if (mq_send(mailbox_ptr->storage.mq_descriptor, (char*)&message, sizeof(message_t), 0) == -1) {
            perror("mq_send");
            exit(1);
        }
    } else {
        char *dst = mailbox_ptr->storage.shm_addr;
        memcpy(dst, &message, sizeof(message_t));
    }
}

int main(int argc, char *argv[]) {
    // usage
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <1|2> <input.txt>\n", argv[0]);
        return 1;
    }
    
    // get the mechanism: 1 for mq, 2 for shm
    mailbox_t mailbox = {0};
    message_t message = {0};
    mailbox.flag = atoi(argv[1]);

    const char *input_file = argv[2];
    FILE *file_obj = fopen(input_file, "r"); // read only
    if(!file_obj) {
        perror("fopen");
        exit(1);
    }

    // setup mailbox
    if(mailbox.flag == 1) {
        // message queue
        printf("%sMessage Passing%s\n", BLUE, RESET);
        struct mq_attr attr = {
            .mq_flags = 0, // 0 for blocked
            .mq_msgsize = sizeof(message_t),
            .mq_maxmsg = 10
        };
        mailbox.storage.mq_descriptor = mq_open(MQ_NAME, O_CREAT|O_RDWR, 0666, &attr);
        if (mailbox.storage.mq_descriptor == (mqd_t)-1) {
            perror("mq_open");
            exit(1);
        }
    } else {
        // shared memory
        printf("%sShared Memory%s\n", BLUE, RESET);
        int shm_fd = shm_open(SHM_NAME, O_CREAT|O_RDWR, 0666);
        if(shm_fd == -1) {
            perror("shm_open");
            exit(1);
        }
        if(ftruncate(shm_fd, sizeof(message_t)) == -1) {
            perror("ftruncate");
            exit(1);
        }
        mailbox.storage.shm_addr = mmap(0, sizeof(message_t), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if(mailbox.storage.shm_addr == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        close(shm_fd);
    }

    sem_t* sender_sem = sem_open(SENDER_SEM_NAME, O_CREAT, 0666, 1);
    sem_t* receiver_sem = sem_open(RECEIVER_SEM_NAME, O_CREAT, 0666, 0);
    double total_time = 0;
    struct timespec start, end;

    int count = 1;
    while(1) {
        if(fgets(message.msgText, PAYLOAD_SIZE, file_obj)) {
            // if !EOF
            sem_wait(sender_sem); // sender.S-- to wait receiver next

            message.len = strnlen(message.msgText, PAYLOAD_SIZE-1);
            message.mType = MSG_DATA;

            clock_gettime(CLOCK_MONOTONIC, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC, &end);

            sem_post(receiver_sem);

            total_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
            //printf("%sSending message[%03d]:%s %s", BLUE, count++, RESET, message.msgText);
            printf("%sSending message:%s %s", BLUE, RESET, message.msgText);
        } else {
            // if EOF, send exit message
            sem_wait(sender_sem);

            snprintf(message.msgText, PAYLOAD_SIZE, "%s", EXIT_MSG);
            message.len = strnlen(message.msgText, PAYLOAD_SIZE-1);
            message.mType = MSG_EXIT;

            clock_gettime(CLOCK_MONOTONIC, &start);
            send(message, &mailbox);
            clock_gettime(CLOCK_MONOTONIC, &end);

            sem_post(receiver_sem);
            total_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
            break;
        }
    }
    printf("\n");
    printf("%sEnd of input file! exit!%s\n", RED, RESET);
    printf("Total time taken in sending msg: %f s\n", total_time);

    fclose(file_obj);

    if(mailbox.flag == 1) {
        // close message queue
        mq_close(mailbox.storage.mq_descriptor);
        mq_unlink(MQ_NAME);
    } else {
        // close shared memory
        munmap(mailbox.storage.shm_addr, sizeof(message_t));
        shm_unlink(SHM_NAME);
    }

    sem_close(sender_sem);
    sem_close(receiver_sem);
    sem_unlink(SENDER_SEM_NAME);
    sem_unlink(RECEIVER_SEM_NAME);

    return 0;
}
