# 内核模块Makefile
obj-m += ioport.o
ioport-objs := ioport_fops.o ioport_main.o
CURRENT_PATH := $(shell pwd)                                  #获取当前路径
LINUX_VERSION := $(shell uname -r)                            #获取内核版本号
LINUX_HEADER_PATH := /usr/src/linux-headers-$(LINUX_VERSION)  #获取内核头文件路径

all:
	make -C $(LINUX_HEADER_PATH) M=$(CURRENT_PATH) modules    #模块编译
clean:

	make -C $(LINUX_HEADER_PATH) M=$(CURRENT_PATH) clean   	  #清理

