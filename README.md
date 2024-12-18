# easydds

Project EasyDDS

############配置环境#####
cd /etc/ld.so.conf.d
sudo touch easydds.conf
sudo vim easydds.conf
写入下方路径（绝对路径）
[installPath]/install/lib
[installPath]/easydds/build
保存后执行
sudo ldconfig

使用./compiler.sh进行编译，可添加参数“-jn”进行多核编译，n为多核核数
