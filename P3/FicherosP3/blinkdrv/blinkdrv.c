/*
 * Simple driver for the Blinkstick Strip USB device (v1.0)
 *
 * Copyright (C) 2018 Juan Carlos Saez (jcsaezal@ucm.es)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the sample driver found in the
 * Linux kernel sources  (drivers/usb/usb-skeleton.c) 
 * 
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");

/* Get a minor range for your devices from the usb maintainer */
#define USB_BLINK_MINOR_BASE	0 

/* Structure to hold all of our device specific stuff */
struct usb_blink {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
};
#define to_blink_dev(d) container_of(d, struct usb_blink, kref)

static struct usb_driver blink_driver;

/* 
 * Free up the usb_blink structure and
 * decrement the usage count associated with the usb device 
 */
static void blink_delete(struct kref *kref)
{
	struct usb_blink *dev = to_blink_dev(kref);

	usb_put_dev(dev->udev);
	vfree(dev);
}

/* Called when a user program invokes the open() system call on the device */
static int blink_open(struct inode *inode, struct file *file)
{
	struct usb_blink *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	
	/* Obtain reference to USB interface from minor number */
	interface = usb_find_interface(&blink_driver, subminor);
	if (!interface) {
		pr_err("%s - error, can't find device for minor %d\n",
			__func__, subminor);
		return -ENODEV;
	}

	/* Obtain driver data associated with the USB interface */
	dev = usb_get_intfdata(interface);
	if (!dev)
		return -ENODEV;

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

	return retval;
}

/* Called when a user program invokes the close() system call on the device */
static int blink_release(struct inode *inode, struct file *file)
{
	struct usb_blink *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, blink_delete);
	return 0;
}


#define NR_LEDS 8
#define NR_BYTES_BLINK_MSG 6
#define MAX_LEN 88



#define NR_SAMPLE_COLORS 4

unsigned int sample_colors=0x000000;

/* Called when a user program invokes the write() system call on the device */
static ssize_t blink_write(struct file *file, const char *user_buffer,
			  size_t len, loff_t *off)
{
	struct usb_blink *dev=file->private_data;
	int retval = 0;
	int i=0;
	unsigned int color;
	int pos;
	unsigned char *message;
	char *kbuf = NULL;
	char *it_kbuf, *ptr_kbuf, *ptr_message;
	
	if ((*off)>0){
		return 0;
	}
	
	if (len > MAX_LEN){
		printk(KERN_INFO "Blinkstick: only 8 leds needed.\n");
		return -ENOSPC;
	}
	
	/* size allocation */
	message=kmalloc(NR_BYTES_BLINK_MSG*NR_BYTES_BLINK_MSG,GFP_DMA);
	kbuf=kmalloc(len+1, GFP_DMA);
	// printk(KERN_INFO "%s\n", kbuf);
	
	/* copy from user */
	if (copy_from_user(kbuf, user_buffer, len)){
		retval =  -EFAULT;
		goto out_error;
	} 
	kbuf[len]='\0';
	
	/* zero fill */
	memset(message,0,NR_BYTES_BLINK_MSG*NR_BYTES_BLINK_MSG);
	
	/* filling up */
	for (i=0; i<NR_LEDS; i++){
		message[0+i*NR_BYTES_BLINK_MSG]='\x05';
		message[1+i*NR_BYTES_BLINK_MSG]=0x00; 
	} 
	
	/* parsing kbuf */
	if (strlen(kbuf) > 1){
		it_kbuf = kbuf;
		while(it_kbuf != NULL){
			ptr_kbuf = strsep(&it_kbuf, ",");
			if (sscanf(ptr_kbuf, "%i:%x", &pos, &color) == 2){
				message[pos*NR_BYTES_BLINK_MSG+2]=pos;
				message[pos*NR_BYTES_BLINK_MSG+3]=((color>>16) & 0xff);
			 	message[pos*NR_BYTES_BLINK_MSG+4]=((color>>8) & 0xff);
			 	message[pos*NR_BYTES_BLINK_MSG+5]=(color & 0xff);
			}
			else{
				printk(KERN_ALERT "Blinkstick: wrong format --> %s.", ptr_kbuf);
				retval = -EINVAL;
				goto out_error;
			}
		}
	}
	
	else {
		for (i=0;i<NR_LEDS;i++){
			message[i*NR_BYTES_BLINK_MSG+2]=i;
			message[i*NR_BYTES_BLINK_MSG+3]=((sample_colors>>16) & 0xff);
		 	message[i*NR_BYTES_BLINK_MSG+4]=((sample_colors>>8) & 0xff);
		 	message[i*NR_BYTES_BLINK_MSG+5]=(sample_colors & 0xff);
		}
	}
	
	ptr_message=message;
	for (i=0;i<NR_LEDS;i++){
	
		/* 
		 * Send message (URB) to the Blinkstick device 
		 * and wait for the operation to complete 
		 */
		retval=usb_control_msg(dev->udev,	
			 usb_sndctrlpipe(dev->udev,00), /* Specify endpoint #0 */
			 USB_REQ_SET_CONFIGURATION, 
			 USB_DIR_OUT| USB_TYPE_CLASS | USB_RECIP_DEVICE,
			 0x5,	/* wValue */
			 0, 	/* wIndex=Endpoint # */
			 ptr_message,	/* Pointer to the message */ 
			 NR_BYTES_BLINK_MSG, /* message's size in bytes */
			 0);		

		if (retval<0){
			printk(KERN_ALERT "Executed with retval=%d\n",retval);
			goto out_error;		
		}
		
		ptr_message+=NR_BYTES_BLINK_MSG;
	}

	kfree(message);
	kfree(kbuf);
	(*off)+=len;
	return len;

out_error:
	kfree(message);
	kfree(kbuf);
	return retval;	
}


