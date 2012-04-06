/*********************************************************
 * File:		ioport_main.h
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-1-24
 * Description:	ioport字符设备主内核模块头文件
 *********************************************************/

#ifndef _IOPORT_MAIN_H_
#define _IOPORT_MAIN_H_

#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/wait.h>

/*设备名称*/
#define IOPORTDEV_NAME	"ioport"
/*最大缓冲区长度*/
#define IOPORT_BUF_SIZE	0xFF
/*IO地址范围，x86*/
#define MIN_PORT_ADDR	0x0
#define MAX_PORT_ADDR	0xFFFF
#define INIT_PORT_ADDR	0x10000
/*占用标志常量*/
#define IO_INUSE_YES	0xFF
#define IO_INUSE_NO		0x00
/*是否使用request_region
 *来申请端口地址区域*/
#define REQ_MODE_YES	0xFF	/*推荐*/
#define	REQ_MODE_NO		0x00	/*不推荐使用*/

/*用户进程描述*/
struct io_user_des{
	struct task_struct*	task_p;		/*进程结构指针*/
	unsigned long		port_addr;	/*进程使用的端口号*/
	unsigned char		req_mode;	/*IO开启方式*/
	unsigned long		rd_delay;	/*读延时量*/
	unsigned long		wr_delay;	/*写延时量*/
	struct list_head	users;		/*用户进程链表*/
};

/*端口描述*/
struct port_des{
	unsigned long		port_addr;		/*正在被使用的端口地址*/
	struct list_head	ports;			/*端口链表*/
};

/*ioport驱动结构私有数据*/
struct ioport_dev{
	struct list_head	list_user;	/*用户链表*/
	struct mutex		mutex_user;	/*用户链表互斥体*/
	struct list_head	port_inuse;	/*被使用中的端口链表*/
	struct mutex		mutex_port;	/*端口链表互斥体*/
	wait_queue_head_t	wait_queue;	/*等待队列*/
	struct cdev			char_dev;	/*cdev结构*/
	char				name[0xF];	/*设备名*/
};


#endif

