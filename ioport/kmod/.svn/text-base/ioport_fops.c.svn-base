/*************************************************************
 * File:		ioport_fops.c
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-1-24
 * Description:	ioport设备文件接口函数
 ************************************************************/

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include "ioport_fops.h"
#include "ioport_main.h"
#include "ioport_cmd.h"

/*****************************辅助函数******************************/
/*用户链表搜索,无互斥体保护*/
static inline struct io_user_des* _search_user(struct ioport_dev* dev_p)
{
	struct list_head* list_p;
	/*搜索用户链表，返回对应用户描述结构*/
	list_for_each(list_p,&dev_p->list_user)
	{
		struct io_user_des* user_p=list_entry(list_p,struct io_user_des,users);
		if(user_p->task_p->pid == current->pid)
			return user_p;
	}
	/*否则返回空指针*/
	return NULL;
}

/*用户链表搜索，有互斥体保护*/
static struct io_user_des* search_user(struct ioport_dev* dev_p)
{
	struct io_user_des* user_p;
	mutex_lock(&dev_p->mutex_user);
	user_p = _search_user(dev_p);
	mutex_unlock(&dev_p->mutex_user);
	return user_p;
}


/*添加用户*/
static int add_user(struct ioport_dev* dev_p)
{
	mutex_lock(&dev_p->mutex_user);
	/*用户不在用户链表则添加*/
	if(NULL==_search_user(dev_p))
	{
		/*分配新用户描述结构空间，并初始化*/
		struct io_user_des* user_p = (struct io_user_des*)kmalloc(sizeof(struct io_user_des),GFP_KERNEL);
		if(NULL==user_p)
		{
			mutex_unlock(&dev_p->mutex_user);
			return -ENOMEM;
		}
		user_p->task_p = current;
		user_p->port_addr = INIT_PORT_ADDR;
		user_p->req_mode = REQ_MODE_YES;
		user_p->rd_delay = 1;
		user_p->wr_delay = 1;
		INIT_LIST_HEAD(&user_p->users);
		/*添加到链表中*/
		list_add(&user_p->users,&dev_p->list_user);
		mutex_unlock(&dev_p->mutex_user);
#ifdef IOPORT_INFO
		printk("Add user,pid=%d\n",user_p->task_p->pid);
#endif
		return 0;
	}
	mutex_unlock(&dev_p->mutex_user);
	return 1;
}

static void del_user(struct ioport_dev* dev_p)
{
	struct io_user_des* user_p;
	mutex_lock(&dev_p->mutex_user);
	/*查找用户*/
	user_p = _search_user(dev_p);
	/*如果存在则删除*/
	if(user_p)
	{
#ifdef IOPORT_INFO
		printk("Del user,pid=%d\n",user_p->task_p->pid);
#endif
		list_del(&user_p->users);
		kfree((void*)user_p);
	}
	mutex_unlock(&dev_p->mutex_user);
}

/*端口搜索*/
static inline struct port_des* _port_search(struct ioport_dev* dev_p,unsigned long portaddr)
{
	struct list_head* list_p;
	/*列举链表中全部端口*/
	list_for_each(list_p,&dev_p->port_inuse)
	{
		struct port_des* port_p=list_entry(list_p,struct port_des,ports);
		/*如果有用户已经使用该端口,则返回端口描述指针*/
		if(portaddr == port_p->port_addr)
			return port_p;
	}
	/*否则返回NULL*/
	return NULL;
}

static int port_inuse(struct ioport_dev* dev_p,unsigned long portaddr)
{
	int rc;
	mutex_lock(&dev_p->mutex_port);
	if(NULL==_port_search(dev_p,portaddr))
		rc=IO_INUSE_NO;	/*端口不在列表，可以使用*/
	else
		rc=IO_INUSE_YES;
	mutex_unlock(&dev_p->mutex_port);
	return rc;
}

/*尝试锁定某个端口
 * 如果端口已经在inuse列表则返回IO_INUSE_YES*/
static int try_lock_port(struct ioport_dev* dev_p,unsigned long portaddr)
{
	int rc;
	mutex_lock(&dev_p->mutex_port);
	/*端口不在列表则添加*/
	if(NULL==_port_search(dev_p,portaddr))
	{
		struct port_des* port_p=(struct port_des*)kmalloc(sizeof(struct port_des),GFP_KERNEL);
		if(NULL==port_p)
			return -ENOMEM;
		port_p->port_addr = portaddr;
		INIT_LIST_HEAD(&port_p->ports);
		list_add(&port_p->ports,&dev_p->port_inuse);
		rc = IO_INUSE_NO;
#ifdef IOPORT_INFO
		printk("Pid=%d,Lock port:%lu\n",current->pid,port_p->port_addr);
#endif
	}
	else
		rc = IO_INUSE_YES;
	mutex_unlock(&dev_p->mutex_port);
	return rc;
}

