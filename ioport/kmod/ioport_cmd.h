/*************************************************************
 * File:		ioport_cmd.h
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-1-24
 * Description:	定义ioctl所使用的命令常数
 ************************************************************/
#ifndef _IOPORT_CMD_H_
#define _IOPORT_CMD_H_
#include <linux/ioctl.h>
/*ioctl命令定义*/
#define IOPORT_MAGIC_CHAR 'k'
#define SET_PORT 		_IO(IOPORT_MAGIC_CHAR,1)	/*设置端口地址*/
#define SET_REQUEST		_IO(IOPORT_MAGIC_CHAR,2)	/*设置端口打开方式为使用request_region*/
#define SET_NONREQ		_IO(IOPORT_MAGIC_CHAR,3)	/*设置端口打开方式为不使用request_region*/
#define SET_RD_DELAY	_IO(IOPORT_MAGIC_CHAR,4)	/*设置端口读延时*/
#define SET_WR_DELAY	_IO(IOPORT_MAGIC_CHAR,5)	/*设置端口写延时*/

/*调试信息输出*/
#define IOPORT_INFO
#endif

