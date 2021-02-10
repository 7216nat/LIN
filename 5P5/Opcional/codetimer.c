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
static unsigned int emergency_threshold = 75;
static int even_proc;								//Guarda si el proceso que hace la lectura de los codigos pares esta activo
static int num_proc;								//Guarda el numero de procesos que tienen abierto /proc/codetimer
DEFINE_SPINLOCK(config_splock);  					//Spinlock para proteger el acceso a las variables de configuracion (y even_proc y num_proc)

static struct kfifo cbuf;
static unsigned int cbuf_numcodes; 					//Numero de codigos en cbuf
DEFINE_SPINLOCK(cbuf_splock);						//Spinlock para proteger el acceso a cbuf y cbuf_numcodes

static struct timer_list gen_timer; 				//Timer para la generacion de codigos

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

static struct list_head codelist_head_even;				//Lista de codigos pares
static struct list_head codelist_head_odd;				//Lista de codigos impares

static struct semaphore codelist_sem_odd; 				//Semaforo para proteger el acceso a la lista de impares

static struct semaphore queue_sem_odd;					//Semaforo para esperar a que haya codigos en la lista de impares
static int nr_waiting_odd;

static struct semaphore codelist_sem_even; 				//Semaforo para proteger el acceso a la lista de pares

static struct semaphore queue_sem_even;					//Semaforo para esperar a que haya codigos en la lista de pares
static int nr_waiting_even;

/////////////////////////////////////////////////////////////////////////////////TOP HALF/////////////////////////////////////////////////////////////////////


static void gen_code_format(char* dest, int max_code_length){
	unsigned int random = get_random_int();
	int lenght = (0xF & random) % max_code_length + 1;
	unsigned int totalBits = sizeof(unsigned int)*8;
	unsigned int bitsUsed = 4;
	int i;

	for(i = 0; i < lenght; ++i){
		dest[i] = 0;
		dest[i] = (char) ((random & (0x3 << bitsUsed)) >> bitsUsed);
		dest[i] %= 3;
		if(dest[i] == 0){
			dest[i] = '0';
		}	
		else if(dest[i] == 1){
			dest[i] = 'a';
		}
		else{
			dest[i] = 'A';
		}
		bitsUsed += 2;
		if(totalBits - bitsUsed < 2){
			random = get_random_int();
			bitsUsed = 0;
		}
	}
	dest[i] = '\0';
}

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

	char code_format[MAX_CODE_CHARS + 1];
	char code[MAX_CODE_CHARS + 1];
	int code_lenght;
	int cbuffer_bytes;
	int current_occupation;

	gen_code_format(code_format, MAX_CODE_CHARS);
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
	spin_lock(&config_splock);
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
			schedule_work_on(cpu, &copy_work);
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
    int evencodes = 0;

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

	if(error || down_interruptible(&codelist_sem_even)){
		for(j = 0; j < i; ++j){
			vfree(pItems[j]);
		}
		vfree(pItems);
		if(error) printk(KERN_ERR "Codetimer: Could not allocate memory to transfer from buffer to list\n");
		return;
	}
	else if(down_interruptible(&codelist_sem_odd)){
		up(&codelist_sem_even);
		for(j = 0; j < i; ++j){
			vfree(pItems[j]);
		}
		vfree(pItems);
	}
	for(i = 0; i < numcodes; ++i){
		if(strlen(pItems[i]->code) % 2 == 0){
			list_add_tail(&(pItems[i]->links), &codelist_head_even);
			evencodes++;
		}else{
			list_add_tail(&(pItems[i]->links), &codelist_head_odd);
		}
	}
	if(nr_waiting_even > 0 && evencodes > 0){
		up(&queue_sem_even);
		nr_waiting_even--;
	}
	if(nr_waiting_odd > 0 && numcodes - evencodes > 0){
		up(&queue_sem_odd);
		nr_waiting_odd--;
	}
	up(&codelist_sem_odd);
	up(&codelist_sem_even);
	vfree(pItems);
	printk(KERN_INFO "Codetimer: %d codes moved from the buffer to the list\n", numcodes);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////UPPER LAYER//////////////////////////////////////////////////////////////////

static struct proc_dir_entry *proc_entry_config; 	//Entrada del directorio /proc/codeconfig
static struct proc_dir_entry *proc_entry_codes; 	//Entrada del directorio /proc/codetimer


