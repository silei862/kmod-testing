/*********************************************************
 * File:		cmosedit.c
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-2-7
 * Description:	CMOS内存编辑程序，提供简单的用户接口
 *
 *********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cmosio.h"
#include "cmosinfo.h"

enum _cmosedit_const{
	CMOS_BUF_SIZE	=128,
	MAX_KEY_BUF		=128,
};

struct input_ctrl{
	/*控制成员*/	
	int 	min;
	int 	max;
	char*	prompt;
	char*	esc_str;
	char*	fmt;
	/*结果成员*/
	char	keybuf[MAX_KEY_BUF];
	int		ret_val;
	int		ret_state;
};

char keybuf[MAX_KEY_BUF]={0};

/*星期几的字符表*/
char* week_str[]={
	"日",
	"一",
	"二",
	"三",
	"四",
	"五",
	"六"
};

/*询问选项*/
#define Q_OPT_NUM 2
char* q_opt[]={
	"yes",
	"no"
};

/*输入辅助模块*/
void keybuf_convert(char* buf);
int get_valid_int(struct input_ctrl* inpc);
int query_user(char* prompt,char** options,int n_opt,int def_opt);
/*各个功能模块*/
void print_main_menu();
void print_date();
void modify_date();
void print_time();
void modify_time();
void print_alarm();
void modify_alarm();
void print_cmos_content();
void modify_cmos_content();
void backup_cmos();
void restore_cmos();

int main()
{
	int is_quit=0;
	/*程序主循环*/
	while(!is_quit)
	{
		int choice;
		char* retstr;
		print_main_menu();
		printf("选择[0~8]:");
		retstr=fgets(keybuf,MAX_KEY_BUF,stdin);
		choice=atoi(keybuf);
		/*根据选择执行相应功能*/
		switch(choice)
		{
			/*退出*/
			case 0:
				is_quit=1;
				break;
			case 1:
				print_date();
				if(0==query_user("进行修改？",q_opt,Q_OPT_NUM,1))
					modify_date();
				break;
			case 2:
				print_time();
				if(0==query_user("进行修改？",q_opt,Q_OPT_NUM,1))
					modify_time();
				break;
			case 3:
				print_alarm();	
				if(0==query_user("进行修改?",q_opt,Q_OPT_NUM,1))
					modify_alarm();
				break;
			case 4:
				print_cmos_content();
				if(0==query_user("进行修改?",q_opt,Q_OPT_NUM,1))
					modify_cmos_content();
				break;
			case 5:
				backup_cmos();
				break;
			case 6:
				restore_cmos();
				break;
			default:
				is_quit=0;
		}

	}
	
	return EXIT_SUCCESS;
}

/*缓冲区转换*/
void keybuf_convert(char* buf)
{
	int i=0;
	while(buf[i])
		i++;
	if(0==i)
		return;
	buf[i-1]=0x0;
	return;
}

/*获取合法输入*/
int get_valid_int(struct input_ctrl* inpc)
{
	char* retstr;
	int	ret_code;
	printf("(提示：输入%s放弃修改)\n",inpc->esc_str);
	while(1)
	{
		printf("%s",inpc->prompt);	/*输出提示*/
		retstr=fgets(inpc->keybuf,MAX_KEY_BUF,stdin);	/*获取输入*/
		sscanf(inpc->keybuf,inpc->fmt,&inpc->ret_val);	/*格式转换*/
		keybuf_convert(inpc->keybuf);
		if(!strcmp(inpc->esc_str,inpc->keybuf))		/*如果是退出字符串，退出*/
		{
			ret_code=1;
			break;
		}
		if(inpc->ret_val>=inpc->min && inpc->ret_val<=inpc->max)	/*范围限定*/
		{
			ret_code=0;
			break;
		}
	}
	return ret_code;
}

/*询问用户*/
int query_user(char* prompt,char** options,int n_opt,int def_opt)
{
	int i;
	char* ret_str;
	while(1)
	{
		/*输出查询*/
		printf("%s(%s",prompt,options[0]);
		for(i=1;i<n_opt;i++)
			printf("|%s",options[i]);
		printf(")%s：",options[def_opt]);
		ret_str=fgets(keybuf,MAX_KEY_BUF,stdin);
		/*如果是默认选项*/
		if(10==keybuf[0])
			return def_opt;
		keybuf_convert(keybuf);
		/*选项比较*/
		for(i=0;i<n_opt;i++)
			if(!strcmp(keybuf,options[i]))
				return i;
	}
	return 0;
}

