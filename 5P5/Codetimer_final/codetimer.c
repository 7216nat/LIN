#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/smp.h>
#include <linux/semaphore.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Alfaro, Xukai Chen");

#define STR_EXPAND(m) #m      
#define STR(m) STR_EXPAND(m)  						//Macro para rodear de comas otra macro tras su expansion

#define MAX_CODE_CHARS 8
#define CONFIG_COMMANDS_LENGHT 100   				//Tamaño del buffer para leer los comandos de escritura sobre /proc/codeconfig
#define READ_BUFF_SIZE 128			 				//Tamaño del buffer para transferir la configuracion del modulo en las lecturas sobre /proc/codeconfig
#define MAX_CBUFFER_BYTES 32

static unsigned int timer_period_ms = 500;
static char code_format[MAX_CODE_CHARS + 1] = "00A";
static unsigned int emergency_threshold = 75;
DEFINE_SPINLOCK(config_splock);  					//Spinlock para proteger el acceso a las variables de configuracion

static struct kfifo cbuf;
static unsigned int cbuf_numcodes; 					//Numero de codigos en cbuf
DEFINE_SPINLOCK(cbuf_splock);						//Spinlock para proteger el acceso a cbuf y cbuf_numcodes

static struct timer_list gen_timer; 				//Timer para la generacion de codigos

static struct workqueue_struct* my_wq;
struct work_struct copy_work;						//Trabajo para copiar los codigos del buffer a la lista

struct codelist_item {
	char code[MAX_CODE_CHARS + 1];
	struct list_head links;
};

static void clear(struct list_head* mList){ 		//Procedimiento para borrar la memoria de los elementos de la lista
	struct list_head* cur_node=NULL;
	struct list_head* aux=NULL;
	struct codelist_item* pItem=NULL;

	list_for_each_safe(cur_node, aux, mList) {
		pItem = list_entry(cur_node, struct codelist_item, links);
		list_del(&(pItem->links));
		vfree(pItem);
	}
}

static struct list_head codelist_head;				//Lista de codigos

static struct semaphore codelist_sem; 				//Semaforo para proteger el acceso a la lista

static struct semaphore queue_sem;					//Semaforo para esperar a que haya codigos en la lista
static int nr_waiting;

/////////////////////////////////////////////////////////////////////////////////TOP HALF/////////////////////////////////////////////////////////////////////

// Por cada 5 bits se genera un nuevo caracter, se llama a la funcion get_random_int cuando se necesiten mas
static int gen_new_code(char* dest, char* format) {
	unsigned int random = get_random_int();

	unsigned int totalBits = sizeof(unsigned int)*8;
	unsigned int bitsUsed = 0;
	int i;

	for(i = 0; i < strlen(format); ++i){
		dest[i] = 0;
		dest[i] = (char) ((random & (0x1F << bitsUsed)) >> bitsUsed);
		if(format[i] == '0'){
			dest[i] = dest[i] % 10 + 48;
		}else if(format[i] == 'a'){
			dest[i] = dest[i] % 26 + 97;
		}
		else{
			dest[i] = dest[i] % 26 + 65;
		}
		bitsUsed += 3;
		if(totalBits - bitsUsed < 5){
			random = get_random_int();
			bitsUsed = 0;
		}
	}
	dest[i] = '\0';
	return i;
}


