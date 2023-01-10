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

#include <linux/gpio.h>

#define GPIO_17	17

static int yohda_kernel_debug_debugfs_drvier_probe(struct platform_device *pdev) 
{ 
	int err;
	printk("===[%s][L:%d]", __func__, __LINE__); 

	// gpio_is_valid() 함수는 인자로 들어가는 숫자가 실제 GPIO 핀인지 확인해준다.
	// 이게 무슨말이냐면, 실제 물리핀 번호와 GPIO 번호는 다를 수 있다.
	// 라즈베리파이 3 모델 B의 경우, 실제 물리적인 7번핀에는 GPIO 4번으로 할당되어 있다.
	// 물론, kernel 사용하는 모든 gpio_xxxx 함수들에는 실제 물리핀번호가 아닌 GPIO로 할당된 번호를 사용해야 한다.
	// 반환 : valid 하지 않으면, false. valid 하면 true.
	err = gpio_is_valid(GPIO_17);
	if(!err)
	{
		pr_err("GPIO%d is not valid\n", GPIO_17);
	}

	// gpio_request() 함수를 쓰는 이유는 이 파일에서만 해당 GPIO를 독점하기 위해서다.
	// gpio_request() 함수를 사용하면 다른 사용자가 shell에서 해당 gpio를 export 하려고 하면, device busy error를 출력한다. 다른 드라이버나 다른 프로세스에서도 해당 gpio를 사용할 수 없다. 오직 여기서만 사용이 가능하다.
	// 즉, gpio를 여기서만 독점해야 할 경우, gpio_request()를 사용해야 한다.
	// 반환 : 성공 시 0 반환. 실패 시 음수 에러 코드 반환. 
	err = gpio_request(GPIO_17, "yohda_gpio_output");
	if(err < 0)
	{
		pr_err("err:%d, GPIO%d won`t be used\n", err, GPIO_17);
		goto gpio_err;
	}

	// 첫 번재 인자로 주어진 GPIO를 output으로 설정한다.
	// 두 번째 인자는 초기값을 1로 할지, 0으로 할지 설정할 수 있다.	
	gpio_direction_output(GPIO_17, 0);

	return 0;

gpio_err:
	gpio_free(GPIO_17);
	
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
 
