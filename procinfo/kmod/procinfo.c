/*******************************************************************
 * File:		procinfo.c
 * Author:		思磊(Silei)
 * E-mail:		silei862@gmail.com
 * Description:	设备驱动程序主要模块
 *******************************************************************/
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/sched.h>
#include "procinfo.h"

/*进程状态信息,摘取自内核源码fs/proc/array.c*/
static const char *task_state_array[] = {
	"running",          /*   0 */
    "sleeping",         /*   1 */
    "disk sleep",       /*   2 */
    "stopped",          /*   4 */
    "tracing stop",     /*   8 */
    "zombie",           /*  16 */
    "dead",             /*  32 */
    "dead",             /*  64 */
    "wakekill",         /* 128 */
    "waking",           /* 256 */
};

MODULE_LICENSE("Dual BSD/GPL");

/*********************************作业功能模块*************************************/
/*遍历所有进程(深度优先),并获取简单进程信息*/
void go_all_tasks(struct task_struct* task_p,struct simple_proc_info* info_buf,size_t * idx)
{	
	struct list_head* head_p;
	/*获取进程简单信息并保存*/
	info_buf[*idx].pid=task_p->pid;
	info_buf[*idx].tgid=task_p->tgid;
	info_buf[*idx].accessed=ACF_FALSE;
	sprintf(info_buf[*idx].name,"%s",task_p->comm);
	(*idx)++;
	/*开始遍历孩子进程*/
	for (head_p= task_p->children.next; head_p!= &(task_p->children);head_p = head_p->next)
	{
		/*获取孩子进程task_struct指针,并调用自身(递归)*/
		struct task_struct* chd_task_p=list_entry(head_p,struct task_struct,sibling);
		go_all_tasks(chd_task_p,info_buf,idx);                            
	}
}

/*将简单进程信息按照群组关系输出到group_buf*/
void get_group_view(char* group_buf)
{
	size_t i=0;
	size_t j=0;
	size_t buf_len=0;
	struct simple_proc_info* info_buf = (struct simple_proc_info*)kmalloc(sizeof(struct simple_proc_info)*MAX_INFO,GFP_KERNEL);
	/*调用go_all_tasks遍历所有进程,获取简单进程信息*/
	go_all_tasks(&init_task,info_buf,&buf_len);
	/*根据简单信息,进行分组*/
	for(i=0;i<buf_len;i++) 
		if(ACF_FALSE==info_buf[i].accessed) /*进程未被访问过*/
	    {
			/*输出进程信息到群组关系字符缓冲group_buf,并设置访问标志*/
			sprintf(group_buf,"%s  %s[pid=%ld tgid=%ld]\n",group_buf,info_buf[i].name,info_buf[i].pid,info_buf[i].tgid);
			info_buf[i].accessed = ACF_TRUE;
			for(j=0;j<buf_len;j++)
		    	if(ACF_FALSE==info_buf[j].accessed && info_buf[i].pid==info_buf[j].tgid) /*该条信息未被输出并且组号等于首领进程号*/
		   		{
					/*输出为该群组进程,并设置访问标志*/
					sprintf(group_buf,"%s     |>-->%s[pid=%ld tgid=%ld]\n",group_buf,info_buf[j].name,info_buf[j].pid,info_buf[j].tgid);
		    		info_buf[j].accessed = ACF_TRUE;
		    	}
		    	else
					continue;
	    }
	    else
			continue;

	kfree((void*)info_buf);
	return;
}

/*遍历所有进程(深度优先),并生成进程树信息*/
void get_tree_view(struct task_struct* task_p,int tab_num,char* tree_buf)
{	
	int i;
	struct list_head* head_p;
	/*输出指定数目空格串(缩进)*/
	for(i=1;i<=tab_num;i++)
		sprintf(tree_buf,"%s%s",tree_buf,"      |");
	/*输出当前进程信息到tree_buf*/
	sprintf(tree_buf,"%s>-->%s pid=%d tgid=%d\n",tree_buf,task_p->comm,task_p->pid,task_p->tgid);
	/*遍历孩子进程*/
	for(head_p = task_p->children.next; head_p!= &(task_p->children); head_p = head_p->next)
	{
		/*获取孩子进程task_struct指针,并调用自身(递归)*/
		struct task_struct* chd_task_p = list_entry(head_p,struct task_struct,sibling);
		get_tree_view(chd_task_p,tab_num+1,tree_buf);                            
	}
}

