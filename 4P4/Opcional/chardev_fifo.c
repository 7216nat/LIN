#include <linux/module.h>
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>

// añadidos
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/kfifo.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Chardev_fifo");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro Miranda");


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev_fifo"	/* Dev name as it appears in /proc/devices   */
#define MAX_CHARS_KBUF	64		/* Max length of the message from the device */

/*
 * Global variables are declared as static, so are global within the file.
 */

dev_t start;
struct cdev* chardev_fifo=NULL;

// añadidos
static struct kfifo cbuf;
struct semaphore prod_queue,cons_queue;
struct semaphore mtx;
int nr_prod_waiting, nr_cons_waiting;
int prod_count = 0, cons_count = 0;

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    int major;		/* Major number assigned to our device driver */
    int minor;		/* Minor number assigned to the associated character device */
    int ret;
	
    /* Get available (major,minor) range */
    if ((ret=alloc_chrdev_region (&start, 0, 1,DEVICE_NAME))) {
        printk(KERN_INFO "Can't allocate chrdev_region()");
        return ret;
    }

    /* Create associated cdev */
    if ((chardev_fifo=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        unregister_chrdev_region(start, 1);
        return -ENOMEM;
    }

    cdev_init(chardev_fifo,&fops);

    if ((ret=cdev_add(chardev_fifo,start,1))) {
        printk(KERN_INFO "cdev_add() failed ");
        kobject_put(&chardev_fifo->kobj);
        unregister_chrdev_region(start, 1);
        return ret;
    }
	
    major=MAJOR(start);
    minor=MINOR(start);
    
    if (kfifo_alloc(&cbuf,MAX_CHARS_KBUF*sizeof(char),GFP_KERNEL)){
		printk(KERN_INFO "kfifo_alloc() failed");
		cdev_del(chardev_fifo);
		kobject_put(&chardev_fifo->kobj);
        unregister_chrdev_region(start, 1);
		return -ENOMEM;
	}
	
    /* Inicialización a 0 de los semáforos usados como colas de espera */
	sema_init(&prod_queue,0);
	sema_init(&cons_queue,0);

	/* Inicializacion a 1 del semáforo que permite acceso en exclusión mutua a la SC */
	sema_init(&mtx,1);
	
	nr_prod_waiting=nr_cons_waiting=0;

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
    printk(KERN_INFO "Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /* Destroy chardev */
    if (chardev_fifo)
        cdev_del(chardev_fifo);
	kfifo_free(&cbuf);
    /*
     * Unregister the device
     */
    unregister_chrdev_region(start, 1);
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev_leds"
 */
static int device_open(struct inode *inode, struct file *file)
{
    if (down_interruptible(&mtx)) return -EINTR;
	
	if(file->f_mode & FMODE_READ){ // cons
		cons_count++;
		
		// cond_signal(condProd)
		if(nr_prod_waiting > 0){
			nr_prod_waiting--;
			up(&prod_queue);
		}
		
		while(prod_count == 0) {
			// cond_wait(condCons, mtx)
			nr_cons_waiting++;
			up(&mtx);
			if(down_interruptible(&cons_queue)){
				down(&mtx);
				nr_cons_waiting--;
				cons_count--;
				up(&mtx);
				return -EINTR;
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}
	} else { // prod
		prod_count++;
			
		// cond_signal(condCons)
		if(nr_cons_waiting > 0){
			nr_cons_waiting--;
			up(&cons_queue);
		}
		
		while(cons_count == 0) {
			// cond_wait(condProd, mtx)
			nr_prod_waiting++;
			up(&mtx);
			if(down_interruptible(&prod_queue)){
				down(&mtx);
				nr_prod_waiting--;
				prod_count--;
				up(&mtx);
				return -EINTR;
			}
			if(down_interruptible(&mtx)) return -EINTR;
		}
	}
	up(&mtx);

    /* Increase the module's reference counter */
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    if (down_interruptible(&mtx)) return -EINTR;
	
	if(file->f_mode & FMODE_READ){ // cons
		cons_count--;
		// cond_signal(condProd)
		if (nr_prod_waiting > 0){
			nr_prod_waiting--;
			up(&prod_queue);
		}
		
	} else { //prod
		prod_count--;
			
		// cond_signal(condCons)
		if (nr_cons_waiting > 0){
			nr_cons_waiting--;
			up(&cons_queue);
		}
	}
	
	// vaciar el buffer si no queda consumidor ni productor
	if (cons_count == 0 && prod_count == 0) kfifo_reset(&cbuf);
	up(&mtx);

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);

    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    int extracted_bytes;
	char kbuff[MAX_CHARS_KBUF+1];

	if ((*off) > 0)
		return 0;

	/* Entrar a la sección crítica */
	if (down_interruptible(&mtx)) {
		return -EINTR;
	}

	/* Bloquearse mientras buffer esté vacío (no haya un entero) */
	while (kfifo_len(&cbuf)<len && prod_count > 0) {
		/* Incremento de consumidores esperando */
		nr_cons_waiting++;

		/* Liberar el 'mutex' antes de bloqueo*/
		up(&mtx);

		/* Bloqueo en cola de espera */
		if (down_interruptible(&cons_queue)) {
			down(&mtx);
			nr_cons_waiting--;
			up(&mtx);
			return -EINTR;
		}

		/* Readquisición del 'mutex' antes de entrar a la SC */
		if (down_interruptible(&mtx)) {
			return -EINTR;
		}
	}
	
	if (prod_count == 0 && kfifo_is_empty(&cbuf)) {
		up(&mtx);
		return 0;
	}
	/* Extraer el primer entero del buffer */
	extracted_bytes = kfifo_out(&cbuf,kbuff,len);

	/* Despertar a los consumidores bloqueados (si hay alguno) */
	if (nr_prod_waiting>0) {
		nr_prod_waiting--;
		up(&prod_queue);
	}

	/* Salir de la sección crítica */
	up(&mtx);

	if (copy_to_user(buf,kbuff,len))
		return -EFAULT;

	return len;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev_leds
 */
static ssize_t
device_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    char kbuff[MAX_CHARS_KBUF+1];

	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;

	if (len > MAX_CHARS_KBUF) {
		return -ENOSPC;
	}
	if (copy_from_user( kbuff, buf, len )) {
		return -EFAULT;
	}

	kbuff[len] ='\0';

	/* Acceso a la sección crítica */
	if (down_interruptible(&mtx))
		return -EINTR;

	/* Bloquearse mientras no haya huecos en el buffer */
	while (kfifo_avail(&cbuf) < len && cons_count > 0) {
		/* Incremento de productores esperando */
		nr_prod_waiting++;

		/* Liberar el 'mutex' antes de bloqueo*/
		up(&mtx);

		/* Bloqueo en cola de espera */
		if (down_interruptible(&prod_queue)) {
			down(&mtx);
			nr_prod_waiting--;
			up(&mtx);
			return -EINTR;
		}

		/* Readquisición del 'mutex' antes de entrar a la SC */
		if (down_interruptible(&mtx)) {
			return -EINTR;
		}
	}
	
	// salir si no que consumidor
	if (cons_count==0){
		up(&mtx);
		return -EPIPE;
	}
	/* Insertar en el buffer */
	kfifo_in(&cbuf,kbuff,len);

	/* Despertar a los productores bloqueados (si hay alguno) */
	if (nr_cons_waiting>0) {
		nr_cons_waiting--;
		up(&cons_queue);
	}

	/* Salir de la sección crítica */
	up(&mtx);

	return len;
}
