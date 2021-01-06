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
#define MAX_USER_BUF    64

/* codeconfig */
unsigned int timer_period_ms = 1000;
unsigned int emergency_threshold = 75;
char *code_format = "aA00";

/* mecanismos de sincronizacion */
static spinlock_t mtx_buf;
struct semaphore mtx_list, cond_list;
unsigned int nr_list_waiting;

/* mecanismos de workqueue */
static struct workqueue_struct* my_wq;
struct work_struct my_work;

static struct proc_dir_entry *proc_entry_codetimer;
static struct kfifo cbuf;
struct timer_list my_timer; /* Structure that describes the kernel timer */

/* list */
struct list_head mylist; // nodo fantasma
struct list_item {
	char data[MAX_ITEM_SIZE];
	struct list_head links;
};

void list_cleanup ( void ){
    struct list_item *it, *item = NULL;

    down(&mtx_list);
	
	list_for_each_entry_safe(item, it, &mylist, links){ // esto recorre las entradas de la lista
		list_del(&(item->links));
		vfree(item);
	}

    up(&mtx_list);
}

void copy_items_into_list( void ){
    
    unsigned long irq_flags;
    char buf_tmp[MAX_LEN_FIFO+1];
    char *ptr = buf_tmp;
    int i, num = 0, len = strlen(code_format);

    struct list_item *item = NULL;

    spin_lock_irqsave(&mtx_buf, irq_flags);

    while(!kfifo_is_empty(&cbuf)){
        ptr += kfifo_out(&cbuf, ptr, len);
        num++;
    }
    kfifo_reset(&cbuf);
    printk(KERN_INFO "kfifo reset.\n");

    spin_unlock_irqrestore(&mtx_buf, irq_flags);

    down(&mtx_list);
    
    ptr = buf_tmp;
    for (i = 0; i < num; i++){
        item = (struct list_item *)vmalloc(sizeof(struct list_item));
        snprintf(item->data, len + 1 ,ptr);
        list_add_tail(&item->links, &mylist);
        ptr+=len;
    }

    if (nr_list_waiting > 0){
        nr_list_waiting--;
        up(&cond_list);
    }
    
    up(&mtx_list);
    printk(KERN_INFO "%d codes moved from the buffer to the list.\n", num);
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
        if(((kfifo_len(&cbuf)*100/MAX_LEN_FIFO) >= emergency_threshold && !work_pending(&my_work))){
            printk(KERN_INFO "Diferiendo trabajo\n");
            INIT_WORK(&my_work, my_wq_function);
            queue_work(my_wq, &my_work);
        }
    }

    /************ PRUEBAS  ***********
    if(((kfifo_len(&cbuf)*100/MAX_LEN_FIFO) >= emergency_threshold)){
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

static int codetimer_open(struct inode *inode, struct file *file) {
    try_module_get(THIS_MODULE);
    /* Activate the timer for the first time */
    add_timer(&my_timer); 
    return 0;
}

static int codetimer_release (struct inode *inode, struct file *file){
    // 1
    del_timer_sync(&my_timer);
    // 2
    flush_workqueue(my_wq);
    // 3
    kfifo_reset(&cbuf);
    // 4
    list_cleanup();
    // 5
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t codetimer_read(struct file *filp, char __user *buf, size_t len, loff_t *off){

    int extracted_bytes = 0;
	char kbuf[MAX_USER_BUF+1];
    char *ptr = kbuf;
    struct list_item *it, *item = NULL; 

	if ((*off) > 0)
		return 0;
    
    /* Entrar a la sección crítica */
	if (down_interruptible(&mtx_list)) {
		return -EINTR;
	}

	/* Bloquearse mientras la lista esté vacía */
	while (list_empty(&mylist)) {
		/* Incremento de consumidores esperando */
		nr_list_waiting++;

		/* Liberar el 'mutex' antes de bloqueo*/
		up(&mtx_list);

		/* Bloqueo en cola de espera */
		if (down_interruptible(&cond_list)) {
			down(&mtx_list);
			nr_list_waiting--;
			up(&mtx_list);
			return -EINTR;
		}

		/* Readquisición del 'mutex' antes de entrar a la SC */
		if (down_interruptible(&mtx_list)) {
			return -EINTR;
		}
	}

    list_for_each_entry_safe(item, it, &mylist, links){
        ptr += sprintf(ptr, "%s\n", item->data);
		list_del(&(item->links));
		vfree(item);
	}

    up(&mtx_list);

    extracted_bytes = ptr - kbuf;
    if(extracted_bytes > len)
        return -ENOSPC;

    if (copy_to_user(buf, kbuf, extracted_bytes))
		return -EINVAL;
    return extracted_bytes;
}

static const struct file_operations proc_entry_fops_codetimer = {
    .open = codetimer_open,
    .release = codetimer_release,
    .read = codetimer_read,
};

int init_codetimer_module( void ){

    proc_entry_codetimer = proc_create( "codetimer", 0666, NULL, &proc_entry_fops_codetimer);
    if (proc_entry_codetimer == NULL) {
		printk(KERN_INFO "Modlist: Can't create /proc entry\n");
        return -ENOMEM;
	} 
    /* Mecanismos de concurrencias */
    spin_lock_init(&mtx_buf);
    sema_init(&mtx_list, 1);
	sema_init(&cond_list, 0);
    nr_list_waiting = 0;

    /* Crear buffer circular */
    if (kfifo_alloc(&cbuf, MAX_LEN_FIFO, GFP_KERNEL)){
		remove_proc_entry("codetimer", NULL);
        return -ENOMEM;
    }
    /* Inicializar list_head */ 
    INIT_LIST_HEAD(&mylist);

    /* Inicilizar workqueue*/
    my_wq = create_workqueue("my_queue");
    if (!my_wq){
        remove_proc_entry("codetimer", NULL);
        kfifo_free(&cbuf);
        return -ENOMEM;
    }
    /* Create timer */
    init_timer(&my_timer);
    /* Initialize field */
    my_timer.data=0;
    my_timer.function=fire_timer;
    my_timer.expires=jiffies + HZ;  /* Activate it one second from now */
    
    printk(KERN_INFO "Codetimer: Module loaded.\n");
    return 0;
}


void exit_codetimer_module( void ){
  /* Wait until completion of the timer function (if it's currently running) and delete timer */

    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    kfifo_free(&cbuf);
    remove_proc_entry("codetimer", NULL);
    printk(KERN_INFO "Codetimer: Module unloaded.\n");
}

module_init( init_codetimer_module );
module_exit( exit_codetimer_module );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("codetimer Module");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro Miranda");