static ssize_t codeconfig_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) { 

	char command_buf[CONFIG_COMMANDS_LENGHT];
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
	copiedBytes = snprintf(read_buf, READ_BUFF_SIZE, "timer_period_ms=%u\nemergency_threshold=%u\n", 
		timer_period_ms, emergency_threshold);
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
	if(module_refcount(THIS_MODULE) > 2){
		module_put(THIS_MODULE);
		return -EPERM;	
	}

	file->private_data = vmalloc(sizeof(char));
	if(!file->private_data){
		module_put(THIS_MODULE);
		return -ENOMEM;
	}
	spin_lock(&config_splock);
	if(even_proc){
		*((char*)file->private_data) = 1;
	} 
	else{
		*((char*)file->private_data) = 0;
		even_proc = 1;
	} 
	num_proc++;
	if(num_proc == 2){
		gen_timer.expires = jiffies + msecs_to_jiffies(timer_period_ms);
		add_timer(&gen_timer); 
	} 
	spin_unlock(&config_splock);
	
	return 0;
}

static int codetimer_release(struct inode *inode, struct file *file) {

	spin_lock(&config_splock);
	if(num_proc == 2){
		del_timer_sync(&gen_timer);
		flush_work(&copy_work);
		kfifo_reset(&cbuf);
		cbuf_numcodes = 0;
		clear(&codelist_head_odd);
		clear(&codelist_head_even);
	}
	
	if(*((char*)file->private_data) == 0){
		even_proc = 0;
	} 
	vfree(file->private_data);
	num_proc--;
	spin_unlock(&config_splock);
	
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t codetimer_read(struct file *file, char __user *buf, size_t len, loff_t *off) {

	char read_buf[MAX_CODE_CHARS + 2];
	char* writeBuf = buf;
	struct codelist_item *pItem = NULL;
	struct list_head *pos, *aux;
	int copiedBytes = 0;
	int ret = 0;
	struct semaphore* list_sem;
	struct semaphore* queue_sem;
	struct list_head* codelist_head;
	int* nr_waiting;

	if(*((char*)file->private_data) == 0){
		list_sem = &codelist_sem_even;
		queue_sem = &queue_sem_even;
		codelist_head = &codelist_head_even;
		nr_waiting = &nr_waiting_even;
	}
	else{
		list_sem = &codelist_sem_odd;
		queue_sem = &queue_sem_odd;
		codelist_head = &codelist_head_odd;
		nr_waiting = &nr_waiting_odd;
	}

	if(len < MAX_CODE_CHARS + 1){
		printk(KERN_INFO "Codetimer: Read buffer must be at least of %d bytes to read from /proc/codetimer\n", MAX_CODE_CHARS + 1);
		return -ENOSPC;
	}

	if(down_interruptible(list_sem)){
		return -EINTR;
	}

	while(list_empty(codelist_head)){
		(*nr_waiting)++;
		up(list_sem);
		if(down_interruptible(queue_sem)){
			down(list_sem);
			(*nr_waiting)--;
			up(list_sem);
			return -EINTR;
		}

		if(down_interruptible(list_sem)){
			return -EINTR;
		}
	}

	list_for_each_safe(pos, aux, codelist_head){
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

	up(list_sem);

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

  	INIT_WORK(&copy_work, copy_items_into_list);

	proc_entry_config = proc_create("codeconfig", 0666, NULL, &proc_entry_config_fops);
	if (!proc_entry_config) {
      	printk(KERN_INFO "Codetimer: Can't create /proc entry\n");
      	kfifo_free(&cbuf);
		return -ENOMEM;
    }
    proc_entry_codes = proc_create("codetimer", 0666, NULL, &proc_entry_codes_fops);
    if (!proc_entry_codes) {
      	printk(KERN_INFO "Codetimer: Can't create /proc entry\n");
      	remove_proc_entry("codeconfig", NULL);
      	kfifo_free(&cbuf);
		return -ENOMEM;
    }
    
    init_timer(&gen_timer);
    gen_timer.data=0;
    gen_timer.function=fire_timer;

    INIT_LIST_HEAD(&codelist_head_odd);
    INIT_LIST_HEAD(&codelist_head_even);

    sema_init(&codelist_sem_even, 1);
    sema_init(&codelist_sem_odd, 1);
    sema_init(&queue_sem_even, 0);
    sema_init(&queue_sem_odd, 0);
    nr_waiting_even = 0;
    nr_waiting_odd = 0;

    even_proc = 0;
    num_proc = 0;

    return 0;
}


static void cleanup_codetimer_module(void){
	
  	remove_proc_entry("codeconfig", NULL);
  	remove_proc_entry("codetimer", NULL);
  	kfifo_free(&cbuf);
  	clear(&codelist_head_odd);
  	clear(&codelist_head_even);
}

module_init(init_codetimer_module);
module_exit(cleanup_codetimer_module);