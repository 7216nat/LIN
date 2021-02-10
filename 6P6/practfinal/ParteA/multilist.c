#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/ftrace.h>
#include <linux/list.h>
#include <asm/spinlock.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Multilist Kernel Module - FDI-UCM");
MODULE_AUTHOR("Xukai Chen");

#define MAX_SIZE      		64
#define TEMP_BUFFER_LENGTH  32
#define COMMANDS_LENGTH		100	
#define N_SIZE				16

/* modules parameters */
static unsigned int max_entries = 5;
static unsigned int max_size = 10;

module_param(max_entries, uint, 0660);
MODULE_PARM_DESC(max_entries, "An unsigned short");
module_param(max_size, uint, 0660);
MODULE_PARM_DESC(max_size, "An unsigned short");

/* module directory */
struct proc_dir_entry *multilist=NULL;
static struct proc_dir_entry *proc_entry_admin; // admin proc 
static struct semaphore multilist_sem; 				
static int terminado = 0;

/* seccion critica de admin */
struct list_head list_entry; // nodo fantasma
static int entries = 0;

struct list_elem {
	char name[N_SIZE];
	int data_type;
	unsigned int num_elem;
	struct list_head data_list;
	spinlock_t mtx;
	struct list_head links;
};

struct list_item_int {
	int data;
	struct list_head links;
};

struct list_item_char {
	char *data;
	struct list_head links;
};

