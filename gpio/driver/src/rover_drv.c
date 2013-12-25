#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "rover_drv_gpio.h"



//#define ROVER_GPIO_MSG_NUM	21
#define ROVER_GPIO_MSG_NUM	114


typedef struct _ROVER_DRV
{
   int 	irq_num;		//irq num associated to gpio
   int 	irq_enabled;		//has irq been enabled as GPIO INTR
   int 	irq_state;  		//state of the irq
   int  irq_count;		//service counts of irq (should be equivalent to /proc/interrupts)
   int  irq_serviced;		//has irq been serviced; what stage is it in
   int 	gpio_num;		//gpio number to associate with pending irq
   int 	gpio_enabled;		//gpio is valid and enabled ("taken")

   dev_t  			rover_drv_dev_num;	//major,minor device number association to driver	
   struct cdev  		rover_drv_cdev;		//character device 
   struct class 	       *rover_drv_class;	//character device class
   struct tasklet_struct 	rover_drv_tasklet;	//tasklet for driver
   wait_queue_head_t 		rover_drv_wait_queue;	//wait queue for driver
   spinlock_t 			rover_drv_lock;		//lock for interrupt processing		
} ROVER_DRV, *PROVER_DRV;


//static atomic_t gOpenCnt = ATOMIC_INIT(0);
static volatile PROVER_DRV gPRoverDrv = NULL;
static char gRoverBanner[] __initdata = KERN_INFO "Platform Gpio Driver: 1.00\n";

/*
 * Function: rover_drv_process_tasklet
 * Bottom half interrupt processing as a tasklet
 * Responsible for second half processing of interrupt
 * Will signal driver wait queue pending on event
 */
static void rover_drv_process_tasklet(unsigned long data)
{
   PROVER_DRV pRoverData = (PROVER_DRV)data;
   spin_lock(&pRoverData->rover_drv_lock);
   if(pRoverData->irq_state)
   {
      pRoverData->irq_serviced = true;
   }
   wake_up_interruptible(&pRoverData->rover_drv_wait_queue);
   spin_unlock(&pRoverData->rover_drv_lock);
}


/*
 * Function: rover_drv_setup_interrupt
 * Sets up configuring GPIO
 * Configures GPIO as IRQ
 */
static irqreturn_t rover_drv_gpio_irq(int irq, void *dev_id)
{
   irqreturn_t ret  = IRQ_NONE;
   PROVER_DRV pRoverDrv = (PROVER_DRV)dev_id; 
   spin_lock(&pRoverDrv->rover_drv_lock);
   if(pRoverDrv->irq_enabled)
   {
	pRoverDrv->irq_count  = ++pRoverDrv->irq_count;
 	pRoverDrv->irq_state  = true;
        ret = IRQ_HANDLED;
   }
   tasklet_schedule(&pRoverDrv->rover_drv_tasklet);
   spin_unlock(&pRoverDrv->rover_drv_lock);
   return ret;
}

/*
 * Function: rover_drv_setup_interrupt
 * Sets up configuring GPIO
 * Configures GPIO as IRQ
 */
int rover_drv_setup_interrupt(PROVER_DRV pRoverDrv)
{
   int err = EROVER_GPIO_SETUP;
 
   if(!gpio_is_valid(ROVER_GPIO_MSG_NUM))
   {
      printk(KERN_INFO "GPIO is Invalid:%s %d\n",ROVER_DRV_NAME, ROVER_GPIO_MSG_NUM);
      err = EROVER_GPIO_INVALID;
      goto setup_default_err;
   }

   pRoverDrv->gpio_num = ROVER_GPIO_MSG_NUM;
   err = gpio_request(ROVER_GPIO_MSG_NUM, "AVR MSG INTR");
   if(err)
   {
      printk(KERN_INFO "Unable to retrieve GPIO:%s %d\n",ROVER_DRV_NAME, ROVER_GPIO_MSG_NUM);
      err = EROVER_GPIO_REQUEST;
      goto setup_default_err;
   }

   pRoverDrv->gpio_enabled = true;
   pRoverDrv->irq_num = gpio_to_irq(ROVER_GPIO_MSG_NUM);
   if(pRoverDrv->irq_num >= 0)
   {
      err = request_irq(pRoverDrv->irq_num, rover_drv_gpio_irq, IRQF_TRIGGER_RISING, ROVER_DRV_NAME, pRoverDrv);
      if(err)
      {
         printk(KERN_INFO "Unable to retrieve IRQ:%s %d\n",ROVER_DRV_NAME, pRoverDrv->irq_num);
         err = EROVER_INTR_ACCESS;
         goto setup_rover_gpio_free;
      }
      pRoverDrv->irq_enabled = true;
   }

   gpio_direction_input(ROVER_GPIO_MSG_NUM);
   return err;

setup_rover_gpio_free:
   gpio_free(ROVER_GPIO_MSG_NUM);

setup_default_err:
   return err;
}


