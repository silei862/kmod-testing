/************************************************************
 * File:		procinfo.h
 * Author:		思磊(Silei)
 * E-mail:		silei862@gmail.com
 * Description:	本文件为内核模块头文件定义一些必要的结构
 ***********************************************************/
#include <linux/list.h>
#include <linux/init.h>
#include <linux/sched.h>
#include "proc_cmd.h"

#ifndef _PROCINFO_H_
#define _PROCINFO_H_

#ifndef PROCINFODEV_NUM
#define PROCINFODEV_NUM 1			/*设备数目*/
#endif

#ifndef PROCINFODEV_NAME
#define PROCINFODEV_NAME "procinfo"	/*设备名称*/
#endif

#define DEV_NAME_LEN	0x20		/*设备名长度*/
#define MAX_INFO		0x1000		/*可枚举的最大进程数目*/
#define MAX_DATA_BUF	0x8000		/*结果输出缓冲大小*/
	
/*设备结构体*/
struct procinfo_dev{
	/*字符设备工作所需成员*/
	char 	name[DEV_NAME_LEN];		/*设备名*/
	struct 	cdev char_dev;			/*字符设备结构*/
	/*自定义成员*/
	char*			info_buf;		/*信息输出字符缓冲*/
	int 			cur_cmd;		/*当前用户命令*/				
	unsigned long 	cur_arg;		/*当前用户参数（一般为进程号）*/
};

/*简单进程信息结构访问标志常量*/
enum _ACCESS_FLAGS{
	ACF_TRUE 	= 0xff,
	ACF_FALSE 	= 0x00,
};
/*简单进程信息结构*/
struct simple_proc_info{
	char 	name[TASK_COMM_LEN];		/*进程名*/
	long 	pid;						/*进程号*/
	long 	tgid;						/*进程组号*/
	int		accessed;					/*访问标志*/
};

#endif /*_PROCINFO_H_*/