/*
 * Operations associated with the character device 
 * exposed by driver
 * 
 */
static const struct file_operations blink_fops = {
	.owner =	THIS_MODULE,
	.write =	blink_write,	 	/* write() operation on the file */
	.open =		blink_open,			/* open() operation on the file */
	.release =	blink_release, 		/* close() operation on the file */
};

/* 
 * Return permissions and pattern enabling udev 
 * to create device file names under /dev
 * 
 * For each blinkstick connected device a character device file
 * named /dev/usb/blinkstick<N> will be created automatically  
 */
char* set_device_permissions(struct device *dev, umode_t *mode) 
{
	if (mode)
		(*mode)=0666; /* RW permissions */
 	return kasprintf(GFP_KERNEL, "usb/%s", dev_name(dev)); /* Return formatted string */
}


/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver blink_class = {
	.name =		"blinkstick%d",  /* Pattern used to create device files */	
	.devnode=	set_device_permissions,	
	.fops =		&blink_fops,
	.minor_base =	USB_BLINK_MINOR_BASE,
};

/*
 * Invoked when the USB core detects a new
 * blinkstick device connected to the system.
 */
static int blink_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_blink *dev;
	int retval = -ENOMEM;

	/*
 	 * Allocate memory for a usb_blink structure.
	 * This structure represents the device state.
	 * The driver assigns a separate structure to each blinkstick device
 	 *
	 */
	dev = vmalloc(sizeof(struct usb_blink));

	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}

	/* Initialize the various fields in the usb_blink structure */
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &blink_class);
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */	
	dev_info(&interface->dev,
		 "Blinkstick device now attached to blinkstick-%d",
		 interface->minor);
	return 0;

error:
	if (dev)
		/* this frees up allocated memory */
		kref_put(&dev->kref, blink_delete);
	return retval;
}

/*
 * Invoked when a blinkstick device is 
 * disconnected from the system.
 */
static void blink_disconnect(struct usb_interface *interface)
{
	struct usb_blink *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &blink_class);

	/* prevent more I/O from starting */
	dev->interface = NULL;

	/* decrement our usage count */
	kref_put(&dev->kref, blink_delete);

	dev_info(&interface->dev, "Blinkstick device #%d has been disconnected", minor);
}

/* Define these values to match your devices */
#define BLINKSTICK_VENDOR_ID	0X20A0
#define BLINKSTICK_PRODUCT_ID	0X41E5

/* table of devices that work with this driver */
static const struct usb_device_id blink_table[] = {
	{ USB_DEVICE(BLINKSTICK_VENDOR_ID,  BLINKSTICK_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, blink_table);

static struct usb_driver blink_driver = {
	.name =		"blinkstick",
	.probe =	blink_probe,
	.disconnect =	blink_disconnect,
	.id_table =	blink_table,
};

/* Module initialization */
int blinkdrv_module_init(void)
{
   return usb_register(&blink_driver);
}

/* Module cleanup function */
void blinkdrv_module_cleanup(void)
{
  usb_deregister(&blink_driver);
}

module_init(blinkdrv_module_init);
module_exit(blinkdrv_module_cleanup);