/*从task_p开始查找进程为pid的进程,并返回该进程task_struct*/
struct task_struct* search_task(struct task_struct* task_p,int pid)
{
	struct task_struct* ret_task_p=NULL;
	struct list_head* head_p;
	if(pid==task_p->pid) /*找到,返回其task_struct指针*/
		return task_p;
	/*遍历孩子进程*/		
	for(head_p = task_p->children.next; head_p!=&(task_p->children); head_p = head_p->next)
	{
		struct task_struct* chd_task_p = list_entry(head_p,struct task_struct,sibling);
		ret_task_p = search_task(chd_task_p,pid);	/*对孩子进程递归*/
		if(ret_task_p)	/*如果找到则不再查找*/
			break;
	}
	return ret_task_p;
}

/*获取指定进程内存布局信息*/
void get_mem_info(char* info_buf,int pid)
{
	unsigned long seg_size;
	unsigned long pgd;
	unsigned long pgd_pa;
	struct mm_struct* mm_p;
	struct task_struct* task_p=search_task(&init_task,pid);/*从init_task开始查找*/
	if(task_p) /*找到,则输出信息*/
	{
		sprintf(info_buf,"%s[pid=%d],Memory Information:\n",task_p->comm,task_p->pid);
		if(task_p->mm)
		{
			mm_p=task_p->mm;
			/*代码段*/
			seg_size=mm_p->end_code-mm_p->start_code;
			sprintf(info_buf,"%s     Code Start:0x%lx, Size:0x%lx\n",info_buf,mm_p->start_code,seg_size);
			/*数据段*/
			seg_size=mm_p->end_data-mm_p->start_data;
			sprintf(info_buf,"%s     Data Start:0x%lx, Size:0x%lx\n",info_buf,mm_p->start_data,seg_size);
			/*栈(stack)*/
			sprintf(info_buf,"%s     Stack Start:0x%lx, Size:0x%lx\n",info_buf,mm_p->start_stack,mm_p->stack_vm);
			/*堆*/
			sprintf(info_buf,"%s     BRK Start:0x%lx, BRK:0x%lx\n",info_buf,mm_p->start_brk,mm_p->brk);
			/*参数*/
			sprintf(info_buf,"%s     ARG Start:0x%lx, ARG End:%lx\n",info_buf,mm_p->arg_start,mm_p->arg_end);
			/*环境变量*/
			sprintf(info_buf,"%s     ENV Start:0x%lx, ENV End:%lx\n",info_buf,mm_p->env_start,mm_p->env_end);
			/*页目录物理地址*/
			pgd_pa=__pa(mm_p->pgd);
			/*页目录内容*/
			pgd=pgd_val(*(mm_p->pgd));
			sprintf(info_buf,"%s     PGD Physical Address:0x%lx\n     PGD Value:0x%lx\n",info_buf,pgd_pa,pgd);
		}
	}
}

/*获取进程状态字符串,以便输出,摘取自内核源码fs/proc/array.c*/
static inline const char *get_task_state(struct task_struct *tsk)
{
	unsigned int state = (tsk->state & TASK_REPORT) | tsk->exit_state;/*获取进程状态*/
    const char **p = &task_state_array[0];	/*获取状态字符串指针*/
	/*对state逐位移位,直到state==0*/
    while (state) {	
		p++;
        state >>= 1;	/*逐个移位*/
    }
    return *p;
}

