
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

#include <linux/kobject.h>
#include <linux/sysfs.h>

int yohda_value = 0;
/*
// 아래의 struct attribute는 name과 permission 필드가 포함되어 있다. 

struct kobj_attribute {
 struct attribute attr;
 ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
 ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);
};

//우리는 아래 매크로를 사용해서 sysfs를 만들자. 저 name은 변수를 집어넣는것이 아니다.
//저기에 들어가는 글자의 변수를 새로 생성해준다.
#define __ATTR(_name, _mode, _show, _store) {    \
 .attr = {.name = __stringify(_name),    \
   .mode = VERIFY_OCTAL_PERMISSIONS(_mode) },  \
 .show = _show,      \
 .store = _store,      \
}

*/


// 아래 show함수와 store함수의 반환값의 의미가 솔직히 뭔지 잘 모르겠다. 아래의 링크에 뭔가 나와있는거 같긴한데, 정확한 의미를 모르겠다.
// https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s08.html
static ssize_t yohda_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("yohfa sysfs - read\n");
	// int integer = 123;
	// sprintf(buf, "integer : (decimal) %d (octal) %o \n", integer, integer);
 	// printf("%s \n", buf);
	// integer : <decimal> 123 <octal> 173
	return sprintf(buf, "%d", yohda_value);
}

static ssize_t yohda_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	pr_info("yohfa sysfs - write\n");
	// char buffer[256]="name:홍길동 num:12 age:20";
	// scanf(buffer,"name:%s num:%d age:%d",name,&num,&age);
	// printf("이름:%s 번호:%d 나이:%d \n",name,num,age);
	// 출력 결과 - "이름:홍길동 번호:12 나이:20"	
	sscanf(buf, "%d", &yohda_value); 	
	return count;
}

struct kobj_attribute yohda_attr = __ATTR(yohda_value, 0660, yohda_sysfs_show, yohda_sysfs_store);
static int yohda_kernel_debug_debugfs_drvier_probe(struct platform_device *pdev) 
{ 
	struct kobject *kobj_yohda;
	printk("===[%s][L:%d]", __func__, __LINE__); 

	// 첫 번째 인자는 생성할 폴더 이름이고, 두 번째 인자는 부모 폴더를 지정한다.
	// 그래서 부모 폴더 아래에 생성하게 만든다. NULL일 경우, /sys/yohda_sysfs/ 폴더를 생성한다.
	// 두 번째 인자인 kernel_kobj, firmware_kobj, fs_kobj는 'linux/kobject.h'에 생성되어 있는 전역변수이다.
	// 아래의 코드를 통해서 /sys/kernel/yohda_sysfs 폴더를 생성한다.
	kobj_yohda = kobject_create_and_add("yohda_sysfs", kernel_kobj); 
	//kobj_yohda = kobject_create_and_add("yohda_sysfs", firmware_kobj); // /sys/firmware/yohda_sysfs 폴더를 생성. 
	//kobj_yohda = kobject_create_and_add("yohda_sysfs", fs_kobj); // /sys/fs/yohda_sysfs 폴더를 생성.

	if(sysfs_create_file(kobj_yohda, &yohda_attr.attr))
	{
		pr_err("Failed to create sysfs file\n");
		goto err_sysfs;
	}
	
	return 0;

err_sysfs:
	// 아래의 함수를 호출함으로써 sysfs 사용한 메모리를 해제한다.	
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
 
