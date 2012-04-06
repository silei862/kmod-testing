/*********************************************************
 * File:		cmosinfo.h
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-2-6
 * Description:	提供对cmos的日期及其他信息提供读写支持
 *
 *********************************************************/
#ifndef _CMOSINFO_H_
#define _CMOSINFO_H_

#include "cmosio.h"

/*基础信息结构*/
/*时间*/
struct cmostime{
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
};
/*日期*/
struct cmosdate{
	unsigned int 	year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	weekday;
};

/*接口函数*/
/*获取cmos时间和闹铃*/
int cmos_gettime(struct cmostime* tm,struct cmostime* altm);
/*设置cmos时间和闹铃*/
int cmos_settime(struct cmostime* tm,struct cmostime* altm);
/*获取cmos日期*/
int cmos_getdate(struct cmosdate* dt);
/*设置cmos日期*/
int cmos_setdate(struct cmosdate* dt);
/*获取cmos ram 0~7f*/
int cmos_getalldata(unsigned char* buf);
/*写入cmos ram 0~7f*/
int cmos_setalldata(unsigned char* buf);

#endif