/*获取指定进程详细信息*/
void get_detail_info(char* info_buf,int pid)
{
	struct timespec* time_p=NULL;
	struct task_struct* task_p=search_task(&init_task,pid); /*从init_task开始查找*/
	if(task_p) /*找到,则输出信息*/
	{
		sprintf(info_buf,"%s[pid=%d],Other Information:\n",task_p->comm,task_p->pid);
		/*优先级信息*/
		sprintf(info_buf,"%s      Process State:%s\n",info_buf,get_task_state(task_p));	
		sprintf(info_buf,"%s      Process Dynamic Priority:%d\n",info_buf,task_p->prio);
		sprintf(info_buf,"%s      Process Normal  Priority:%d\n",info_buf,task_p->normal_prio);
		sprintf(info_buf,"%s      Process Static  Priority:%d\n",info_buf,task_p->static_prio);	
		sprintf(info_buf,"%s      Process Realtime Priority:%d\n",info_buf,task_p->rt_priority);
		/*进程时间信息*/
		time_p=&task_p->start_time;
		sprintf(info_buf,"%s      Process Monotonic time:%ld.%ld\n",info_buf,time_p->tv_sec,time_p->tv_nsec);
		time_p=&task_p->real_start_time;
		sprintf(info_buf,"%s      Process Real Start time:%ld.%ld\n",info_buf,time_p->tv_sec,time_p->tv_nsec);
	}
}

/*******************************驱动程序接口函数部分*********************************/
/*驱动open接口*/
int procinfo_open(struct inode* inode,struct file* file)
{
	struct procinfo_dev* procinfo_dev_p;
	printk("open operation called!\n");
	/*获取字符设备结构指针并保存*/
	procinfo_dev_p = container_of(inode->i_cdev,struct procinfo_dev,char_dev);
	file->private_data = procinfo_dev_p;
	/*初始化缓冲区,命令及参数*/
	procinfo_dev_p->info_buf=(char*)kmalloc(sizeof(char)*MAX_DATA_BUF,GFP_KERNEL);
	if(NULL==procinfo_dev_p->info_buf)	/*缓冲区申请失败*/
	{
		printk("Can't allocate info_buf!!!");
		return -ENOMEM;
	}
	procinfo_dev_p->cur_cmd = CMD_NULL;
	procinfo_dev_p->cur_arg = 0;	
	return 0;
}

/*驱动release接口*/
int procinfo_release(struct inode* inode,struct file* file)
{
	/*释放所申请缓冲*/
	struct procinfo_dev* procinfo_dev_p=file->private_data;
	kfree((void*)procinfo_dev_p->info_buf);
	printk("release operation called!\n");
	return 0;
}

/*驱动read接口*/
ssize_t procinfo_read(struct file* file,char* buf,size_t count,loff_t* ppos)
{
	size_t len=0;
	struct procinfo_dev* procinfo_dev_p=file->private_data;
	char* info_data=procinfo_dev_p->info_buf;	/*别名,便于使用*/
	memset(info_data,0x00,sizeof(char)*MAX_DATA_BUF);	/*填充0,防止不必要错误*/
	/*根据命令执行不同操作*/
	switch(procinfo_dev_p->cur_cmd)
	{
		case CMD_PROC_TREE:	/*按进程树输出所有进程*/
			printk("PROC_TREE cmd invoked!\n"); 
			get_tree_view(&init_task,1,info_data);
			break;
		case CMD_THREAD_GROUP: /*按线程组关系输出所有进程*/
			printk("THREAD_GROUP cmd invoked!\n");
			get_group_view(info_data);	
			break;
		case CMD_MEM_STATE:	/*输出指定进程内存布局*/
			printk("MEM_STATUS cmd invoked!arg=%ld\n",procinfo_dev_p->cur_arg);
			get_mem_info(info_data,procinfo_dev_p->cur_arg);
			break;
		case CMD_PROC_DETAIL:	/*输出指定进程详情*/
			printk("PROC_DETAIL cmd invoked!arg=%ld\n",procinfo_dev_p->cur_arg);
			get_detail_info(info_data,procinfo_dev_p->cur_arg);
			break;
		default:		
			printk("procinfo.ko:Unknown command invoked\n");
			return 0;
	}
	len=strlen(info_data);
	/*复制到用户空间*/
	if(copy_to_user(buf,(void*)info_data,len))
	{
		printk("I/O Error!\n");
		return -EIO;
	}
	return len;
}

