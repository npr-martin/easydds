#!/bin/bash
# 检查是否传入了参数
if [ $# -eq 0 ]; then
    gnome-terminal --tab -- ./EDTest
    exit 0
fi

# 从第一个参数获取循环次数
COUNT=$1

if [ "$COUNT" -eq 2 ]; then
    gnome-terminal --tab -- ./EDTest publisher
    gnome-terminal --tab -- ./EDTest subscriber
elif [ "$COUNT" -eq 3 ]; then
    gnome-terminal --tab -- ./EDTest ds_publisher
    gnome-terminal --tab -- ./EDTest ds_subscriber
    gnome-terminal --tab -- ./EDTest ds_server
else
    # 循环执行命令，次数由参数决定
    for ((i=1; i<=COUNT; i++)); do
        gnome-terminal --tab -- ./EDTest
    done
fi


