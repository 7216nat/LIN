#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/ftrace.h>
#include <linux/list.h>
#include <asm/spinlock.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modlist Kernel Module - FDI-UCM");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro");

#define MAX_SIZE      		12
#define TEMP_BUFFER_LENGTH  4
#define COMMANDS_LENGTH		100	

static struct proc_dir_entry *proc_entry;
struct list_head mylist; // nodo fantasma

struct list_item {
	int data;
	struct list_head links;
};

// Commit P4
static spinlock_t mtx;

void modlist_cleanup ( void ){
	/* cleanup de la lista */
	struct list_item *it, *item = NULL;
	// commit P4 
	spin_lock(&mtx);
	list_for_each_entry_safe(item, it, &mylist, links){ // esto recorre las entradas de la lista
		list_del(&(item->links));
		vfree(item);
	}
	spin_unlock(&mtx);
	/************************/
}

static ssize_t modlist_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {

	int available_space = COMMANDS_LENGTH-1;
	char command_buf[COMMANDS_LENGTH];
	int num;
	
	struct list_item *item = NULL;

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
	 
	if(sscanf(command_buf, "add %d", &num) == 1) {
		item = (struct list_item *) vmalloc(sizeof (struct list_item));
		if (!item)
			return -ENOMEM;
			
		// commit P4 
		spin_lock(&mtx);
		item->data = num;
		list_add_tail(&(item->links), &mylist);
		spin_unlock(&mtx);
	}
	else if(sscanf(command_buf, "remove %d", &num) == 1){
		struct list_item *it = NULL;
		// commit P4 
		spin_lock(&mtx);
		list_for_each_entry_safe(item, it, &mylist, links){
			if(item->data == num){
				list_del(&(item->links));
				vfree(item);
			}
		}
		spin_unlock(&mtx);
	}
	else if(strncmp(command_buf, "cleanup\n", len) == 0){
		modlist_cleanup();
	}
	else {
		printk(KERN_INFO "ERROR: comando inválido.\n");
	}
	return len;
}

static ssize_t modlist_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
  
	int nr_bytes, total_bytes = 0, wt_bytes = 0, to_wt = 0;			// nr_bytes: aux, total_bytes: return value, wt_bytes: bytes to write, to_wt: max bytes can write on buffer 
	struct list_item *it, *item = NULL; 							// iterator and temp value
	char rd_buf[TEMP_BUFFER_LENGTH];								// temp buffer
	char data_buf[MAX_SIZE];										// aux buffer, number to char 
	char *ptr_rdbuf = rd_buf, *ptr_buf = buf, *ptr_data = data_buf; // as names indicate
	
	if ((*off) > 0) /* Tell the application that there is nothing left to read */
		return 0;
	
	// commit P4 
	spin_lock(&mtx);
	list_for_each_entry_safe(item, it, &mylist, links){
		
		nr_bytes = ptr_rdbuf - rd_buf;
		to_wt = TEMP_BUFFER_LENGTH - nr_bytes;
		
		if ( to_wt < sprintf(ptr_data, "%d\n", item->data)){
			trace_printk("Modlist: Looping on buffer: %d out %d.\n", nr_bytes, TEMP_BUFFER_LENGTH);
			do {
				wt_bytes = snprintf(ptr_rdbuf, to_wt, ptr_data);
				if (copy_to_user(ptr_buf, rd_buf, TEMP_BUFFER_LENGTH))
					return -EINVAL;
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
	spin_unlock(&mtx);
	total_bytes += ptr_rdbuf - rd_buf;
		
	if (len<total_bytes)
		return -ENOSPC;

	/* Transfer data from the kernel to userspace */  
	if (copy_to_user(ptr_buf, rd_buf, (ptr_rdbuf - rd_buf)))
		return -EINVAL;

	(*off)+=total_bytes;  /* Update the file pointer */

	return total_bytes; 
}


static const struct file_operations proc_entry_fops = {
    .read = modlist_read,
    .write = modlist_write,    
};

int init_modlist_module( void )
{
	int ret = 0;
		
	proc_entry = proc_create( "modlist", 0666, NULL, &proc_entry_fops);
	if (proc_entry == NULL) {
		ret = -ENOMEM;
    	printk(KERN_INFO "Modlist: Can't create /proc entry\n");
	} else {
		// commit P4
		spin_lock_init(&mtx);
		
		INIT_LIST_HEAD(&mylist);
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
