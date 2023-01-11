#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
/*
	- function list
		- platform_device_register 
		- platform_driver_register
	- variable list
		- struct platform_device
		- struct platform_driver
*/
#include <linux/kobject.h>
/*
	- function list
	- variable list
*/
#include <linux/sysfs.h>
/*
	- function list
	- variable list
*/
#include <linux/gpio.h>
/*
	- function list
		- gpio_request
		- gpio_is_valid
		- gpio_set_value
		- gpio_direction_output, gpio_direction_input
		- gpio_to_irq
		- gpio_free
	- variable list
		-
*/
#include <linux/interrupt.h>
/*
	- function list
		- request_irq
	- variable list
		- IRQF_TRIGGER_RISING
*/

#define GPIO_4 4	
#define GPIO_17	17
int yohda_value = 0;
int yohda_led_toggle = 0;

#define EN_DEBOUNCE

#ifdef EN_DEBOUNCE
// 인터럽트시에 DEBOUNCE가 굉장히 심하다.
// EN_DEBOUNCE feature에 묶여있는 코드를 통해서 인터럽트가 채터링 혹 바운싱 되는 것을 막을 수 있다. 이건 임시 방편이다.
#include <linux/jiffies.h>
extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
#endif

static ssize_t yohda_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("yohfa sysfs - read\n");
	
	return sprintf(buf, "%d", yohda_value);
}

static ssize_t yohda_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	pr_info("yohfa sysfs - write\n");
	gpio_set_value(GPIO_17, (buf[0] == '0' ? 0 : 1));
	
	sscanf(buf, "%d", &yohda_value); 	
	return count;
}

struct kobj_attribute yohda_attr = __ATTR(yohda_value, 0660, yohda_sysfs_show, yohda_sysfs_store);

irqreturn_t yohda_irq_handler(int irq, void *dev_id)
{
#ifdef EN_DEBOUNCE
	unsigned long diff = jiffies - old_jiffie;
	if(diff < 20)
	{
		return IRQ_HANDLED;
	}
	old_jiffie = jiffies;
#endif

	pr_info("GPIO_4 is rising at the moment!");

	yohda_led_toggle = 0x01 ^ yohda_led_toggle;
	gpio_set_value(GPIO_17, 0);
	
	return IRQ_HANDLED;
}

static int yohda_kernel_debug_debugfs_drvier_probe(struct platform_device *pdev) 
{ 
	int err, irq_number;
	struct kobject *kobj_yohda;
	printk("===[%s][L:%d]", __func__, __LINE__); 

	kobj_yohda = kobject_create_and_add("yohda_sysfs", kernel_kobj); 

	if(sysfs_create_file(kobj_yohda, &yohda_attr.attr))
	{
		pr_err("Failed to create sysfs file\n");
		goto err_sysfs;
	}

	err = gpio_is_valid(GPIO_17);
	if(!err)
	{
		pr_err("GPIO%d is not valid\n", GPIO_17);
	}

	err = gpio_request(GPIO_17, "yohda_gpio_output");
	if(err < 0)
	{
		pr_err("err:%d, GPIO%d won`t be used\n", err, GPIO_17);
		goto gpio_err;
	}

	gpio_direction_output(GPIO_17, 0);

	err = gpio_is_valid(GPIO_4);
	if(!err)
	{
		pr_err("GPIO%d is not valid\n", GPIO_4);
	}

	err = gpio_request(GPIO_4, "yohda_gpio_input");
	if(err < 0)
	{
		pr_err("err:%d, GPIO%d won`t be used\n", err, GPIO_4);
		goto gpio_err;
	}
	
	gpio_direction_input(GPIO_4);

#ifndef EN_DEBOUNCE
	// 이게 실제로 먹히지가 않는다. 컴파일시 에러가 나지 않는 거 보면, 존재하는 API인걸로 보이는데 동작은 하지 않는 것 같다.
	if(gpio_set_debounce(GPIO_4, 200) < 0)
	{
		pr_err("Failed to set debounce time of gpio%d", GPIO_4);
	}
#endif

	irq_number = gpio_to_irq(GPIO_4);
	if(irq_number <= 0)
	{
		pr_err("err: %d, Failed to get irq number from gpio number", irq_number);
		goto gpio_err;
	}

	err = request_irq(irq_number, (void *)yohda_irq_handler, IRQF_TRIGGER_RISING, "yohda_irq_handler", NULL);
	if(err)
	{
		pr_err("Failed to register the irq handler on irq number: %d", irq_number);
		goto gpio_err;
	}

	pr_info("yohda device driver probe was successful\n");
	return 0;

gpio_err:
	gpio_free(GPIO_17);
	gpio_free(GPIO_4);

err_sysfs:
	kobject_put(kobj_yohda);
	sysfs_remove_file(kernel_kobj, &yohda_attr.attr);

	return -1;
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
 