/*输出主菜单*/
void print_main_menu()
{
	printf("\n");
	printf("[1]CMOS日期\n");
	printf("[2]CMOS时间\n");
	printf("[3]CMOS闹铃\n");
	printf("[4]CMOS内存\n");
	printf("[5]备份CMOS\n");
	printf("[6]恢复CMOS\n");
	printf("[0]退出程序\n");
}

/*输出日期*/
void print_date()
{
	int rc;
	struct cmosdate date;
	printf("读取中，请稍候...\n");
	rc=cmos_getdate(&date);
	if(rc)
	{
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	printf("\nCMOS日期：%u年%u月%u日 星期%s\n",date.year,date.month,date.day,week_str[date.weekday]);
	return;
}

/*修改日期*/
void modify_date()
{
	int rc;
	struct cmosdate date;
	struct input_ctrl inpc;
	inpc.fmt="%d";
	inpc.esc_str="quit";
	/*获取年份*/
	inpc.min=1971;
	inpc.max=2071;
	inpc.prompt="年：";
	if(get_valid_int(&inpc))
		return;
	date.year=inpc.ret_val;
	/*获取月份*/
	inpc.min=1;
	inpc.max=12;
	inpc.prompt="月(1~12)：";
	if(get_valid_int(&inpc))
		return;
	date.month=inpc.ret_val;
	/*获取日*/
	inpc.min=1;
	inpc.max=31;
	inpc.prompt="日(1~31):";
	if(get_valid_int(&inpc))
		return;
	date.day=inpc.ret_val;
	/*获取星期*/
	inpc.min=0;
	inpc.max=6;
	inpc.prompt="星期(0~6):";
	if(get_valid_int(&inpc))
		return;
	date.weekday=inpc.ret_val;
	printf("写入中，请稍候...\n");
	rc=cmos_setdate(&date);
	if(rc)
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);
	return;
}

/*输出时间*/
void print_time()
{
	int rc;
	struct cmostime time;
	printf("读取中，请稍候...\n");
	rc=cmos_gettime(&time,NULL);
	if(rc)
	{
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	printf("\nCMOS时间：%d时%d分%d秒\n",time.hour,time.minute,time.second);
	return;
}

/*修改时间*/
void modify_time()
{
	int rc;
	struct cmostime time;
	struct input_ctrl inpc;
	inpc.fmt="%d";
	inpc.esc_str="quit";
	/*获取小时*/
	inpc.min=0;
	inpc.max=23;
	inpc.prompt="时(0~23)：";
	if(get_valid_int(&inpc))
		return;
	time.hour=inpc.ret_val;
	/*获取分钟*/
	inpc.min=0;
	inpc.max=59;
	inpc.prompt="分(0~59)：";
	if(get_valid_int(&inpc))
		return;
	time.minute=inpc.ret_val;
	/*获取秒*/
	inpc.min=0;
	inpc.max=59;
	inpc.prompt="秒(0~59)：";
	if(get_valid_int(&inpc))
		return;
	time.second=inpc.ret_val;
	printf("写入中，请稍候...\n");
	rc=cmos_settime(&time,NULL);
	if(rc)
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);
	return;
}

