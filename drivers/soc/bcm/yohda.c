
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/init.h> 
#include <linux/memblock.h> 
#include <linux/slab.h> 
#include <linux/of.h> 
#include <linux/of_address.h> 
#include <linux/cpu.h> 
#include <linux/delay.h> 
#include <asm/setup.h> 
#include <linux/input.h> 
#include <linux/debugfs.h> 
#include <linux/timer.h> 
#include <linux/workqueue.h> 
#include <linux/mutex.h> 
#include <linux/slub_def.h> 
#include <linux/uaccess.h> 
#include <asm/memory.h> 

#define CONFIG_YOHDA_IRQ

#ifdef CONFIG_YOHDA_IRQ
	#include <linux/gpio.h>
	#include <linux/interrupt.h>
#endif

#ifdef CONFIG_YOHDA_IRQ
static void yohda_get_irq_desc(int irq)
{
	struct irqaction *action;
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	if(!desc){
		pr_err("invalid desc at %s line: %d caller:%pS\n", __func__, __LINE__, (void*)__builtin_return_address(0));
		return ;
	}

	action = desc->action;

	if(!action){
		pr_err("invalid action at %s line: %d caller:%pS\n", __func__, __LINE__, (void*)__builtin_return_address(0));
		return ;
	}

	printk("irq num:%d name:%8s\n", action->irq, action->name);
	printk("dev_id:0x%x\n", (unsigned int)action->dev_id);
	printk("irq depth:%d\n", desc->depth);
	
	if(action->handler){
		printk("interrupt handler: %pF \n", action->handler);
	}
}

static irqreturn_t yohda_irq_handler(int irq, void *data)
{
	printk("===[YOHDA][%s][L:%d]===", __func__, __LINE__);

	return IRQ_HANDLED;
}
#endif

static int yohda_kernel_debug_debugfs_drvier_probe(struct platform_device *pdev) 
{ 
	printk("===[%s][L:%d]", __func__, __LINE__); 
#ifdef CONFIG_YOHDA_IRQ
	int irq;

	if(gpio_request(4, "yohda_irq"))
		return -EINVAL;

	gpio_direction_input(4);

	irq = gpio_to_irq(4);
	if(irq <= 0)
	{
		pr_err("[YOHDA] failed to set gpio to irq. check the direction of the gpio. If the direction is output, failed to set. at %s line: %d caller:%pS\n", __func__, __LINE__, (void*)__builtin_return_address(0));
	}
	if(request_irq(irq, yohda_irq_handler, IRQF_TRIGGER_FALLING, "yohda_irq_handler", &(pdev->dev)) != 0)
	{
		pr_err("[YOHDA] failed to request irq at %s line: %d caller:%pS\n", __func__, __LINE__, (void*)__builtin_return_address(0));

		return -EBUSY;
	}	

	yohda_get_irq_desc(irq);
#endif

	return 0;
} 

static struct platform_device yohda_device = {
	.name = "yohda",
	.id = -1,
};

static struct platform_driver yohda_kernel_debug_debugfs_driver = { 
	.probe = yohda_kernel_debug_debugfs_drvier_probe, 
	.driver = { 
		.owner = THIS_MODULE,
		.name = "yohda",
	},
}; 


static int __init yohda_kernel_debug_debugfs_init(void) 
{
	int err = 1;

	printk("===[%s][L:%d]===\n", __func__, __LINE__);
	err = platform_driver_register(&yohda_kernel_debug_debugfs_driver); 
	if(!err)
		err = platform_device_register(&yohda_device);

	return err;
}

late_initcall(yohda_kernel_debug_debugfs_init); // late_initcall은 *_initcall 중에서 우선순위가 가장 낮다. 
 
