/*********************************************************
 * File:		cmosio.h
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-2-4
 * Description:	提供对cmos的读写支持
 * 
 *********************************************************/

#ifndef _CMOSIO_H_
#define _CMOSIO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "kmod/ioport_cmd.h"

#define DEV_IO_NAME	"/dev/ioport"

enum _CMOS_CONST{
	/*IO参数*/
	REQ_MODE_YES	=0,
	REQ_MODE_NO		=1,
	DEF_RD_DELAY	=1,
	DEF_WR_DELAY	=1,
	MIN_DELAY		=1,
	MAX_DELAY		=300,
	MIN_CMOS_ADDR	=0,
	MAX_CMOS_ADDR	=127,
	/*地址参数*/
	CMOS_ADDR_PORT	=0x70,
	CMOS_DATA_PORT	=0x71,
};

enum _CMOS_IO_OPT_ID{
	OPT_REQ_MODE	=0,
	OPT_RD_DELAY	,
	OPT_WR_DELAY	,
	OPT_START_ADDR	,
	OPT_END_ADDR	,
	OPT_RDWR_BUF	,
	OPT_NUM			,
};

enum _CMOS_ERR_CODE{
	CODE_NULL			=0,
	RET_SUCCESS			=0,
	RET_FAILURE			=1,
	RET_ERROR			=2,
/*详细错误代码*/
	NULL_OBJECT			=3,
	NULL_BUF			=4,
	MODE_ARG_ERROR		=5,
	DELAY_ARG_ERROR		=6,
	ADDR_ARG_ERROR		=7,
	BUF_ARG_ERROR		=8,
	OPT_ID_ERROR		=9,
	FILEDEV_ERROR		=10,
	SET_REQ_MODE_FAILED	=11,
	SET_RD_DELAY_FAILED	=12,
	SET_WR_DELAY_FAILED	=13,
};

typedef struct _CMOS_OBJ{
	/*io参数*/
	int 			file_dev;
	unsigned char	req_mode;
	unsigned long	rd_delay;
	unsigned long	wr_delay;
	/*读写参数*/
	unsigned char	start_addr;
	unsigned char	end_addr;
	unsigned char*	buf;
	/*错误代码*/
	int				error_code;
} CMOS_OBJ_T;

typedef CMOS_OBJ_T* CMOS_OBJ_P;

/*cmos基础函数*/
/*创建cmos ram读写控件*/
CMOS_OBJ_P cmos_create_obj();
/*销毁读写控件*/
void cmos_destroy_obj(CMOS_OBJ_P pobj);
/*获取调试信息*/
void cmos_debug_info(CMOS_OBJ_P pobj);
/*参数设置，根据opt_id设置不同参数*/
int cmos_io_opt(CMOS_OBJ_P pobj,int opt_id,long arg);
/*使得io参数生效，写入驱动模块*/
int cmos_io_set(CMOS_OBJ_P pobj);
/*读取cmos ram*/
int cmos_read(CMOS_OBJ_P pobj);
/*写入cmos ram*/
int	cmos_write(CMOS_OBJ_P pobj);
/*读取cmos ram 0~7f*/
int cmos_read_all(CMOS_OBJ_P pobj,unsigned char* buf);
/*写入cmos ram 0~7f*/
int cmos_write_all(CMOS_OBJ_P pobj,unsigned char* buf);

#endif /*_CMOSIO_H_*/
