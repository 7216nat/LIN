#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/ftrace.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modlist Kernel Module - FDI-UCM");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro");

#define BUFFER_LENGTH       512
#define TEMP_BUFFER_LENGTH  32
#define DATA_LENGTH			32
#define COMMANDS_LENGTH		100	

static struct proc_dir_entry *proc_entry;

struct list_head mylist; // nodo fantasma

#ifdef PARTE_OPCIONAL

struct list_item {
	char data[DATA_LENGTH];
	struct list_head links;
};

void modlist_cleanup ( void ){
	/* cleanup de la lista */
	struct list_item *it, *item = NULL;
	list_for_each_entry_safe(item, it, &mylist, links){ // esto recorre las entradas de la lista
		list_del(&(item->links));
		vfree(item);
	}
	/************************/
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	int available_space = BUFFER_LENGTH-1;
	char data[COMMANDS_LENGTH];
	char command_buf[COMMANDS_LENGTH];
	
	struct list_item *item = NULL;

	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;

	if (len > available_space) {
		printk(KERN_INFO "modlist: not enough space!!\n");
		return -ENOSPC;
	}

	/* Transfer data from user to kernel space */
	if (copy_from_user( command_buf, buf, len ))  
		return -EFAULT;

	command_buf[len] = '\0'; /* Add the `\0' */ 
	*off+=len;            /* Update the file pointer */
	
	trace_printk("Modlist: Current command: %s, with %lu length.", command_buf, len);
	
	if(sscanf(command_buf, "add %s", data) == 1) {
		if (strlen(data) > DATA_LENGTH){
			printk(KERN_INFO "modlist: data size too large!!\n");
			return -ENOSPC;
		}  
		item = (struct list_item *) vmalloc(sizeof (struct list_item));
		strcpy(item->data, data);
		list_add_tail(&(item->links), &mylist);
	}
	else if(sscanf(command_buf, "remove %s", data) == 1){
		struct list_item *it = NULL;
		list_for_each_entry_safe(item, it, &mylist, links){
			if(strcmp(item->data, data) == 0){
				list_del(&(item->links));
				vfree(item);
			}
		}
	}
	else if(strncmp(command_buf, "cleanup", len-1) == 0){
		modlist_cleanup();
	}
	else {
		printk(KERN_INFO "ERROR: comando inválido.\n");
	}
	return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
	int nr_bytes;						// return value
	struct list_item *it, *item = NULL; // iterator and temp value
	char rd_buf[TEMP_BUFFER_LENGTH];    // buffer to user
	char data_buf[DATA_LENGTH]; 					// 10 because of the integer's maximum number of char
	char *ptr_rdbuf = rd_buf;			// buffer's ptr

	if ((*off) > 0) /* Tell the application that there is nothing left to read */
		return 0;
	
	// excepcion de ENOSPC para buffer no controlada
	list_for_each_entry_safe(item, it, &mylist, links){
		trace_printk("Current length of read buffer: %ld\n", (ptr_rdbuf - rd_buf));
		trace_printk("Current remaining buffer: %lu\n", (TEMP_BUFFER_LENGTH - (ptr_rdbuf - rd_buf)));
		trace_printk("Current length written: %d\n", sprintf(data_buf, "%s\n", item->data));
		if ((TEMP_BUFFER_LENGTH - (ptr_rdbuf - rd_buf)) < sprintf(data_buf, "%s\n", item->data)){
			trace_printk("Modlist: Current length of read buffer: %ld\n, not enougth space on buffer", (ptr_rdbuf - rd_buf));
			return -ENOSPC;
		}
		ptr_rdbuf += sprintf(ptr_rdbuf, "%s\n", item->data);
	}
	nr_bytes = ptr_rdbuf - rd_buf;
	
	if (len<nr_bytes)
		return -ENOSPC;

	/* Transfer data from the kernel to userspace */  
	if (copy_to_user(buf, rd_buf, nr_bytes))
		return -EINVAL;

	(*off)+=len;  /* Update the file pointer */

	return nr_bytes; 
}

#else

struct list_item {
	int data;
	struct list_head links;
};

