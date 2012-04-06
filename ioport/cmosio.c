/**********************************************************
 * File:		cmosio.c
 * Author:		思磊
 * E-mail:		silei862@gmail.com
 * Date:		2012-2-5
 * Description:	cmos读写库接口函数定义，提供基本读写函数
 *
 **********************************************************/

#include "cmosio.h"

/*创建cmos读写对象*/
CMOS_OBJ_P cmos_create_obj()
{
	CMOS_OBJ_P pobj;
	/*申请pobj空间*/
	pobj=(CMOS_OBJ_P)malloc(sizeof(CMOS_OBJ_T));
	if(NULL==pobj)
		return NULL;
	memset((void*)pobj,0,sizeof(CMOS_OBJ_T));
	/*打开io设备*/
	pobj->file_dev=open(DEV_IO_NAME,O_RDWR);
	if(pobj->file_dev<=0)
	{
		free((void*)pobj);
		return NULL;
	}
	/*初始化各个成员*/
	pobj->buf=NULL;
	pobj->req_mode=REQ_MODE_NO;
	pobj->rd_delay=DEF_RD_DELAY;
	pobj->wr_delay=DEF_WR_DELAY;
	pobj->start_addr=MIN_CMOS_ADDR;
	pobj->end_addr=MIN_CMOS_ADDR;
	pobj->error_code=CODE_NULL;
	return pobj;
}

/*销毁cmos读写对象*/
void cmos_destroy_obj(CMOS_OBJ_P pobj)
{
	if(pobj)
	{
		close(pobj->file_dev);
		free((void*)pobj);
	}
	return;
}

/*获取对象调试信息*/
void cmos_debug_info(CMOS_OBJ_P pobj)
{
	printf("**********Current Info**************\n");
	printf("File Device:%d\n",pobj->file_dev);
	printf("Request Mode:%u\n",pobj->req_mode);
	printf("Read Delay:%lu\n",pobj->rd_delay);
	printf("Write Delay:%lu\n",pobj->wr_delay);
	printf("Start Address:%u\n",pobj->start_addr);
	printf("End	Address:%u\n",pobj->end_addr);
	printf("Buffer Address:%lx\n",(unsigned long)pobj->buf);
	printf("Error Code:%d\n",pobj->error_code);
	return;
}

/*对象参数设置*/
int cmos_io_opt(CMOS_OBJ_P pobj,int opt_id,long arg)
{
	if(NULL==pobj)
		return NULL_OBJECT;

	switch(opt_id)
	{
		/*IO开启方式*/
		case OPT_REQ_MODE:
			/*参数检查*/
			if(arg<0 || arg>1)
			{
				pobj->error_code=MODE_ARG_ERROR;
				return RET_ERROR;
			}
			pobj->req_mode=(unsigned char)arg;
			break;

		/*设置读取延时*/
		case OPT_RD_DELAY:
			/*参数检查*/
			if(arg<MIN_DELAY || arg>MAX_DELAY)
			{
				pobj->error_code=DELAY_ARG_ERROR;
				return RET_ERROR;
			}
			pobj->rd_delay=arg;
			break;
		
		/*设置写入延时*/
		case OPT_WR_DELAY:
			/*参数检查*/
			if(arg<MIN_DELAY || arg>MAX_DELAY)
			{
				pobj->error_code=DELAY_ARG_ERROR;
				return RET_ERROR;
			}
			pobj->wr_delay=arg;
			break;
		/*设置读取写入首地址*/
		case OPT_START_ADDR:
			if(arg<MIN_CMOS_ADDR || arg>MAX_CMOS_ADDR)
			{
				pobj->error_code=ADDR_ARG_ERROR;
				return RET_ERROR;
			}
			pobj->start_addr=(unsigned char)arg;
			break;
		/*设置读取写入尾地址*/
		case OPT_END_ADDR:
			if(arg<MIN_CMOS_ADDR || arg>MAX_CMOS_ADDR)
			{
				pobj->error_code=ADDR_ARG_ERROR;
				return RET_ERROR;
			}
			pobj->end_addr=(unsigned char)arg;
			break;
		/*设置读写缓冲区*/
		case OPT_RDWR_BUF:
			{
				unsigned char* pbuf=(unsigned char*)arg;
				if(NULL==pbuf)
				{
					pobj->error_code=BUF_ARG_ERROR;
					return RET_ERROR;
				}
				pobj->buf=pbuf;
			}
			break;

		default:
			pobj->error_code=OPT_ID_ERROR;
			return RET_FAILURE;
	}
	pobj->error_code=CODE_NULL;
	return RET_SUCCESS;
}

