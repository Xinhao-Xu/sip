SIB8500    18.04.02LTS x64 kernel 4.18.0-15-generic

從 gmb8350改來

libdialog.a　要抓source重新build CFLAGS+= -fPIC

初版板子只有　雙聲道

黑名內建wifi driver
vim /etc/modprobe.d/blacklist.conf
### DK ###
blacklist rtl8xxxu
===================================================================================================================================

變更網路代號回古早味
vim etc/default/grub
net.ifnames=0 biosdevname=0
===================================================================================================================================

切換login mode
To switch to text-only mode:
sudo systemctl set-default multi-user.target
To set it back to graphical mode:
sudo systemctl set-default graphical.target

vim /sbin/gototty,  gotodesk
===================================================================================================================================

autologin consloe tty1

18.04.02實驗OK, 搭配上述切換login mode 方式

首先建立 getty@tty1.service.d 目錄以及 override.conf 設定檔。
sudo mkdir /etc/systemd/system/getty@tty1.service.d
sudo vim /etc/systemd/system/getty@tty1.service.d/override.conf

[Service]
ExecStart=
ExecStart=-/sbin/agetty --noissue --autologin 帳戶名稱 %I 38400 linux
Type=idle

將上述內容貼至設定檔內，記得換成自己的帳戶名稱，並使用 :wq 指令儲存。


OR

Debian 自動登入 tty1

In /etc/systemd/logind.conf, changed #NAutoVTs=6 to NAutoVTs=1
Used systemctl edit getty@tty1 and added

[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin root --noclear %I 38400 linux
Enabled the service: systemctl enable getty@tty1.service
After rebooting, the login prompt was gone from tty1 and nothing else happened. It still showed the boot log.
On tty2-5, only a cursor appeared, no login prompt as before. Luckily, tty6 was still available to recover the system. So I did:

恢復:(兩個步驟都要, 不然卡死)
Disable the service: systemctl disable getty@tty1.service
Undid the change to /etc/systemd/logind.conf
===================================================================================================================================

ubuntu 18.04.02 x64 ->　sib8500　console mode一直印出下列出現在dmesg 的訊息

mmc0: Timeout waiting for hardware cmd interrupt.
mmc0: sdhci: ========== SDHCIREGISRER DUMP ==========

解法：
觀察msg level:
# cat /proc/sys/kernel/printk
8 4 1 7    將 level 8　下降成1

加在 /etc/profile.d/dk.sh
#!/bin/bash
dmesg -n 1 -> level 1 console 不會顯示
*待解: xwindow 出現語法錯誤
===================================================================================================================================

VGA test 需修改
test_items.c

int fun_TestVGA_002(char *dev)
y_field = (vinfo.yres_virtual/3-3);
修改繪圖最下條數
y_field = (vinfo.yres_virtual/3-10);	//sib8500
===================================================================================================================================

不使用dialog package 的修改方式:
 
1. diag_main()

	init_dialog() before scan
or
	inti_dialog() after scanBarCode_keyin_NoDiag

Modify function in diag.c:
init_Dialog()
{
	.
	.
	.
	//init_dialog(input, dialog_state.output); //don't init dialog
	.
	.
	.
}

deinit_Dialog()
{
	endwin();
	dlg_clear();
	//end_dialog();		//no need.
}
===================================================================================================================================