/*将端口从使用列表移除，解锁端口*/
static void unlock_port(struct ioport_dev* dev_p,unsigned long portaddr)
{
	struct port_des* port_p;
	mutex_lock(&dev_p->mutex_port);
	/*查找到端口，则移除之*/
	port_p=_port_search(dev_p,portaddr);
	if(port_p)
	{
#ifdef IOPORT_INFO
		printk("Pid=%d,Unlock port:%lu\n",current->pid,port_p->port_addr);
#endif
		list_del(&port_p->ports);
		kfree((void*)port_p);
	}
	mutex_unlock(&dev_p->mutex_port);
	return;
}

/**************************************驱动程序接口部分*****************************/
/*打开端口驱动*/
int ioport_open(struct inode* inode,struct file* file)
{
	struct ioport_dev* dev_p;
	/*获取ioport私有数据结构指针并保存*/
	dev_p = container_of(inode->i_cdev,struct ioport_dev,char_dev);
	file->private_data = (void*)dev_p;
	/*添加用户*/
	add_user(dev_p);
	return 0;
}

/*端口驱动释放*/
int ioport_release(struct inode* inode,struct file* file)
{
	struct ioport_dev* dev_p = (struct ioport_dev*)file->private_data;
	/*删除用户*/
	del_user(dev_p);
	return 0;
}

/*读端口*/
ssize_t ioport_read(struct file* file,char* buf,size_t count,loff_t* ppos)
{
	struct ioport_dev* dev_p=(struct ioport_dev*)file->private_data;
	struct io_user_des* user_p;
	size_t i=0;
	size_t rdcount=count;
	unsigned char* rdbuf=NULL;
	unsigned long portaddr;
	/*获取用户描述指针*/
	user_p=search_user(dev_p);
	if(NULL==user_p)
		return -EIO;
	portaddr=user_p->port_addr;
	/*检查用户端口范围是否合法*/
	if(portaddr<MIN_PORT_ADDR || portaddr>MAX_PORT_ADDR)
		return -EIO;
	/*尝试锁定端口*/
	do
	{
		int rc=try_lock_port(dev_p,portaddr);
		/*出现错误则直接返回*/
		if(rc<0)
			return rc;
		if(IO_INUSE_NO==rc)
			break;
		/*如果是非阻塞方式读取，则直接返回*/
		if(file->f_flags & O_NONBLOCK)
			return -EBUSY;
		/*进入等待队列*/
		if(wait_event_interruptible(dev_p->wait_queue,IO_INUSE_NO==port_inuse(dev_p,portaddr)))
			return -ERESTARTSYS;
	}while(1);
#ifdef IOPORT_INFO
	printk("Pid=%d,Get into read section!\n",current->pid);
#endif
	/*是否使用requset_region获取资源*/
	if(user_p->req_mode)
		if(!request_region(portaddr,1,dev_p->name))
		{
			/*申请失败，则退出并返回0*/
			printk("ioport_read:Request port:%lu faild!\n",portaddr);
			return -EIO;
		}
	/*限制读出字节数*/
	if(rdcount>IOPORT_BUF_SIZE)
		rdcount=IOPORT_BUF_SIZE;
	/*分配读缓冲区*/
	rdbuf=(unsigned char*)kmalloc(sizeof(unsigned char)*rdcount,GFP_KERNEL);
	if(!rdbuf)
		return -ENOMEM;
	memset(rdbuf,0x00,rdcount);
	/*读端口*/
	for(i=0;i<rdcount;i++)
	{
		rdbuf[i]=inb(portaddr);
		udelay(user_p->rd_delay);
	}
	/*复制到用户空间*/
	if(copy_to_user(buf,(void*)rdbuf,rdcount))
	{
		printk("ioport_read:Transfer data to user space faild!\n");
		kfree((void*)rdbuf);
		return -EFAULT;
	}
	/*释放读缓冲*/
	kfree((void*)rdbuf);
	/*释放IO端口*/
	if(user_p->req_mode)
		release_region(portaddr,1);
	/*解锁端口并唤醒队列中其他进程*/
	unlock_port(dev_p,portaddr);
	wake_up_interruptible(&dev_p->wait_queue);
	return rdcount;
}

