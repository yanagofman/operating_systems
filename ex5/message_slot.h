

#ifndef MESSAGE_SLOT_H_
#define MESSAGE_SLOT_H_

#define BUF_LEN 128
#define HASHSIZE 131

#define SUCCESS 0
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot"

#define MAJOR_NUM 246


#define IOCTL_SET_CHA _IOW(MAJOR_NUM, 0, unsigned long)

typedef struct message_slot {
    char buff1[BUF_LEN];
    char buff2[BUF_LEN];
    char buff3[BUF_LEN];
    char buff4[BUF_LEN];
    int num;
} slot_struct;

struct list {
    struct list *next;
    char *name;
    slot_struct *device_struct;
};


#endif