/*
 * Function: rover_drv_open
 * Called when user space client is opening a handle to the driver
 * Responsible for adding any client specific data 
 */
static int rover_drv_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Rover Module open:%s Cnt:%d\n",ROVER_DRV_NAME, module_refcount(THIS_MODULE));
    return 0;
}

/*
 * Function: rover_drv_close
 * Called when user space client is closing handle to file
 * Responsible for removing any client specific data 
 */
static int rover_drv_close(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Rover Module close:%s Cnt:%d\n",ROVER_DRV_NAME, module_refcount(THIS_MODULE));
    return 0;
}

/*
 * Function: rover_drv_ioctl
 * Entry point for performing ioctls to rover_drv
 * Responsible for handling ioctls from user space 
 * Current function is handling of GPIO
 */
long rover_drv_ioctl( struct file *file, unsigned int cmd, unsigned long arg )
{
   int err = 0;
   int value = 0;
   int irq_serviced_state = 0;
   unsigned long flags;

   if(!gPRoverDrv)
   {
      err = -ENOMEM;
      goto rover_done;    
   }

   switch(cmd)
   {
      case ROVER_GPIO_PEND_INTR:
         spin_lock_irqsave(&gPRoverDrv->rover_drv_lock, flags);
   	 irq_serviced_state = gPRoverDrv->irq_serviced;
         spin_unlock_irqrestore(&gPRoverDrv->rover_drv_lock, flags);
	 //pending on interrupt being serviced; will unblock once bottom half wakes up wait queue 
   	 if(!irq_serviced_state)
         {
            wait_event_interruptible(gPRoverDrv->rover_drv_wait_queue, gPRoverDrv->irq_serviced);
         }
         //if irq_seviced is already set then we have already processed the interrupt
         //client does not need to pend on INTR
	 spin_lock_irqsave(&gPRoverDrv->rover_drv_lock, flags);
         gPRoverDrv->irq_serviced  = false;
   	 gPRoverDrv->irq_state     = false;
         spin_unlock_irqrestore(&gPRoverDrv->rover_drv_lock, flags);
         break;
      case ROVER_GPIO_PERFORM_INTR:
         //drive low
	 printk(KERN_INFO "DRV:%s Err:%d, Value:%d IRQ EN:%d\n",ROVER_DRV_NAME, err, value, gPRoverDrv->irq_enabled);
         err = gpio_direction_output(ROVER_GPIO_MSG_NUM, 0);
         gpio_set_value(ROVER_GPIO_MSG_NUM, 0);
         value = gpio_get_value(ROVER_GPIO_MSG_NUM);
	 printk(KERN_INFO "DRV:%s Err:%d, Value:%d IRQ State:%d\n",ROVER_DRV_NAME, err, value, gPRoverDrv->irq_state);
         //drive high
	 gpio_set_value(ROVER_GPIO_MSG_NUM, 1);
   	 value = gpio_get_value(ROVER_GPIO_MSG_NUM);
	 printk(KERN_INFO "DRV:%s Err:%d, Value:%d IRQ State:%d\n",ROVER_DRV_NAME, err, value, gPRoverDrv->irq_state);
         break;
      case ROVER_GPIO_DUMP_INFO:
	 printk(KERN_INFO "DRV:%s GPIO NUM:%d GPIO EN:%d\n",ROVER_DRV_NAME, gPRoverDrv->gpio_num, gPRoverDrv->gpio_enabled);
	 printk(KERN_INFO "DRV:%s IRQ State:%d\n",ROVER_DRV_NAME, gPRoverDrv->irq_state);
	 printk(KERN_INFO "DRV:%s IRQ Count:%d\n",ROVER_DRV_NAME, gPRoverDrv->irq_count);
	 printk(KERN_INFO "DRV:%s IRQ Serviced:%d\n",ROVER_DRV_NAME, gPRoverDrv->irq_serviced);
	 printk(KERN_INFO "DRV:%s IRQ Num:%d, IRQ EN:%d\n",ROVER_DRV_NAME, gPRoverDrv->irq_num, gPRoverDrv->irq_enabled);
	 break;
      default:
	err = ENOIOCTLCMD;
	break; 
   }

rover_done:
   return err;
}