static void multilist_cleanup (void){
	
	struct list_elem *elem = NULL;
	struct list_elem *it_elem = NULL;
	
	if(down_interruptible(&multilist_sem)){
		return;
	}

	list_for_each_entry_safe(elem, it_elem, &list_entry, links){
		spin_lock(&(elem->mtx));
		if (elem->data_type){
			struct list_item_char *item = NULL;
			struct list_item_char *it = NULL;
			list_for_each_entry_safe(item, it, &(elem->data_list), links){ // esto recorre las entradas de la lista
				list_del(&(item->links));
				vfree(item->data);
				vfree(item);
			}
		}
		else {
			struct list_item_int *item = NULL;
			struct list_item_int *it = NULL;
			list_for_each_entry_safe(item, it, &(elem->data_list), links){ // esto recorre las entradas de la lista
				list_del(&(item->links));
				vfree(item);
			}
		}
		spin_unlock(&(elem->mtx));
		list_del(&(elem->links));
		vfree(elem);
		remove_proc_entry(elem->name, multilist);	
	}
	terminado = 1;
	up(&multilist_sem);
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	/* scope local */
	int available_space = COMMANDS_LENGTH-1;
	char command_buf[COMMANDS_LENGTH];
	int num;
	char temp[COMMANDS_LENGTH];

	/* scope global before*/
	int type;
	unsigned int *num_elem;
	struct list_head *data_list; 
	spinlock_t *mtx;

	struct list_item_char *item_char = NULL;
	struct list_item_char *it_char = NULL;
	struct list_item_int *item_int = NULL;
	struct list_item_int *it_int = NULL;

	struct list_elem *data_cb=(struct list_elem *)PDE_DATA(filp->f_inode);
	if (data_cb == NULL) {
		return -EINVAL;
	}
	type = data_cb->data_type;
	num_elem = &(data_cb->num_elem);
	data_list = &(data_cb->data_list);
	mtx = &(data_cb->mtx);
	
	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;

	if (len > available_space) {
		printk(KERN_INFO "modlist: command not enough space!!\n");
		return -ENOSPC;
	}

	/* Transfer data from user to kernel space */
	if (copy_from_user( command_buf, buf, len ))  
		return -EFAULT;

	command_buf[len] = '\0'; /* Add the `\0' */ 
	*off+=len;            /* Update the file pointer */
	
	trace_printk("Modlist: Current command: %s", command_buf);
	 
	if((!type && sscanf(command_buf, "add %d", &num) == 1)) {
		item_int = (struct list_item_int *) vmalloc(sizeof (struct list_item_int));
		if (!item_int)
			return -ENOMEM;
		item_int->data = num;

		spin_lock(mtx);
		if (*num_elem == max_size){
			vfree(item_int);
			spin_unlock(mtx);
			return -ENOSPC;
		}
		else{
			list_add_tail(&(item_int->links), data_list);
			(*num_elem)++;
		}
		spin_unlock(mtx);
	}
	else if(type && (sscanf(command_buf, "add %s", temp) == 1)) {
		if(strlen(temp) >= MAX_SIZE){
			printk(KERN_INFO "modlist: data size < %d expected\n", MAX_SIZE);
			return -ENOSPC;
		}
		 
		item_char = (struct list_item_char *) vmalloc(sizeof (struct list_item_char));
		if (!item_char)
			return -ENOMEM;

		item_char->data = vmalloc(sizeof(char)*strlen(temp)+1);
		if (!(item_char->data)){
			vfree(item_char);
			return -ENOMEM;
		}
		strcpy(item_char->data, temp);

		spin_lock(mtx);
		if (*num_elem == max_size){
			vfree(item_char->data);
			vfree(item_char);
			spin_unlock(mtx);
			return -ENOSPC;
		}
		else{
			list_add_tail(&(item_char->links), data_list);
			(*num_elem)++;
		}
		spin_unlock(mtx);
	}
	else if(!type && (sscanf(command_buf, "remove %d", &num) == 1)){
		spin_lock(mtx);
		list_for_each_entry_safe(item_int, it_int, data_list, links){
			if(item_int->data == num){
				list_del(&(item_int->links));
				vfree(item_int);
				(*num_elem)--;
			}
		}
		spin_unlock(mtx);
	}
	else if(type && (sscanf(command_buf, "remove %s", temp) == 1)){
		spin_lock(mtx);
		list_for_each_entry_safe(item_char, it_char, data_list, links){
			if(strcmp(item_char->data, temp) == 0){
				list_del(&(item_char->links));
				vfree(item_char->data);
				vfree(item_char);
				(*num_elem)--;
			}
		}
		spin_unlock(mtx);
	}
	else if(strncmp(command_buf, "cleanup\n", len) == 0){
		spin_lock(mtx);
		if (type){
			list_for_each_entry_safe(item_char, it_char, data_list, links){ // esto recorre las entradas de la lista
				list_del(&(item_char->links));
				vfree(item_char->data);
				vfree(item_char);
			}
		} else {
			list_for_each_entry_safe(item_int, it_int, data_list, links){ // esto recorre las entradas de la lista
				list_del(&(item_int->links));
				vfree(item_int);
			}
		}
		*num_elem = 0;
		spin_unlock(mtx);
	}
	else {
		printk(KERN_INFO "ERROR: comando inválido.\n");
		return -EINVAL;
	}
	return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
	/* scope local */
	int nr_bytes, total_bytes = 0, wt_bytes = 0, to_wt = 0;			// nr_bytes: aux, total_bytes: return value, wt_bytes: bytes to write, to_wt: max bytes can write on buffer 
	// struct list_item *it, *item = NULL; 							// iterator and temp value
	char rd_buf[TEMP_BUFFER_LENGTH];								// temp buffer
	char data_buf[MAX_SIZE];										// aux buffer, number to char 
	char *ptr_rdbuf = rd_buf, *ptr_buf = buf, *ptr_data = data_buf; // as names indicate
	
	/* scope global */
	int type;
	struct list_head *data_list;
	spinlock_t *mtx;
	
	struct list_elem *data_cb=(struct list_elem *)PDE_DATA(filp->f_inode);
	if (data_cb == NULL) 
		return -EINVAL;
	type = data_cb->data_type;
	data_list = &(data_cb->data_list);
	mtx = &(data_cb->mtx);
	
	if ((*off) > 0) /* Tell the application that there is nothing left to read */
		return 0;
	
	// commit P4 
	spin_lock(mtx);
	if (type){
		struct list_item_char *item = NULL;
		struct list_item_char *it = NULL;
		list_for_each_entry_safe(item, it, data_list, links){
			
			nr_bytes = ptr_rdbuf - rd_buf;
			to_wt = TEMP_BUFFER_LENGTH - nr_bytes;
			
			if (to_wt < sprintf(ptr_data, "%s\n", item->data)){
				trace_printk("Modlist: Looping on buffer: %d out %d.\n", nr_bytes, TEMP_BUFFER_LENGTH);
				do {
					wt_bytes = snprintf(ptr_rdbuf, to_wt, ptr_data);
					if (copy_to_user(ptr_buf, rd_buf, TEMP_BUFFER_LENGTH)){
						spin_unlock(mtx);
						return -EINVAL;
					}
					ptr_buf += TEMP_BUFFER_LENGTH - 1;		
					ptr_rdbuf = rd_buf;
					ptr_data += to_wt - 1;
					total_bytes += TEMP_BUFFER_LENGTH - 1;
					wt_bytes -= to_wt - 1;
					to_wt = TEMP_BUFFER_LENGTH;
				}while (TEMP_BUFFER_LENGTH <= wt_bytes);
			}
			ptr_rdbuf += sprintf(ptr_rdbuf, ptr_data);
			ptr_data = data_buf;
		}
	}
	else {
		struct list_item_int *item = NULL;
		struct list_item_int *it = NULL;
		list_for_each_entry_safe(item, it, data_list, links){
			
			nr_bytes = ptr_rdbuf - rd_buf;
			to_wt = TEMP_BUFFER_LENGTH - nr_bytes;
			
			if (to_wt < sprintf(ptr_data, "%d\n", item->data)){
				trace_printk("Modlist: Looping on buffer: %d out %d.\n", nr_bytes, TEMP_BUFFER_LENGTH);
				do {
					wt_bytes = snprintf(ptr_rdbuf, to_wt, ptr_data);
					if (copy_to_user(ptr_buf, rd_buf, TEMP_BUFFER_LENGTH)){
						spin_unlock(mtx);
						return -EINVAL;
					}
					ptr_buf += TEMP_BUFFER_LENGTH - 1;		
					ptr_rdbuf = rd_buf;
					ptr_data += to_wt - 1;
					total_bytes += TEMP_BUFFER_LENGTH - 1;
					wt_bytes -= to_wt - 1;
					to_wt = TEMP_BUFFER_LENGTH;
				}while (TEMP_BUFFER_LENGTH <= wt_bytes);
			}
			ptr_rdbuf += sprintf(ptr_rdbuf, ptr_data);
			ptr_data = data_buf;
		}
	}
	spin_unlock(mtx);
	total_bytes += ptr_rdbuf - rd_buf;
		
	if (len<total_bytes)
		return -ENOSPC;

	/* Transfer data from the kernel to userspace */  
	if (copy_to_user(ptr_buf, rd_buf, (ptr_rdbuf - rd_buf)))
		return -EINVAL;

	(*off)+=total_bytes;  /* Update the file pointer */

	return total_bytes; 
}

