/* Declare what kind of code we want from the header files
   Defining __KERNEL__ and MODULE allows us to access kernel-level
   code not usually available to userspace programs. */
#undef __KERNEL__
#define __KERNEL__ /* We're part of the kernel */
#undef MODULE
#define MODULE     /* Not a permanent part, though. */

/* ***** Example w/ minimal error handling - for ease of reading ***** */

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <asm/uaccess.h>    /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include "message_slot.h"
#include <linux/slab.h>

MODULE_LICENSE("GPL");


struct chardev_info{
    spinlock_t lock;
};


static struct chardev_info device_info;



//CREDIT : https://stackoverflow.com/questions/4384359/quick-way-to-implement-dictionary-in-c

static struct list *hashtable[HASHSIZE];


unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}


struct list *lookup(char *s)
{
    struct list *node;
    for (node = hashtable[hash(s)]; node != NULL; node = node->next){
        if (strcmp(s, node->name) == 0){
          return node;
        }
    }
    return NULL;
}


struct list *insert(char *name, slot_struct *device_struct)
{
    struct list *np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) {
        np = (struct list *) kmalloc(sizeof(*np), GFP_KERNEL);
        if (np == NULL)
          return NULL;
        np->name = name;
        hashval = hash(name);
        np->next = hashtable[hashval];
        hashtable[hashval] = np;
    } else
        kfree((void *) np->device_struct);
    np->device_struct = device_struct;
    return np;
}



void delete_hash(void){
	int i;
	struct list *curr ;
	for(i=0 ;i < HASHSIZE; i++){
		curr = hashtable[i];
		while(curr != NULL){
			kfree((void *) curr->device_struct);
			kfree((void *) curr->name);
			hashtable[i] = curr->next;
			kfree((void *) curr);
			curr = hashtable[i];
		}
	}
	kfree((void *)hashtable);

}



/***************** char device functions *********************/



static int device_release(struct inode *inode, struct file *file)
{
    printk("device_release(%p,%p)\n", inode, file);

    return SUCCESS;
}

/* process attempts to open the device file */
static int device_open(struct inode *inode, struct file *file)
{
	char file_name[100];
    unsigned long flags;
    slot_struct *new_slot;
    printk("device_open(%p)\n", file);


    spin_lock_irqsave(&device_info.lock, flags);
    sprintf(file_name, "%lu", file->f_inode->i_ino);
    if (lookup(file_name)!= NULL){
    	printk("this device: %s is already associated with a data struct\n", file_name);
        spin_unlock_irqrestore(&device_info.lock, flags);
        return SUCCESS;
    }
    new_slot = (slot_struct *) kmalloc(sizeof(slot_struct), GFP_KERNEL);
    insert(file_name, new_slot);
    spin_unlock_irqrestore(&device_info.lock, flags);

    return SUCCESS;
}

static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset)
{
		int i;
		char file_name[100];
	    struct list *tmp;
	    sprintf(file_name, "%lu", file->f_inode->i_ino);
	    tmp = lookup(file_name);
	    if (tmp == NULL) {
	        printk("no such device(%p)\n", file);
	        return -1;
	    }
	    if(tmp->device_struct->num == 0){
	    	for(i = 0; i<length && i < BUF_LEN;i++){
	    		put_user(tmp->device_struct->buff1[i], buffer + i);
	    	}
	    }
    if(tmp->device_struct->num == 1){
    	for(i = 0; i<length && i < BUF_LEN;i++){
    		put_user(tmp->device_struct->buff2[i], buffer + i);
    	}
    }
    if(tmp->device_struct->num == 2){
    	for(i = 0; i<length && i < BUF_LEN;i++){
    		put_user(tmp->device_struct->buff3[i], buffer + i);
    	}
    }
    if(tmp->device_struct->num == 3){
    	for(i = 0; i<length && i < BUF_LEN;i++){
    		put_user(tmp->device_struct->buff4[i], buffer + i);
    	}
    }

    return SUCCESS;
}

static ssize_t device_write(struct file *file,
         const char __user * buffer, size_t length, loff_t * offset)
{
    int i;
    int ans;
	char file_name[100];
	struct list *tmp;
	sprintf(file_name, "%lu", file->f_inode->i_ino);
	tmp = lookup(file_name);
	if (tmp == NULL) {
	     printk("no such device(%p)\n", file);
	     return 1;
	}

	if(tmp->device_struct->num == 0){
	    for (i = 0; i < length && i < BUF_LEN; i++){
	        get_user(tmp->device_struct->buff1[i], buffer + i);
	    }
	    ans = i;
	    while(i<BUF_LEN){
	    	tmp->device_struct->buff1[i] = '\0';
	    	i++;
	    }
	}
	if(tmp->device_struct->num == 1){
		for (i = 0; i < length && i < BUF_LEN; i++){
			get_user(tmp->device_struct->buff2[i], buffer + i);
	    }
		ans = i;
		while(i<BUF_LEN){
			    	tmp->device_struct->buff2[i] = '\0';
			    	i++;
			    }

	}
	if(tmp->device_struct->num == 2){
		for (i = 0; i < length && i < BUF_LEN; i++){
			get_user(tmp->device_struct->buff3[i], buffer + i);
		}
		ans = i;
		while(i<BUF_LEN){
			    	tmp->device_struct->buff3[i] = '\0';
			    	i++;
			    }
	}
	if(tmp->device_struct->num == 3){
		for (i = 0; i < length && i < BUF_LEN; i++){
		    get_user(tmp->device_struct->buff4[i], buffer + i);
		}
		ans = i;
		while(i<BUF_LEN){
			    	tmp->device_struct->buff4[i] = '\0';
			    	i++;
			    }
	}

    return ans;
}

static long device_ioctl(//struct inode*  inode,
                         struct file*   file,
                         unsigned int   cmd,
                         unsigned long  arg) {
    struct list *tmp;
    char file_name[100];
    if(IOCTL_SET_CHA == cmd){
    	if (arg < 0 || arg > 3){
    		return -1;
    	}

    	sprintf(file_name, "%lu", file->f_inode->i_ino);
    	tmp = lookup(file_name);
    	if (tmp == NULL) {
    		printk("no such device (%p)\n", file);
    		return -1;
    	}
    	tmp->device_struct->num = arg;
    }
    return SUCCESS;
}

/************** Module Declarations *****************/

/* This structure will hold the functions to be called
 * when a process does something to the device we created */
struct file_operations Fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,    /* a.k.a. close */
    .unlocked_ioctl = device_ioctl,
};

/* Called when module is loaded.
 * Initialize the module - Register the character device */
static int __init simple_init(void)
{
    /* init dev struct*/
	unsigned int rc = 0;
    memset(&device_info, 0, sizeof(struct chardev_info));
    spin_lock_init(&device_info.lock);

    /* Register a character device. Get newly assigned major num */
    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops /* our own file operations struct */);

    /*
     * Negative values signify an error
     */
    if (rc < 0) {
        printk(KERN_ALERT "%s failed with %d\n",
               "Sorry, registering the character device ", MAJOR_NUM);
        return -1;
    }

    printk("Registeration is a success. The major device number is %d.\n", MAJOR_NUM);
    printk("If you want to talk to the device driver,\n");
    printk("you have to create a device file:\n");
    printk("mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
    printk("You can echo/cat to/from the device file.\n");
    printk("Dont forget to rm the device file and rmmod when you're done\n");

    return 0;
}

/* Cleanup - unregister the appropriate file from /proc */
static void __exit simple_cleanup(void)
{
    /*
     * Unregister the device
     * should always succeed (didnt used to in older kernel versions)
     */
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);



