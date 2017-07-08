

#ifndef MESSAGE_SLOT_H_
#define MESSAGE_SLOT_H_

#define BUF_LEN 128
#define HASHSIZE 131

#define SUCCESS 0
#define NULL 0
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot"///the same??

typedef struct message_slot {
    char buff1[BUF_LEN];
    char buff2[BUF_LEN];
    char buff3[BUF_LEN];
    char buff4[BUF_LEN];
    int num;
} slot_struct;

struct list { /* table entry: */
    struct list *next; /* next entry in chain */
    char *name; /* defined name */
    slot_struct *device_struct;
};


#endif