/*
 * Data: rover_fops
 * Set up entry points for module
 */
static const struct file_operations rover_drv_fops = {
    .owner   		= THIS_MODULE,
    .open    		= rover_drv_open,
    .release 		= rover_drv_close,
    .unlocked_ioctl 	= rover_drv_ioctl,
};


/*
 * Function: rover_drv_init
 * Called when exiting the loading the module
 * Responsible for initializing the character device 
 * Setting up GPIO as interupt and setting up resources
 */
static int __init rover_drv_init(void)
{
    int err = 0;
    printk(gRoverBanner);

    gPRoverDrv = kzalloc(sizeof(ROVER_DRV), GFP_KERNEL);
    memset(gPRoverDrv, 0x00, sizeof(ROVER_DRV));
    if(!gPRoverDrv) 
    {
       err = -ENOMEM; 
       goto rover_done;
    }

    //dynamically alloc character driver region
    //not using statically assigned and registration
    //starting at minor number base  0
    if (( err = alloc_chrdev_region( &gPRoverDrv->rover_drv_dev_num, 0, 1, ROVER_DRV_NAME )) < 0 )
    {
       printk( KERN_WARNING "%s: alloc_chrdev_region failed; err: %d\n", __func__, err );
       goto rover_done;
    }

    cdev_init(&gPRoverDrv->rover_drv_cdev, &rover_drv_fops);
    gPRoverDrv->rover_drv_cdev.owner = THIS_MODULE;
    //add the device
    if ( (err = cdev_add(&gPRoverDrv->rover_drv_cdev, gPRoverDrv->rover_drv_dev_num, 1)) != 0)
    {
       printk( KERN_WARNING "%s: cdev_add failed: %d\n", __func__, err );
       goto rover_unregister;
    }
    //create the class for mdev to create /dev entry
    //assuming runtime creating and not static entry
    gPRoverDrv->rover_drv_class = class_create( THIS_MODULE, ROVER_DRV_NAME );
    if( IS_ERR(gPRoverDrv->rover_drv_class) )
    {
       err = EROVER_CREATE_CLASS_DEV;
       printk( KERN_WARNING "%s: Unable to create class\n", __func__ );
       goto rover_cdev_del;
    }
    device_create(gPRoverDrv->rover_drv_class, NULL, gPRoverDrv->rover_drv_dev_num, NULL, ROVER_DRV_NAME);
    
    //setup gpio as interrupt and initialize tasklet for bottom half interrupt processing
    spin_lock_init(&gPRoverDrv->rover_drv_lock);
    tasklet_init(&gPRoverDrv->rover_drv_tasklet, rover_drv_process_tasklet,(unsigned long)gPRoverDrv);
    init_waitqueue_head(&gPRoverDrv->rover_drv_wait_queue);
    err = rover_drv_setup_interrupt(gPRoverDrv);
    goto rover_done;

rover_cdev_del:
    cdev_del(&gPRoverDrv->rover_drv_cdev);

rover_unregister:
    unregister_chrdev_region(gPRoverDrv->rover_drv_dev_num, 1 );

rover_done:
    return err;
}


/*
 * Function: rover_drv_exit
 * Called when exiting the unloading the module
 * Close all resources and memory
 */
static void __exit rover_drv_exit(void)
{
    device_destroy(gPRoverDrv->rover_drv_class, gPRoverDrv->rover_drv_dev_num);
    class_destroy( gPRoverDrv->rover_drv_class );

    cdev_del(&gPRoverDrv->rover_drv_cdev);
    unregister_chrdev_region(gPRoverDrv->rover_drv_dev_num, 1);

    tasklet_kill(&gPRoverDrv->rover_drv_tasklet);

    if(gPRoverDrv->gpio_enabled) 
	gpio_free(ROVER_GPIO_MSG_NUM);
    if(gPRoverDrv->irq_enabled)
	free_irq(gPRoverDrv->irq_num, gPRoverDrv);

    kfree(gPRoverDrv);
    printk(KERN_INFO "Rover Module exit\n");
    return;
}


module_init(rover_drv_init);
module_exit(rover_drv_exit);

MODULE_AUTHOR("Ari Kamlani");
MODULE_DESCRIPTION( "ROVER DRV GPIO Driver" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION( "1.0" );
