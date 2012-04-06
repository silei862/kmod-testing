#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "kmod/proc_cmd.h"

#define PROC_PORT "/dev/procinfo"	/*设备名称*/

enum _PSEX_CONST{
	RD_BUF_SIZE	= 0x8000,			/*读取缓冲区大小*/
};

/*显示帮助*/
void print_help(char* error_option)
{
	printf("psex:%s\n",error_option);
	printf("psex用法：\n");
	printf("    psex [选项] [进程pid]\n");
	printf("    -t    按父子关系打印进程树。\n");
	printf("    -g    按线程组关系打印所有进程。\n");
	printf("    -m n  打印pid为n的进程内存详情。\n");
	printf("    -d n  打印pid为n的进程详细信息。\n");
	printf("    -h    打印本帮助。\n");
	printf("Report bugs to <silei862@gmail.com>\n");
}

int main(int argc,char** argv)
{
	int cmd=0;
	long arg=0;
	
	/*参数检查*/
	if(argc<2)
	{
		print_help("缺少参数");
		return EXIT_FAILURE;
	}
	/*参数长度检查*/
	if(strlen(argv[1])<2)
	{
		print_help("缺少参数");
		return EXIT_FAILURE;
	}
	/*参数解析*/
	if('-'==argv[1][0])
	{
		switch(argv[1][1])
		{
			case 't':
				cmd = CMD_PROC_TREE;
				break;
			case 'g':
				cmd = CMD_THREAD_GROUP;
				break;
			case 'm':
				if(argc<3)
				{
					print_help("请指定进程号。");
					return EXIT_FAILURE;
				}
				cmd = CMD_MEM_STATE;
				arg = atol(argv[2]);
				break;
			case 'd':
				if(argc<3)
				{
					print_help("请指定进程号。");
					return EXIT_FAILURE;
				}
				cmd = CMD_PROC_DETAIL;
				arg = atol(argv[2]);
				break;
			case 'h':
				print_help("请阅读Readme获取更详细信息。");
				return EXIT_SUCCESS;
			default:
				print_help("无法识别的选项");
				return EXIT_FAILURE;
		}
	}
	else
	{
		print_help("参数错误");
		return EXIT_FAILURE;
	}

	/*打开字符设备*/
	int file_dev;
	if((file_dev = open(PROC_PORT,O_RDWR))<=0)
	{
		printf("打开 %s 错误!ret_code=%x\n",PROC_PORT,file_dev);
		return EXIT_FAILURE;
	}
	/*写入控制命令*/
	ioctl(file_dev,cmd,arg);
	
	/***读取执行结果***/
	char* rd_buf; 
	int num_rd;
	if(NULL==(rd_buf = (char*)malloc(sizeof(char)*RD_BUF_SIZE)))
	{
		printf("内存分配错误!!!\n");
		return EXIT_FAILURE;
	}
	/*读缓冲先清零*/
	memset(rd_buf,0x00,sizeof(char)*RD_BUF_SIZE);
	/*读取并显示*/
	num_rd = read(file_dev,rd_buf,RD_BUF_SIZE);
	if(num_rd)
		printf("%s\n",rd_buf);
	else
		printf("缓冲区空：无需求的信息，或指定进程不存在。\n");
	/*关闭设备*/
	close(file_dev);	
	return EXIT_SUCCESS;
}