static const struct file_operations proc_entry_fops_others = {
    .read = modlist_read,
    .write = modlist_write,    
};

static ssize_t multilist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	/* scope local */
	int available_space = COMMANDS_LENGTH-1;
	char command_buf[COMMANDS_LENGTH];
	char type;
	char temp[COMMANDS_LENGTH];

	struct list_elem *data_cb;
	struct proc_dir_entry *entry;
	struct list_elem *elem = NULL;
	struct list_elem *it_elem = NULL;
	int enc = 0;
	
	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;

	if (len > available_space) {
		printk(KERN_INFO "modlist: command not enough space!!\n");
		return -ENOSPC;
	}

	/* Transfer data from user to kernel space */
	if (copy_from_user( command_buf, buf, len ))  
		return -EFAULT;

	command_buf[len] = '\0'; /* Add the `\0' */ 
	*off+=len;            /* Update the file pointer */
	
	trace_printk("AdminList: Current command: %s", command_buf);
	 
	if(sscanf(command_buf, "new %s %c", temp, &type) == 2) {
		if(strlen(temp) >= N_SIZE){
			printk(KERN_INFO "adminList: name size < %d expected\n", N_SIZE);
			return -ENOSPC;
		}
		if(down_interruptible(&multilist_sem)){
			return -EINTR;
		}
		if (terminado){
			printk(KERN_INFO "adminList: modulo descargado, no se ha añadido\n");
			up(&multilist_sem);
			return -EFAULT;
		}

		if (entries == max_entries){
			printk(KERN_INFO "adminList: max entries raised\n");
			up(&multilist_sem);
			return -ENOSPC;
		}
		data_cb = (struct list_elem *)vmalloc(sizeof(struct list_elem));
		if (!data_cb){
			up(&multilist_sem);
			return -ENOMEM;
		} 
		if (type == 's'){
			data_cb->data_type=1;
		}
		if (type == 'i'){
			data_cb->data_type=0;
		}
		data_cb->num_elem=0;
		spin_lock_init(&(data_cb->mtx));
		strcpy(data_cb->name, temp);
		INIT_LIST_HEAD(&(data_cb->data_list));
		list_add_tail(&(data_cb->links), &list_entry);

		entry = proc_create_data(temp, 0666, multilist, &proc_entry_fops_others, data_cb);
		if (!entry){
			up(&multilist_sem);
			list_del(&(data_cb->links));
			vfree(data_cb);
			return -ENOMEM;
		}
		
		entries++;
		up(&multilist_sem);
	}
	else if(sscanf(command_buf, "delete %s", temp) == 1){
		if (strcmp("admin", temp) == 0){
			printk(KERN_INFO "ERROR: can not delete admin.\n");
			return -EINVAL;
		}
		if(down_interruptible(&multilist_sem)){
			return -EINTR;
		}

		if (terminado){
			printk(KERN_INFO "adminList: modulo descargado, operation cancelled\n");
			up(&multilist_sem);
			return -EFAULT;
		}

		list_for_each_entry_safe(elem, it_elem, &list_entry, links){
			if(strcmp(elem->name, temp) == 0){
				spin_lock(&(elem->mtx));
				if (elem->data_type){
					struct list_item_char *item = NULL;
					struct list_item_char *it = NULL;
					list_for_each_entry_safe(item, it, &(elem->data_list), links){ // esto recorre las entradas de la lista
						list_del(&(item->links));
						vfree(item->data);
						vfree(item);
					}
				}
				else {
					struct list_item_int *item = NULL;
					struct list_item_int *it = NULL;
					list_for_each_entry_safe(item, it, &(elem->data_list), links){ // esto recorre las entradas de la lista
						list_del(&(item->links));
						vfree(item);
					}
				}
				spin_unlock(&(elem->mtx));
				list_del(&(elem->links));
				vfree(elem);
				remove_proc_entry(elem->name, multilist);
				entries--;
				enc = 1;
				break;
			}
		}
		up(&multilist_sem);
		if (!enc){
			return -ENOENT;
		}
	}
	else {
		printk(KERN_INFO "ERROR: comando inválido.\n");
		return -EINVAL;
	}
	return len;
}

