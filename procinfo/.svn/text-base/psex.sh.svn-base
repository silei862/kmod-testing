#!/bin/sh
# 作业程序配套脚本
# 本脚本在 Ubuntu server(amd64) 10.04 LTS下测试通过
# 其他版本linux请用root用户登录,并去掉sudo

echo 请输入管理员密码,以便程序执行
if [ ! -c /dev/procinfo ]; then
	sudo insmod ./kmod/procinfo.ko
fi

while echo 请选择要执行的功能
do
echo "   "1.按照父子关系列举进程树
echo "   "2.按照线程组关系列举进程
echo "   "3.显示特定进程存储信息
echo "   "4.显示特定进程详细信息
echo "   "5.退出脚本程序
echo -n 选择功能:
read sel
case $sel in
	1) sudo ./psex -t
		;;
	2) sudo ./psex -g
		;;
	3) echo -n 请输入进程号:
		read pid
		sudo ./psex -m $pid
		;;
	4) echo -n 请输入进程号:
		read pid
		sudo ./psex -d $pid
		;;
	5)  break
		;;
	*) echo 输入错误
esac

done

sudo rmmod procinfo
echo 脚本程序正常退出
