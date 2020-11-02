#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0

#define COMMAND_LENGTH 256

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Modleds");
MODULE_AUTHOR("Xukai Chen, Daniel Alfaro");

static struct proc_dir_entry *proc_entry;
struct tty_driver* kbd_driver= NULL;


/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

static ssize_t modleds_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
	int available_space = COMMAND_LENGTH;
	char command_buf[COMMAND_LENGTH+1];
	unsigned int num;
	
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
	
	if(sscanf(command_buf, "add 0x%x", &num) == 1) {
		num &= 0x7;
		set_leds(kbd_driver, num); 
	} else 
		return -EINVAL;   
	
	*off += len;
	return len;
}


static const struct file_operations proc_entry_fops = {
	.write = modleds_write,
};

static int __init modleds_init(void)
{	
	int ret = 0;
	
	proc_entry = proc_create( "modleds", 0666, NULL, &proc_entry_fops);
	
	if (proc_entry == NULL) {
		ret = -ENOMEM;
    	printk(KERN_INFO "Modleds: Can't create /proc entry\n");
	} else {
		kbd_driver = get_kbd_driver_handler();
		set_leds(kbd_driver,ALL_LEDS_ON); 
		printk(KERN_INFO "Modleds: Module loaded\n");
	}
	return ret;
}

static void __exit modleds_exit(void){
    set_leds(kbd_driver,ALL_LEDS_OFF); 
    remove_proc_entry("modleds", NULL);
    printk(KERN_INFO "Modleds: Module unloaded\n");
}

module_init(modleds_init);
module_exit(modleds_exit);