/*输出闹铃时间*/
void print_alarm()
{
	int rc;
	struct cmostime alarm_time;
	printf("读取中，请稍候...\n");
	rc=cmos_gettime(NULL,&alarm_time);
	if(rc)
	{
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	printf("\nCMOS闹铃：%d时%d分%d秒\n",alarm_time.hour,alarm_time.minute,alarm_time.second);
	return;
}

/*修改闹铃*/
void modify_alarm()
{
	int rc;
	struct cmostime alarm_time;
	struct input_ctrl inpc;
	inpc.fmt="%d";
	inpc.esc_str="quit";
	/*获取小时*/
	inpc.min=0;
	inpc.max=23;
	inpc.prompt="时(0~23)：";
	if(get_valid_int(&inpc))
		return;
	alarm_time.hour=inpc.ret_val;
	/*获取分钟*/
	inpc.min=0;
	inpc.max=59;
	inpc.prompt="分(0~59)：";
	if(get_valid_int(&inpc))
		return;
	alarm_time.minute=inpc.ret_val;
	/*获取秒*/
	inpc.min=0;
	inpc.max=59;
	inpc.prompt="秒(0~59)：";
	if(get_valid_int(&inpc))
		return;
	alarm_time.second=inpc.ret_val;
	printf("写入中，请稍候...\n");
	rc=cmos_settime(NULL,&alarm_time);
	if(rc)
		printf("出现错误，Code=%d，请联系silei862@gmail.com\n",rc);

	return;
}

/*输出CMOS存储区内容*/
void print_cmos_content()
{
	int i,rc;
	unsigned char buf[CMOS_BUF_SIZE]={0};
	printf("读取中，请稍候...\n");
	rc=cmos_getalldata(buf);
	if(rc)
	{
		printf("出现错误，错误代码%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	/*打印表头*/
	printf("============================CMOS Content============================\n");
	printf("CMOS");
	for(i=0;i<0x10;i++)
		printf("[%2x]",i);
	for(i=0;i<128;i++)
	{
		if(!(i%0x10))
			printf("\n[%2x]",i);
		printf(" %2x ",buf[i]);
	}
	printf("\n====================================================================\n");
}

/*修改CMOS存储区*/
void modify_cmos_content()
{
	int rc;
	int addr;
	struct input_ctrl inpc;
	char prompt_str[0x20]={0};
	unsigned char buf[CMOS_BUF_SIZE]={0};
	/*先读取修改后全部回写*/
	printf("读取中，请稍候...\n");
	rc=cmos_getalldata(buf);
	if(rc)
	{
		printf("出现错误，错误代码%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	inpc.esc_str="quit";
	inpc.fmt="%x";
	/*输入*/
	while(1)
	{
		/*CMOS地址输入*/
		inpc.prompt="CMOS地址(0~7F)：";
		inpc.min=0x0;
		inpc.max=0x7f;
		/*获取用户输入，如果是quit命令则终止执行*/
		if(get_valid_int(&inpc))
			break;
		addr=inpc.ret_val;
		/*输入该地址内存值*/
		sprintf(prompt_str,"内容CMOS_MEM[%x]=",addr);
		inpc.prompt=prompt_str;
		inpc.min=0x0;
		inpc.max=0xff;
		/*获取用户输入*/
		if(get_valid_int(&inpc))
			break;
		buf[addr]=(unsigned char)inpc.ret_val;
	}
	/*回写内容*/
	printf("写入中，请稍候...\n");
	rc=cmos_setalldata(buf);
	if(rc)
	{
		printf("出现错误，错误代码%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	return;
}

/*备份CMOS*/
void backup_cmos()
{
	FILE* fp;
	char* ret_str;
	unsigned char buf[CMOS_BUF_SIZE]={0};
	int rc;
	size_t wrsz;
	/*获取备份文件路径*/
	printf("\n备份文件路径：");
	ret_str=fgets(keybuf,MAX_KEY_BUF,stdin);
	keybuf_convert(keybuf);
	/*尝试创建文件*/
	fp=fopen(keybuf,"wb+");
	if(NULL==fp)
	{
		printf("文件%s创建失败！\n",keybuf);
		return;
	}
	/*读取CMOS内存*/
	printf("读取中，请稍候...\n");
	rc=cmos_getalldata(buf);
	if(rc)
	{
		printf("出现错误，错误代码%d，请联系silei862@gmail.com\n",rc);
		return;
	}
	/*保存文件*/
	wrsz=fwrite((void*)buf,sizeof(unsigned char),CMOS_BUF_SIZE,fp);
	printf("向%s写入%lu字节，完成。\n",keybuf,wrsz);
	return;
}

/*恢复CMOS*/
void restore_cmos()
{
	FILE* fp;
	char* ret_str;
	unsigned char buf[CMOS_BUF_SIZE]={0};
	int rc;
	size_t rdsz;
	/*获取备份文件路径*/
	printf("\n备份文件路径：");
	ret_str=fgets(keybuf,MAX_KEY_BUF,stdin);
	keybuf_convert(keybuf);
	/*尝试打开文件*/
	fp=fopen(keybuf,"r");
	if(NULL==fp)
	{
		printf("文件%s打开失败！\n",keybuf);
		return;
	}
	/*读取文件*/
	rdsz=fread((void*)buf,sizeof(unsigned char),CMOS_BUF_SIZE,fp);
	printf("从%s读入%lu字节，完成。\n",keybuf,rdsz);
	/*确认恢复*/
	if(0==query_user("确定要恢复备份至CMOS内存？",q_opt,Q_OPT_NUM,1))
	{
		/*写入*/
		printf("写入中，请稍候...\n");
		rc=cmos_setalldata(buf);
		if(rc)
		{
			printf("出现错误，错误代码%d，请联系silei862@gmail.com\n",rc);
			return;
		}
	}
	return;
}
