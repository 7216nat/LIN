#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <asm-generic/errno.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0x0

SYSCALL_DEFINE1(ledctl, unsigned int, leds){
	struct tty_driver* kbd_driver = vc_cons[fg_console].d->port.tty->driver;
	mask &= ALL_LEDS_ON;
	return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

