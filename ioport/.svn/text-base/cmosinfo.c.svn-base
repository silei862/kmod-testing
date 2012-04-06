/*********************************************************
 * File:		cmosinfo.c
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-2-6
 * Description:	提供对cmos的日期及其他信息提供读写支持
 *
 *********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmosio.h"
#include "cmosinfo.h"
/*参数设置数组*/
static long para_array[OPT_NUM]={0};

/*对外隐藏接口*/
/*参数设置*/
static int cmos_para_set(CMOS_OBJ_P pobj)
{
	int rc=0;
	int opt;
	para_array[OPT_REQ_MODE]=REQ_MODE_NO;
	para_array[OPT_RD_DELAY]=1;
	para_array[OPT_WR_DELAY]=1;

	/*逐个参数设置*/
	for(opt=0;opt<OPT_NUM;opt++)
		if(rc=cmos_io_opt(pobj,opt,para_array[opt]))
			break;
	return rc;
}

/*格式转换*/
static inline unsigned char bcd2dec(unsigned char bcd)
{
	int num=(int)bcd;
	num=(num>>4)*10+(int)(num&(int)0xf);
	return (unsigned char)num;
}

static inline unsigned char dec2bcd(unsigned char dec)
{
	int num;
	num=((dec/(unsigned char)10)<<4)|(dec%(unsigned char)10);
	return num;
}

/**************************对外开放接口***************************/
/*获取时间信息*/
int cmos_gettime(struct cmostime* tm,struct cmostime* altm)
{
	int rc;
	unsigned char buf[6]={0};
	/*初始化*/
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	para_array[OPT_RDWR_BUF]=(long)buf;
	/*起始地址*/
	para_array[OPT_START_ADDR]=0;
	para_array[OPT_END_ADDR]=5;
	/*设置参数*/
	rc=cmos_para_set(pobj);
	if(rc)
		return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*读取*/
	rc=cmos_read(pobj);
	if(rc)
		return rc;
	/*转换时间信息*/
	if(tm)
	{
		memset(tm,0,sizeof(struct cmostime));
		tm->second=bcd2dec(buf[0]);
		tm->minute=bcd2dec(buf[2]);
		tm->hour=bcd2dec(buf[4]);
	}
	if(altm)
	{
		memset(altm,0,sizeof(struct cmostime));
		altm->second=bcd2dec(buf[1]);
		altm->minute=bcd2dec(buf[3]);
		altm->hour=bcd2dec(buf[5]);
	}
	cmos_destroy_obj(pobj);
	return RET_SUCCESS;
}

/*设置时间信息*/

int cmos_settime(struct cmostime* tm,struct cmostime* altm)
{
	int rc;
	unsigned char buf[6]={0};
	/*初始化*/
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	para_array[OPT_RDWR_BUF]=(long)buf;
	/*起始地址*/
	para_array[OPT_START_ADDR]=0;
	para_array[OPT_END_ADDR]=5;
	/*设置参数*/
	rc=cmos_para_set(pobj);
	if(rc)
		return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*读取,保证原有信息正确性*/
	rc=cmos_read(pobj);
	/*转换时间格式*/
	if(tm)
	{
		buf[0]=dec2bcd(tm->second);
		buf[2]=dec2bcd(tm->minute);
		buf[4]=dec2bcd(tm->hour);
	}
	if(altm)
	{
		buf[1]=dec2bcd(altm->second);
		buf[3]=dec2bcd(altm->minute);
		buf[5]=dec2bcd(altm->hour);
	}
	/*写入信息保存*/
	rc=cmos_write(pobj);
	if(rc)
		return rc;
	return RET_SUCCESS;
}

/*获取日期信息*/
int cmos_getdate(struct cmosdate* dt)
{
	int rc;
	unsigned char buf[4]={0};
	unsigned char century;
	/*初始化*/
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	para_array[OPT_RDWR_BUF]=(long)buf;
	/*起始地址*/
	para_array[OPT_START_ADDR]=6;
	para_array[OPT_END_ADDR]=9;
	/*设置参数*/
	rc=cmos_para_set(pobj);
	if(rc)
		return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*读取日期信息*/
	rc=cmos_read(pobj);
	if(rc)
		return rc;
	/*设置世纪信息提取参数*/
	para_array[OPT_START_ADDR]=50;
	para_array[OPT_END_ADDR]=50;
	para_array[OPT_RDWR_BUF]=(long)(&century);
	rc=cmos_para_set(pobj);
	/*读取世纪信息*/
	rc=cmos_read(pobj);
	if(rc)
		return rc;
	/*转换日期信息*/
	if(dt)
	{
		memset(dt,0,sizeof(struct cmosdate));
		dt->weekday=bcd2dec(buf[0]);
		dt->day=bcd2dec(buf[1]);
		dt->month=bcd2dec(buf[2]);
		dt->year=bcd2dec(buf[3])+bcd2dec(century)*100;
	}
	cmos_destroy_obj(pobj);
	return RET_SUCCESS;
}

/*设置日期*/
int cmos_setdate(struct cmosdate* dt)
{
	int rc;
	unsigned char buf[4]={0};
	unsigned char century;
	/*初始化*/
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	/*转换日期格式*/
	if(dt)
	{
		buf[0]=dec2bcd(dt->weekday);
		buf[1]=dec2bcd(dt->day);
		buf[2]=dec2bcd(dt->month);
		buf[3]=dec2bcd(dt->year%100);
		century=dec2bcd(dt->year/100);
	}
	/*设置写入参数*/
	para_array[OPT_RDWR_BUF]=(long)buf;
	/*起始地址*/
	para_array[OPT_START_ADDR]=6;
	para_array[OPT_END_ADDR]=9;
	rc=cmos_para_set(pobj);
	if(rc)
		return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*写入日期信息*/
	rc=cmos_write(pobj);
	if(rc)
		return rc;
	/*写入世纪信息*/
	para_array[OPT_RDWR_BUF]=(long)&century;
	para_array[OPT_START_ADDR]=50;
	para_array[OPT_END_ADDR]=50;
	rc=cmos_para_set(pobj);
	if(rc)
		return rc;
	rc=cmos_write(pobj);
	if(rc)
		return rc;
	return RET_SUCCESS;
}

/*获取CMOS所有数据*/
int cmos_getalldata(unsigned char* buf)
{
	int rc;
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	/*读取所有数据*/
	rc=cmos_read_all(pobj,buf);
	if(rc)
		return rc;
	cmos_destroy_obj(pobj);
	return RET_SUCCESS;
}

/*写入数据（0-128）*/
int cmos_setalldata(unsigned char* buf)
{
	int rc;
	CMOS_OBJ_P pobj=cmos_create_obj();
	if(NULL==pobj)
		return NULL_OBJECT;
	/*写入数据(0-128)*/
	rc=cmos_write_all(pobj,buf);
	if(rc)
		return rc;
	cmos_destroy_obj(pobj);
	return RET_SUCCESS;
}
