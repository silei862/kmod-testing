# 内核模块Makefile
obj-m += ioport.o
ioport-objs := ioport_fops.o ioport_main.o
CURRENT_PATH := $(shell pwd)
LINUX_VERSION := $(shell uname -r)
LINUX_HEADER_PATH := /lib/modules/$(LINUX_VERSION)/build

all:
	make -C $(LINUX_HEADER_PATH) M=$(CURRENT_PATH) modules    #模块编译
clean:

	make -C $(LINUX_HEADER_PATH) M=$(CURRENT_PATH) clean   	  #清理