/*写端口*/
ssize_t ioport_write(struct file* file,const char* buf,size_t count,loff_t* ppos)
{
	struct ioport_dev* dev_p=file->private_data;
	struct io_user_des* user_p;
	unsigned long portaddr;
	unsigned char* wrbuf=NULL;
	size_t wrcount=count;
	size_t i;
	
	/*获取用户描述指针*/
	user_p=search_user(dev_p);
	if(NULL==user_p)
		return -EIO;
	portaddr=user_p->port_addr;
	/*检查用户端口范围是否合法*/
	if(portaddr<MIN_PORT_ADDR || portaddr>MAX_PORT_ADDR)
		return -EIO;
	/*尝试锁定端口*/
	do
	{
		int rc=try_lock_port(dev_p,portaddr);
		/*出现错误则直接返回*/
		if(rc<0)
			return rc;
		if(IO_INUSE_NO==rc)
			break;
		/*如果是非阻塞方式读取，则直接返回*/
		if(file->f_flags & O_NONBLOCK)
			return -EBUSY;
		/*进入等待队列*/
		if(wait_event_interruptible(dev_p->wait_queue,IO_INUSE_NO==port_inuse(dev_p,portaddr)))
			return -ERESTARTSYS;
	}while(1);
#ifdef IOPORT_INFO
	printk("Pid=%d,Get into write section!\n",current->pid);
#endif
	/*根据req_mode来决定是否使用request_region*/
	if(user_p->req_mode)
		if(!request_region(portaddr,1,dev_p->name))
		{
			/*申请失败，则退出并返回0*/
			printk("ioport_read:Request port:%lu faild!\n",portaddr);
			return -EIO;
		}
	/*限制写入字节数*/
	if(wrcount>IOPORT_BUF_SIZE)
		wrcount=IOPORT_BUF_SIZE;
	/*分配写缓冲区*/
	wrbuf=(unsigned char*)kmalloc(sizeof(unsigned char)*wrcount,GFP_KERNEL);
	if(!wrbuf)
		return -ENOMEM;
	memset(wrbuf,0x00,wrcount);
	/*获取用户数据*/
	if(copy_from_user((void*)wrbuf,buf,wrcount))
	{
		printk("ioport_write:Transfer data from user faild!\n");
		kfree((void*)wrbuf);
		return -EFAULT;
	}
	/*写入端口*/
	for(i=0;i<wrcount;i++)
	{
		outb(wrbuf[i],portaddr);
		udelay(user_p->wr_delay);
	}
	/*释放缓冲区*/
	kfree((void*)wrbuf);
	/*释放端口*/
	if(user_p->req_mode)
		release_region(portaddr,1);
	/*解锁端口并唤醒队列中其他进程*/
	unlock_port(dev_p,portaddr);
	wake_up_interruptible(&dev_p->wait_queue);
	return wrcount;
}

/*改变端口地址*/
loff_t ioport_llseek(struct file* file,loff_t offset,int orig)
{
	struct ioport_dev* dev_p=(struct ioport_dev*)file->private_data;
	loff_t result=0;
	/*获取用户描述指针*/
	struct io_user_des* user_p=search_user(dev_p);
	if(NULL==user_p)
		return -EIO;
	/*根据指定起始位置改变当前读写端口地址
	 *此处仅支持前两种方式*/
	switch(orig)
	{
		case SEEK_SET:
			result=offset;
			break;
		case SEEK_CUR:
			result=(loff_t)user_p->port_addr+offset;
			break;
		default:
			printk("ioport_llseek:This orig has not been suported yet!");
			return -EIO;
	}
	/*IO地址越界检查*/
	if(result>MAX_PORT_ADDR)
	{
		result=MAX_PORT_ADDR;
		printk("ioport_llseek:IO address out of range,adjust to:%d",MAX_PORT_ADDR);
	}
	/*保存重新定位的结果*/
	user_p->port_addr = result;
	return result; 
}

/*驱动参数设置*/
long ioport_ioctl(struct file* file,unsigned int cmd,unsigned long arg)
{
	long rc=0;
	/*获取结构指针*/
	struct ioport_dev* dev_p=file->private_data;
	/*获取用户描述指针*/
	struct io_user_des* user_p=search_user(dev_p);
	if(NULL==user_p)
		return -EIO;
	/*根据命令设置参数*/
	switch(cmd)
	{
		case SET_PORT:
			if(arg>MAX_PORT_ADDR)
				return -EIO;
			user_p->port_addr=arg;
			rc=arg;
			break;
		case SET_REQUEST:
			user_p->req_mode=REQ_MODE_YES;
			rc=0;
			break;
		case SET_NONREQ:
			user_p->req_mode=REQ_MODE_NO;
			rc=0;
			break;
		case SET_RD_DELAY:
			user_p->rd_delay=arg;
			break;
		case SET_WR_DELAY:
			user_p->wr_delay=arg;
			break;
		default:
			return -EIO;
	}
	return rc;
}