/*将设置参数写入IO*/
int cmos_io_set(CMOS_OBJ_P pobj)
{
	int cmd;
	if(NULL==pobj)
		return NULL_OBJECT;
	if(pobj->file_dev<=0)
	{
		pobj->error_code=FILEDEV_ERROR;
		return RET_FAILURE;
	}
	/*使用ioctl进行参数设置*/
	/*IO请求模式*/
	if(REQ_MODE_YES==pobj->req_mode)
		cmd=SET_REQUEST;
	else
		cmd=SET_NONREQ;
	if(ioctl(pobj->file_dev,cmd)<0)
	{
			pobj->error_code=SET_REQ_MODE_FAILED;
			return RET_FAILURE;
	}
	/*设置读取延时*/
	if(ioctl(pobj->file_dev,SET_RD_DELAY,pobj->rd_delay)<0)
	{
		pobj->error_code=SET_RD_DELAY_FAILED;
		return RET_FAILURE;
	}
	/*设置写入延时*/
	if(ioctl(pobj->file_dev,SET_WR_DELAY,pobj->wr_delay)<0)
	{
		pobj->error_code=SET_WR_DELAY_FAILED;
		return RET_FAILURE;
	}
	pobj->error_code=CODE_NULL;
	return RET_SUCCESS;
}

/*读取cmos内容*/
int cmos_read(CMOS_OBJ_P pobj)
{
	int rc;
	unsigned char cmos_addr;
	unsigned char* pbuf;
	/*检查进行必要检查*/
	if(NULL==pobj)
		return NULL_OBJECT;
	pbuf=pobj->buf;
	if(NULL==pbuf)
	{
		pobj->error_code=NULL_BUF;
		return RET_FAILURE;
	}
	/*开始读取*/
	for(cmos_addr=pobj->start_addr;cmos_addr<=pobj->end_addr;cmos_addr++)
	{
		/*设置CMOS地址端口*/
		if(ioctl(pobj->file_dev,SET_PORT,CMOS_ADDR_PORT)<=0)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*将要读取的CMOS地址写入地址端口*/
		rc=(int)write(pobj->file_dev,&cmos_addr,1);
		if(rc<1)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*设置CMOS数据端口*/
		if(ioctl(pobj->file_dev,SET_PORT,CMOS_DATA_PORT)<=0)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*从数据端口读出一个字节*/
		rc=(int)read(pobj->file_dev,pbuf,1);
		if(rc<1)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		pbuf++;
	}
	return RET_SUCCESS;
}

/*将数据写入CMOS*/
int cmos_write(CMOS_OBJ_P pobj)
{
	int rc;
	unsigned char cmos_addr;
	unsigned char* pbuf;
	/*检查进行必要检查*/
	if(NULL==pobj)
		return NULL_OBJECT;
	pbuf=pobj->buf;
	if(NULL==pbuf)
	{
		pobj->error_code=NULL_BUF;
		return RET_FAILURE;
	}
	/*开始写入*/
	for(cmos_addr=pobj->start_addr;cmos_addr<=pobj->end_addr;cmos_addr++)
	{
		/*设置CMOS地址端口*/
		if(ioctl(pobj->file_dev,SET_PORT,CMOS_ADDR_PORT)<=0)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*将要写入的CMOS地址写入地址端口*/
		rc=(int)write(pobj->file_dev,&cmos_addr,1);
		if(rc<1)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*设置CMOS数据端口*/
		if(ioctl(pobj->file_dev,SET_PORT,CMOS_DATA_PORT)<=0)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		/*向数据端口写入一个字节*/
		rc=(int)write(pobj->file_dev,pbuf,1);
		if(rc<1)
		{
			pobj->error_code=FILEDEV_ERROR;
			return RET_FAILURE;
		}
		pbuf++;
	}
	return RET_SUCCESS;
}

/*读取所有CMOS数据*/
int cmos_read_all(CMOS_OBJ_P pobj,unsigned char* buf)
{
	int rc;
	int opt;
	long para[OPT_NUM]={0}; 
	if(NULL==pobj)
		return NULL_OBJECT;
	/*参数初始化*/
	para[OPT_REQ_MODE]=REQ_MODE_NO;
	para[OPT_RD_DELAY]=1;
	para[OPT_WR_DELAY]=1;
	para[OPT_START_ADDR]=MIN_CMOS_ADDR;
	para[OPT_END_ADDR]=MAX_CMOS_ADDR;
	para[OPT_RDWR_BUF]=(long)buf;
	/*参数设置*/
	for(opt=0;opt<OPT_NUM;opt++)
		if(rc=cmos_io_opt(pobj,opt,para[opt]))
			return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*读取所有CMOS数据*/
	rc=cmos_read(pobj);
	if(rc)
		return rc;
	return RET_SUCCESS;
}

/*写入所有CMOS数据*/
int cmos_write_all(CMOS_OBJ_P pobj,unsigned char* buf)
{
	int rc;
	int opt;
	long para[OPT_NUM]={0}; 
	if(NULL==pobj)
		return NULL_OBJECT;
	/*参数初始化*/
	para[OPT_REQ_MODE]=REQ_MODE_NO;
	para[OPT_RD_DELAY]=1;
	para[OPT_WR_DELAY]=1;
	para[OPT_START_ADDR]=MIN_CMOS_ADDR;
	para[OPT_END_ADDR]=MAX_CMOS_ADDR;
	para[OPT_RDWR_BUF]=(long)buf;
	/*参数设置*/
	for(opt=0;opt<OPT_NUM;opt++)
		if(rc=cmos_io_opt(pobj,opt,para[opt]))
			return rc;
	rc=cmos_io_set(pobj);
	if(rc)
		return rc;
	/*写入所有CMOS数据*/
	rc=cmos_write(pobj);
	if(rc)
		return rc;
	return RET_SUCCESS;
}