/*驱动write接口,作业中未用到,仅做测试*/
ssize_t procinfo_write(struct file* file,const char* buf,size_t count,loff_t* ppos)
{
	char* kbuf;
	printk("write operation called!\n");
	kbuf=(char*)kmalloc(count+1,GFP_KERNEL);
	if(NULL==kbuf)
	{
		printk("procinfo_write:No enough memory!\n");
		return -ENOMEM;
	}
	if(copy_from_user(kbuf,buf,count))
	{
		printk("procinfo_write:EFAULT!\n");
		return -EFAULT;
	}
	kbuf[count]=0x00;
	printk("User Message:%s\n",kbuf);
	return count;
}

/*驱动seek接口,作业中未用到,仅做测试*/
static loff_t procinfo_llseek(struct file* file,loff_t offset,int orig)
{
	printk("seek operation called!\n");
	printk("orig=%x\n",orig);
	return offset;
}

/*驱动ioctl接口*/
static long procinfo_ioctl(struct file* file,unsigned int cmd,unsigned long arg)
{
	struct procinfo_dev* procinfo_dev_p;
	
	printk("ioctl operation calld!\n");
	printk("cmd=%x,arg=%lx\n",cmd,arg);
	/*保存用户命令及参数*/
	procinfo_dev_p = file->private_data;
	procinfo_dev_p->cur_cmd = cmd;
	procinfo_dev_p->cur_arg = arg;
	return 0;
}

/*将上面驱动接口函数"填表"*/
static struct file_operations procinfo_fops = {
	.owner				= THIS_MODULE,
	.open				= procinfo_open,
	.release			= procinfo_release,
	.read				= procinfo_read,	
	.write				= procinfo_write,
	.llseek				= procinfo_llseek,
	.unlocked_ioctl		= procinfo_ioctl,		
};

/*************************内核模块初始化及退出部分********************************/
static dev_t procinfo_dev_num;			/*存放主设备号*/
struct class *procinfo_class;			/*存放驱动类别*/
struct procinfo_dev* procinfo_dev_p;	/*自定义字符驱动结构指针*/

/*初始化函数*/
int __init procinfo_init(void)
{
	int ret;
	/*申请字符驱动主设备号*/
	if(alloc_chrdev_region(&procinfo_dev_num,0,1,PROCINFODEV_NAME)<0)
	{
		printk(KERN_DEBUG "Can't register devices\n");
		return -1;
	}
	/*创建procinfo驱动类别*/
	procinfo_class = class_create(THIS_MODULE,PROCINFODEV_NAME);	
	procinfo_dev_p = kmalloc(sizeof(struct procinfo_dev),GFP_KERNEL);
	if(!procinfo_dev_p)
	{
		printk("Can't allocate memory for procinfo_dev_p\n");
		return -ENOMEM;
	}
	/*初始化字符设备驱动*/
	sprintf(procinfo_dev_p->name,"procinfo");
	cdev_init(&procinfo_dev_p->char_dev,&procinfo_fops);
	procinfo_dev_p->char_dev.owner = THIS_MODULE;	
	/*添加字符设备驱动*/
	ret = cdev_add(&procinfo_dev_p->char_dev,procinfo_dev_num,1);
	if(ret)
	{
		printk("Error char device,code=%x",ret);
		return ret;
	}
	/*创建该设备,生成/dev/procinfo*/
	device_create(procinfo_class,NULL,MKDEV(MAJOR(procinfo_dev_num),0),NULL,"procinfo");
	return 0;
}

/*模块退出*/
void __exit procinfo_exit(void)
{
	/*释放主设备号*/
	unregister_chrdev_region((procinfo_dev_num),1);
	/*释放设备*/
	device_destroy(procinfo_class,MKDEV(MAJOR(procinfo_dev_num),0));
	/*删除设备及其他清理工作*/
	cdev_del(&procinfo_dev_p->char_dev);
	kfree(procinfo_dev_p);
	class_destroy(procinfo_class);
	return;
}

module_init(procinfo_init);
module_exit(procinfo_exit);

