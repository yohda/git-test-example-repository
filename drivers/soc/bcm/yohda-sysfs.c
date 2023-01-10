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

static int yohda_kernel_debug_debugfs_drvier_probe(struct platform_device *pdev) 
{ 
	printk("===[%s][L:%d]", __func__, __LINE__); 

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
 
