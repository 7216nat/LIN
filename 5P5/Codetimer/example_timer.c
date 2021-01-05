#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/ctype.h>
#include <linux/slab.h>

char code_format[9] = "aA00";
struct timer_list my_timer; /* Structure that describes the kernel timer */


/* Function invoked when timer expires (fires) */
static void fire_timer(unsigned long data)
{
	int len = strlen(code_format), i;
    unsigned int rand = get_random_int();
    static char flag[5];
    flag[len] = '\0'; 
    
    for (i = 0 ; i < len; i++){
        if (isalpha(code_format[i])) {
            flag[i] = (((rand >> i*8) & 0xff)%26)+code_format[i];
        }
        else flag[i] = (((rand >> i*8) & 0xff)%10)+code_format[i];
    }
    
    printk(KERN_INFO "Generated code: %s\n", flag);
    //kfree(flag);
        /* Re-activate the timer one second from now */
	mod_timer( &(my_timer), jiffies + HZ); 
}

int init_timer_module( void )
{
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


void cleanup_timer_module( void ){
  /* Wait until completion of the timer function (if it's currently running) and delete timer */
  del_timer_sync(&my_timer);
}

module_init( init_timer_module );
module_exit( cleanup_timer_module );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("timermod Module");
MODULE_AUTHOR("Juan Carlos Saez");
