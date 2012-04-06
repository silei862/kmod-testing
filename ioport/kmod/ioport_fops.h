/*************************************************************
 * File:		ioport_fops.h
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-1-24
 * Description:	ioport设备文件接口函数头文件
 ************************************************************/

#ifndef _IOPORT_FOPS_H_
#define _IOPORT_FOPS_H_

#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/mutex.h>


/******************字符文件设备接口函数************************/
/*打开*/
int ioport_open(struct inode* inode,struct file* file);
/*释放*/
int ioport_release(struct inode* inode,struct file* file);
/*读操作*/
ssize_t ioport_read(struct file* file,char* buf,size_t count,loff_t* ppos);
/*写操作*/
ssize_t ioport_write(struct file* file,const char* buf,size_t count,loff_t* ppos);
/*改变读写指针位置*/
loff_t ioport_llseek(struct file* file,loff_t offset,int orig);
/*端口控制*/
long ioport_ioctl(struct file* file,unsigned int cmd,unsigned long arg);


#endif
