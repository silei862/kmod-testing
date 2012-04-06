/*********************************************************
 * File:		ioport_main.c
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-1-24
 * Description:	ioport字符设备主内核模块
 *********************************************************/

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include "ioport_main.h"
#include "ioport_fops.h"

/*字符设备驱动接口结构*/
static struct file_operations ioport_fops = {
	.owner			= THIS_MODULE,
	.open			= ioport_open,
	.release		= ioport_release,
	.read			= ioport_read,
	.write			= ioport_write,
	.llseek			= ioport_llseek,
	.unlocked_ioctl	= ioport_ioctl,
};

/*模块许可证*/
MODULE_LICENSE("Dual BSD/GPL");

/*内核模块全局变量*/
static dev_t ioport_dev_num;			/*主设备号*/
static struct class* ioport_class_p;	/*驱动类指针*/
static struct ioport_dev* ioport_dev_p;	/*私有数据指针*/

/*模块初始化函数*/
int __init ioportdev_init(void)
{
	int ret;
	/*申请字符驱动主设备号*/
	if(alloc_chrdev_region(&ioport_dev_num,0,1,IOPORTDEV_NAME)<0)
	{
		printk("IOPORTDEV:Can't register the device!\n");
		return -1;
	}
	/*创建ioport字符设备驱动类*/
	ioport_class_p = class_create(THIS_MODULE,IOPORTDEV_NAME);
	/*给私有数据分配内存*/
	ioport_dev_p = (struct ioport_dev*)kmalloc(sizeof(struct ioport_dev),GFP_KERNEL);
	if(!ioport_dev_p)
	{
		/*如果失败则注销所有前面初始化内容*/
		printk("IOPORTDEV:Can't allocate memory for ioport_dev_p!\n");
		class_destroy(ioport_class_p);
		unregister_chrdev_region(ioport_dev_num,1);
		return -ENOMEM;
	}
	/*初始化设备名*/
	sprintf(ioport_dev_p->name,"%s",IOPORTDEV_NAME);
	/*将字符设备操作与字符设备驱动关联*/
	cdev_init(&ioport_dev_p->char_dev,&ioport_fops);
	ioport_dev_p->char_dev.owner = THIS_MODULE;
	/*连接主设备号与字符设备*/
	ret = cdev_add(&ioport_dev_p->char_dev,ioport_dev_num,1);
	if(ret)
	{		
		/*如果出错则进行清理*/
		printk("IOPORTDEV:Bad char devices,Error Code:%x\n",ret);
		kfree((void*)ioport_dev_p);
		class_destroy(ioport_class_p);
		unregister_chrdev_region(ioport_dev_num,1);
		return ret;
	}
	/*向udev发送uevents以便创建/dev/ioport节点*/
	device_create(ioport_class_p,NULL,MKDEV(MAJOR(ioport_dev_num),0),NULL,IOPORTDEV_NAME);
	/*初始化各个成员列表*/
	INIT_LIST_HEAD(&ioport_dev_p->list_user);
	mutex_init(&ioport_dev_p->mutex_user);
	INIT_LIST_HEAD(&ioport_dev_p->port_inuse);
	mutex_init(&ioport_dev_p->mutex_port);
	init_waitqueue_head(&ioport_dev_p->wait_queue);
	printk("IOPORTDEV:ioport module initialized!\n");
	return 0;
}

/*模块退出函数*/
void __exit ioportdev_exit(void)
{
	/*释放主设备号*/
	unregister_chrdev_region(ioport_dev_num,1);
	/*释放设备类型*/
	device_destroy(ioport_class_p,MKDEV(MAJOR(ioport_dev_num),0));
	/*删除字符设备*/
	cdev_del(&ioport_dev_p->char_dev);
	kfree((void*)ioport_dev_p);
	/*销毁ioport字符设备类*/
	class_destroy(ioport_class_p);

	printk("IOPORTDEV:ioport module exited!\n");
	return;
}

module_init(ioportdev_init);
module_exit(ioportdev_exit);