static void fire_timer(unsigned long data) {

	char code[MAX_CODE_CHARS + 1];
	int code_lenght;
	int cbuffer_bytes;
	int current_occupation;

	spin_lock(&config_splock);
	code_lenght = gen_new_code(code, code_format);

	spin_lock(&cbuf_splock);
	cbuffer_bytes = kfifo_len(&cbuf);
	if(MAX_CBUFFER_BYTES - cbuffer_bytes >= code_lenght + 1){
		kfifo_in(&cbuf, code, code_lenght + 1);
		cbuf_numcodes++;
		cbuffer_bytes = kfifo_len(&cbuf);
	}
	spin_unlock(&cbuf_splock);

	current_occupation = 100 * cbuffer_bytes / MAX_CBUFFER_BYTES;

	printk(KERN_INFO "Codetimer: Generated code: %s. Current occupation %d%%\n", code, current_occupation);
	if((current_occupation >= emergency_threshold || MAX_CBUFFER_BYTES - cbuffer_bytes < code_lenght + 1) && !work_pending(&copy_work)){
		int current_cpu = smp_processor_id();
		int cpu;
		for_each_cpu(cpu, cpu_online_mask){ //Se selecciona una cpu para planificar el trabajo con distinta paridad a la actual y que pueda recibir trabajo
			if(current_cpu % 2 == 0){
				if(cpu % 2 != 0) break;
			}
			else{
				if(cpu % 2 == 0) break;
			}
		}
		if(cpu < nr_cpu_ids){ //Si cpu >= que nr_cpu_ids no hay ninguna cpu disponible
			printk(KERN_INFO "Codetimer: Emergency threshold %d%% reached\n", emergency_threshold);
			printk(KERN_INFO "Codetimer: Timer runned in cpu %d. Work scheduled in cpu %d\n", current_cpu, cpu);
			queue_work_on(cpu, my_wq, &copy_work);
		} 
	}
	
	mod_timer(&gen_timer, jiffies + msecs_to_jiffies(timer_period_ms));
	spin_unlock(&config_splock);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////BOTTOM HALF//////////////////////////////////////////////////////////////////

static void copy_items_into_list(struct work_struct *work) {

    char copy_buffer[MAX_CBUFFER_BYTES];
    char* parse_buffer = copy_buffer;
    unsigned long flags = 0;
    int cbuffer_bytes = 0;
    int bytes_extracted = 0;
    unsigned int numcodes = 0;
    int i = 0;
    int j = 0;
    struct codelist_item** pItems=NULL;
    int codelen = 0;
    int error = 0;

    spin_lock_irqsave(&cbuf_splock, flags);
    cbuffer_bytes = kfifo_len(&cbuf);
    numcodes = cbuf_numcodes;
    bytes_extracted = kfifo_out(&cbuf, copy_buffer, cbuffer_bytes);
    cbuf_numcodes = 0;
    spin_unlock_irqrestore(&cbuf_splock, flags);

    if(bytes_extracted != cbuffer_bytes){
    	printk(KERN_ERR "Codetimer: Could not extract all bytes from the circular buffer\n");
    	return;
    }

	pItems=(struct codelist_item**)vmalloc(sizeof(struct codelist_item*)*numcodes);
	if(!pItems){
		printk(KERN_ERR "Codetimer: Could not allocate memory to transfer from buffer to list\n");
		return; //ENOMEM
	}

	while(cbuffer_bytes > 0 && (codelen = strlen(parse_buffer)) != 0){
		pItems[i]=(struct codelist_item*)vmalloc(sizeof(struct codelist_item));
		if(!pItems[i]){
			error = 1;
			break;
		}
		strcpy(pItems[i]->code, parse_buffer);
		parse_buffer += codelen + 1;
		cbuffer_bytes -= codelen + 1;
		++i;
	}

	if(error || down_interruptible(&codelist_sem)){
		for(j = 0; j < i; ++j){
			vfree(pItems[j]);
		}
		vfree(pItems);
		if(error) printk(KERN_ERR "Codetimer: Could not allocate memory to transfer from buffer to list\n");
		return;
	}
	for(i = 0; i < numcodes; ++i){
		list_add_tail(&(pItems[i]->links), &codelist_head);
	}
	if(nr_waiting > 0){
		up(&queue_sem);
		nr_waiting--;
	}
	up(&codelist_sem);
	vfree(pItems);
	printk(KERN_INFO "Codetimer: %d codes moved from the buffer to the list\n", numcodes);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////UPPER LAYER//////////////////////////////////////////////////////////////////

static struct proc_dir_entry *proc_entry_config; 	//Entrada del directorio /proc/codeconfig
static struct proc_dir_entry *proc_entry_codes; 	//Entrada del directorio /proc/codetimer


static ssize_t codeconfig_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) { 

	char command_buf[CONFIG_COMMANDS_LENGHT];
	char new_code_format[MAX_CODE_CHARS + 1];
	char auxChar;
	unsigned int uVal;
	unsigned long flags;
	int ret;

	if ((*off) > 0)
		return 0;

	if (len > CONFIG_COMMANDS_LENGHT-1) {
		printk(KERN_INFO "Codetimer: Not enough space to write command\n");
		return -ENOSPC;
	}

	if (copy_from_user(command_buf, buf, len))  
		return -EFAULT;

	command_buf[len] = '\0'; //Marcamos el final de la cadena del buffer para que no se lean mas argumentos en sscanf
	*off+=len;            
	ret = len;

	spin_lock_irqsave(&config_splock, flags);
	if(sscanf(command_buf, "timer_period_ms %u %c", &uVal, &auxChar) == 1){
		timer_period_ms = uVal;
	}
	else if(sscanf(command_buf, "emergency_threshold %u %c", &uVal, &auxChar) == 1){
		emergency_threshold = uVal;
	}
	else if(sscanf(command_buf, "code_format %"STR(MAX_CODE_CHARS)"s %c", new_code_format, &auxChar) == 1){
		if(strspn(new_code_format, "0aA") != strlen(new_code_format)){
			ret = -EINVAL;
		}
		else strcpy(code_format, new_code_format);
	}
	else{
		ret = -EINVAL;
	}
	spin_unlock_irqrestore(&config_splock, flags);

	return ret;
}

static ssize_t codeconfig_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

	char read_buf[READ_BUFF_SIZE];
	int copiedBytes = 0;
	unsigned long flags;
	int ret = 0;

	if ((*off) > 0)
		return 0;

	if(len < READ_BUFF_SIZE-1){
		printk(KERN_INFO "Codetimer: Read buffer must be at least of %d bytes to read from /proc/codeconfig\n", READ_BUFF_SIZE);
		return -ENOSPC;
	} 

	spin_lock_irqsave(&config_splock, flags);
	copiedBytes = snprintf(read_buf, READ_BUFF_SIZE, "timer_period_ms=%u\nemergency_threshold=%u\ncode_format=%s\n", 
		timer_period_ms, emergency_threshold, code_format);
	spin_unlock_irqrestore(&config_splock, flags);
	if(copiedBytes <= 0 || copiedBytes >= READ_BUFF_SIZE){
		ret = -ENOMEM;
	}else if(copy_to_user(buf, read_buf, copiedBytes)){
		ret = -EINVAL;
	}
	else ret = copiedBytes;
			

	(*off)+=copiedBytes;  

	return ret; 
}

