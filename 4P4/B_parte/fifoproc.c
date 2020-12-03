#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <asm-generic/errno.h>
#include <linux/semaphore.h>
#include <linux/kfifo.h>


#define MAX_CHARS_KBUF	64

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fifoproc para LIN");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro Miranda");

static struct proc_dir_entry *proc_entry;
static struct kfifo cbuf;
struct semaphore prod_queue,cons_queue;
struct semaphore mtx;
int nr_prod_waiting, nr_cons_waiting;
int prod_count = 0, cons_count = 0;

static int fifoproc_open(struct inode *inode, struct file *file) {
	
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
	return 0;
}

static int fifoproc_release (struct inode *inode, struct file *file){
	if (down_interruptible(&mtx)) return -EINTR;
	
	if(file->f_mode & FMODE_READ){ // cons
		cons_count--;
		// cond_broadcast(condProd)
		while (nr_prod_waiting > 0){
			nr_prod_waiting--;
			up(&prod_queue);
		}
		
	} else { //prod
		prod_count--;
			
		// cond_broadcast(condCons)
		while (nr_cons_waiting > 0){
			nr_cons_waiting--;
			up(&cons_queue);
		}
	}
	
	// vaciar el buffer si no queda consumidor ni productor
	if (cons_count == 0 && prod_count == 0) kfifo_reset(&cbuf);
	up(&mtx);
	return 0;
}

static ssize_t fifoproc_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
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
	//*off+=len;            /* Update the file pointer */

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


static ssize_t fifoproc_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
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

	//(*off)+=len;  /* Update the file pointer */

	return len;
}

static const struct file_operations proc_entry_fops = {
	.read = fifoproc_read,
	.write = fifoproc_write,
	.open = fifoproc_open,
	.release = fifoproc_release
};

int init_fifoproc_module( void )
{
	int retval;
	/* Inicialización del buffer */
	retval = kfifo_alloc(&cbuf,MAX_CHARS_KBUF*sizeof(char),GFP_KERNEL);

	if (retval)
		return -ENOMEM;

	/* Inicialización a 0 de los semáforos usados como colas de espera */
	sema_init(&prod_queue,0);
	sema_init(&cons_queue,0);

	/* Inicializacion a 1 del semáforo que permite acceso en exclusión mutua a la SC */
	sema_init(&mtx,1);

	nr_prod_waiting=nr_cons_waiting=0;

	proc_entry = proc_create_data("fifoproc",0666, NULL, &proc_entry_fops, NULL);

	if (proc_entry == NULL) {
		kfifo_free(&cbuf);
		printk(KERN_INFO "Fifoproc: No puedo crear la entrada en proc\n");
		return  -ENOMEM;
	}

	printk(KERN_INFO "Fifoproc: Cargado el Modulo.\n");

	return 0;
}


void exit_fifoproc_module( void )
{
	remove_proc_entry("fifoproc", NULL);
	kfifo_free(&cbuf);
	printk(KERN_INFO "Fifoproc: Modulo descargado.\n");
}


module_init( init_fifoproc_module );
module_exit( exit_fifoproc_module );