static const struct file_operations proc_entry_fops_admin = {
    .write = multilist_write,    
};

int init_modlist_module( void ){
	
	int ret = 0;
	struct list_elem *data_cb;
	struct proc_dir_entry *test_entry;

	multilist = proc_mkdir("multilist", NULL);

	if (!multilist) {
        return -ENOMEM;
    }

	proc_entry_admin = proc_create( "admin", 0666, multilist, &proc_entry_fops_admin);
	if (!proc_entry_admin){
		remove_proc_entry("multilist", NULL);
	}
	
	sema_init(&multilist_sem, 1);
	INIT_LIST_HEAD(&list_entry);

	data_cb = (struct list_elem *)vmalloc(sizeof(struct list_elem));
	if (!data_cb){
		remove_proc_entry("admin", multilist);
		remove_proc_entry("multilist", NULL);
		return -ENOMEM;
	} 
	data_cb->data_type=0;
	data_cb->num_elem=0;
	spin_lock_init(&(data_cb->mtx));
	strcpy(data_cb->name, "test");
	INIT_LIST_HEAD(&(data_cb->data_list));
	list_add_tail(&(data_cb->links), &list_entry);

	test_entry = proc_create_data( "test", 0666, multilist, &proc_entry_fops_others, data_cb);
	if (!test_entry){
		remove_proc_entry("admin", multilist);
		remove_proc_entry("multilist", NULL);
		list_del(&(data_cb->links));
		vfree(data_cb);
		return -ENOMEM;
	}
	entries++;
	
	printk(KERN_INFO "Multilist: Module loaded\n");
	
	return ret;
}


void exit_modlist_module( void )
{	
	multilist_cleanup();
	remove_proc_entry("admin", multilist);
	remove_proc_entry("multilist", NULL); // eliminar la entrada del /proc
	printk(KERN_INFO "Multilist: Module unloaded.\n");
}

module_init( init_modlist_module );
module_exit( exit_modlist_module );
