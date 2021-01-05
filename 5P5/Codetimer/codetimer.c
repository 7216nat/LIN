#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/semaphore.h>
#include <linux/kfifo.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/vmalloc.h>

/* Constantes */
#define MAX_LEN_FIFO    32
#define MAX_ITEM_SIZE   8

/* codeconfig */
unsigned int timer_period_ms = 1000;
unsigned int emergency_threshold = 75;
char *code_format = "aA00";

/* mecanismos de sincronizacion */
static spinlock_t mtx_buf;
struct semaphore mtx_list, cond_list;
unsigned int nr_list_waiting;

/* mecanismos de workqueue */
static struct workqueue_struct *my_wq;
struct work_struct my_work;

static struct proc_dir_entry *proc_entry_codetimer;
static struct kfifo cbuf;
struct timer_list my_timer; /* Structure that describes the kernel timer */

/* list */
struct list_head mylist; // nodo fantasma
struct list_item {
	int data[MAX_ITEM_SIZE];
	struct list_head links;
};

int list_cleanup ( void ){
    struct list_item *it, *item = NULL;

    if(down_interruptible(&mtx_list)){
        return -EINTR;
    }
	
	list_for_each_entry_safe(item, it, &mylist, links){ // esto recorre las entradas de la lista
		list_del(&(item->links));
		vfree(item);
	}

    up(&mtx_list);
    
    return 0;
}

/* Work's handler function */
static void my_wq_function( struct work_struct *work ){
    copy_items_into_list();
}

/* Function invoked when timer expires (fires) */
static void fire_timer(unsigned long data){
	
    unsigned long irq_flags;
    int len = strlen(code_format), i;
    unsigned int rand = get_random_int();
    char code[len+1];
    
    code[len] = '\0'; 
    for (i = 0 ; i < len; i++){
        if (isalpha(code_format[i])) {
            code[i] = (((rand >> i*8) & 0xff)%26)+code_format[i];
        }
        else code[i] = (((rand >> i*8) & 0xff)%10)+code_format[i];
    }
    
    spin_lock_irqsave(&mtx_buf, irq_flags);

    if(kfifo_avail(&cbuf) > len){
        kfifo_in(&cbuf, &code, sizeof(char)*len);
        printk(KERN_INFO "Generated code: %s\n", code);
        
        if(((kfifo_len(&cbuf)*100/MAX_LEN_FIFO) >= emergency_threshold) && !work_pending(my_work)){
            INIT_WORK(&my_work, my_wq_function);
            my_wq = create_workqueue("my_queue");
            queue_work(my_wq, &my_work);
        }
    }

    /************ PRUEBAS  ************
    if(kfifo_avail(&cbuf) < len){
        kfifo_reset(&cbuf);
        printk(KERN_INFO "kfifo reset.\n");
    }
    kfifo_in(&cbuf, &code, sizeof(char)*len);
    printk(KERN_INFO "Generated code: %s\n", code);  
    */  

    spin_unlock_irqrestore(&mtx_buf, irq_flags);

    /* Re-activate the timer one second from now */
	mod_timer( &(my_timer), jiffies + (timer_period_ms/1000)*HZ); 
}

int init_codetimer_module( void ){

    try_module_get(THIS_MODULE);
    spin_lock_init(&mtx_buf);
    kfifo_alloc(&cbuf, MAX_LEN_FIFO, GFP_KERNEL);
    /* Create timer */
    init_timer(&my_timer);
    /* Initialize field */
    my_timer.data=0;
    my_timer.function=fire_timer;
    my_timer.expires=jiffies + HZ;  /* Activate it one second from now */
    /* Activate the timer for the first time */
    add_timer(&my_timer); 
    return 0;
}


void cleanup_codetimer_module( void ){
  /* Wait until completion of the timer function (if it's currently running) and delete timer */
    del_timer_sync(&my_timer);
    kfifo_free(&cbuf);
    module_put(THIS_MODULE);
}

/*static const struct file_operations proc_fops_codetimer = {
    .opne = codetimer_open,
    .release = codetimer_release,
    .read = codetimer_read,
};*/

module_init( init_codetimer_module );
module_exit( cleanup_codetimer_module );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("codetimer Module");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro Miranda");