static int codetimer_open(struct inode *inode, struct file *file) {

	if(!try_module_get(THIS_MODULE)) return -EPERM;
	if(module_refcount(THIS_MODULE) > 1){
		module_put(THIS_MODULE);
		return -EPERM;	
	} 

	spin_lock(&config_splock);
	gen_timer.expires = jiffies + msecs_to_jiffies(timer_period_ms);
	spin_unlock(&config_splock);
    add_timer(&gen_timer); 

	return 0;
}

static int codetimer_release(struct inode *inode, struct file *file) {

	del_timer_sync(&gen_timer);
	flush_workqueue(my_wq);
	kfifo_reset(&cbuf);
	cbuf_numcodes = 0;
	clear(&codelist_head);
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t codetimer_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

	char read_buf[MAX_CODE_CHARS + 2];
	char* writeBuf = buf;
	struct codelist_item *pItem = NULL;
	struct list_head *pos, *aux;
	int copiedBytes = 0;
	int ret = 0;

	if(len < MAX_CODE_CHARS + 1){
		printk(KERN_INFO "Codetimer: Read buffer must be at least of %d bytes to read from /proc/codetimer\n", MAX_CODE_CHARS + 1);
		return -ENOSPC;
	}

	if(down_interruptible(&codelist_sem)){
		return -EINTR;
	}

	while(list_empty(&codelist_head)){
		nr_waiting++;
		up(&codelist_sem);
		if(down_interruptible(&queue_sem)){
			down(&codelist_sem);
			nr_waiting--;
			up(&codelist_sem);
			return -EINTR;
		}

		if(down_interruptible(&codelist_sem)){
			return -EINTR;
		}
	}

	list_for_each_safe(pos, aux, &codelist_head){
		pItem = list_entry(pos, struct codelist_item, links);

		copiedBytes = snprintf(read_buf, MAX_CODE_CHARS + 2, "%s\n", pItem->code); 
		if(copiedBytes <= 0 || copiedBytes >= MAX_CODE_CHARS + 2){
			ret = -ENOMEM;
			break;
		}
		else if(len < copiedBytes){
			break;
		}
		else if(copy_to_user(writeBuf, read_buf, copiedBytes)){
			ret = -EINVAL;
			break;
		}
		else{
			writeBuf += copiedBytes;
			len -= copiedBytes;
		}

		list_del(&(pItem->links));
		vfree(pItem);
	}

	up(&codelist_sem);

	if(ret == 0) ret = writeBuf - buf;

	(*off)+= writeBuf - buf;  

	return ret; 
}