void modlist_cleanup ( void ){
	/* cleanup de la lista */
	struct list_item *it, *item = NULL;
	list_for_each_entry_safe(item, it, &mylist, links){ // esto recorre las entradas de la lista
		list_del(&(item->links));
		vfree(item);
	}
	/************************/
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	int available_space = BUFFER_LENGTH-1;
	int num;
	char command_buf[COMMANDS_LENGTH];
	
	struct list_item *item = NULL;

	if ((*off) > 0) /* The application can write in this entry just once !! */
		return 0;

	if (len > available_space) {
		printk(KERN_INFO "modlist: not enough space!!\n");
		return -ENOSPC;
	}

	/* Transfer data from user to kernel space */
	if (copy_from_user( command_buf, buf, len ))  
		return -EFAULT;

	command_buf[len] = '\0'; /* Add the `\0' */ 
	*off+=len;            /* Update the file pointer */
	
	trace_printk("Modlist: Current command: %s", command_buf);
	 
	if(sscanf(command_buf, "add %d", &num) == 1) {
		item = (struct list_item *) vmalloc(sizeof (struct list_item));
		item->data = num;
		list_add_tail(&(item->links), &mylist);
	}
	else if(sscanf(command_buf, "remove %d", &num) == 1){
		struct list_item *it = NULL;
		list_for_each_entry_safe(item, it, &mylist, links){
			if(item->data == num){
				list_del(&(item->links));
				vfree(item);
			}
		}
	}
	else if(strncmp(command_buf, "cleanup", len-1) == 0){
		modlist_cleanup();
	}
	else {
		printk(KERN_INFO "ERROR: comando inválido.\n");
	}
	return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
	int nr_bytes;						// return value
	struct list_item *it, *item = NULL; // iterator and temp value
	char rd_buf[TEMP_BUFFER_LENGTH];    // buffer to user
	char data_buf[10]; 					// 10 because of the integer's maximum number of char
	char *ptr_rdbuf = rd_buf;			// buffer's ptr

	if ((*off) > 0) /* Tell the application that there is nothing left to read */
		return 0;
	
	// excepcion de ENOSPC para buffer no controlada
	list_for_each_entry_safe(item, it, &mylist, links){
		/*trace_printk("Current length of read buffer: %ld\n", (ptr_rdbuf - rd_buf));
		trace_printk("Current len: %lu\n", len);
		trace_printk("Current remaining buffer: %lu\n", (TEMP_BUFFER_LENGTH - (ptr_rdbuf - rd_buf)));
		trace_printk("Current length written: %d\n", sprintf(data_buf, "%d\n", item->data));*/
		if ((TEMP_BUFFER_LENGTH - (ptr_rdbuf - rd_buf)) < sprintf(data_buf, "%d\n", item->data)){
			trace_printk("Modlist: Current length of read buffer: %ld\n, not enougth space on buffer", (ptr_rdbuf - rd_buf));
			return -ENOSPC;
		}
		ptr_rdbuf += sprintf(ptr_rdbuf, "%d\n", item->data);
	}
	nr_bytes = ptr_rdbuf - rd_buf;
	
	if (len<nr_bytes)
		return -ENOSPC;

	/* Transfer data from the kernel to userspace */  
	if (copy_to_user(buf, rd_buf, nr_bytes))
		return -EINVAL;

	(*off)+=len;  /* Update the file pointer */

	return nr_bytes; 
}

#endif

static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,    
};

int init_modlist_module( void )
{
	int ret = 0;
	INIT_LIST_HEAD(&mylist);
	proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
	if (proc_entry == NULL) {
		ret = -ENOMEM;
		list_del(&mylist);
    	printk(KERN_INFO "Modlist: Can't create /proc entry\n");
	} else {
		printk(KERN_INFO "Modlist: Module loaded\n");
	}
	return ret;
}


void exit_modlist_module( void )
{
	modlist_cleanup();
	remove_proc_entry("modlist", NULL); // eliminar la entrada del /proc
	printk(KERN_INFO "Modlist: Module unloaded.\n");
}

module_init( init_modlist_module );
module_exit( exit_modlist_module );