static const struct file_operations proc_entry_config_fops = {
    .read = codeconfig_read,
    .write = codeconfig_write,    
};

static const struct file_operations proc_entry_codes_fops = {
	.owner = THIS_MODULE,
    .read = codetimer_read,
    .open = codetimer_open,
    .release = codetimer_release,    
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int init_codetimer_module(void){

	int ret = 0;

	if((ret = kfifo_alloc(&cbuf, MAX_CBUFFER_BYTES, GFP_KERNEL))){
		printk(KERN_INFO "Codetimer: Can't allocate circular buffer\n");
		return ret;
	}
	cbuf_numcodes = 0;

	my_wq = create_workqueue("codetimer_queue");
  
  	if (!my_wq){
  		printk(KERN_INFO "Codetimer: Can't create workqueue\n");
  		goto wq_error;
  	}

  	INIT_WORK(&copy_work, copy_items_into_list);

	proc_entry_config = proc_create("codeconfig", 0666, NULL, &proc_entry_config_fops);
	if (!proc_entry_config) {
      	printk(KERN_INFO "Codetimer: Can't create /proc entry\n");
      	goto proc_config_error;
    }
    proc_entry_codes = proc_create("codetimer", 0666, NULL, &proc_entry_codes_fops);
    if (!proc_entry_codes) {
      	printk(KERN_INFO "Codetimer: Can't create /proc entry\n");
      	goto proc_codes_error;
    }
    
    init_timer(&gen_timer);
    gen_timer.data=0;
    gen_timer.function=fire_timer;

    INIT_LIST_HEAD(&codelist_head);

    sema_init(&codelist_sem, 1);
    sema_init(&queue_sem, 0);
    nr_waiting = 0;

    return 0;

proc_codes_error:
	remove_proc_entry("codeconfig", NULL);
proc_config_error:
	destroy_workqueue(my_wq);    
wq_error:
	kfifo_free(&cbuf);
	return -ENOMEM;
}


static void cleanup_codetimer_module(void){
	
  	remove_proc_entry("codeconfig", NULL);
  	remove_proc_entry("codetimer", NULL);
  	destroy_workqueue( my_wq );
  	kfifo_free(&cbuf);
  	clear(&codelist_head);
}

module_init(init_codetimer_module);
module_exit(cleanup_codetimer_module);