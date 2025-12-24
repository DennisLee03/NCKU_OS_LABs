# Kernel Modules Notes
## 簡介
* Linux kernel module 能根據需求，在 runtime 時(OS 已經跑起來了) dynamically load or unload.
* These loadable kernel modules run in `priviledged kernel mode`. In theory, there is no restriction on what a kernael module is allowed to do.
* A notable example is seen in the `device driver` module, which facilitates kernel interaction with hardware components linked to the system.
* 如果沒有 loadable modules 的方式的話，現行的 Linux kernel 會更像是 monolithic kernels 的概念，此時若要新增新的功能的話: 
    1. 就需要想辦法把東西 intergrate 進整個 kernel code 的某處
    2. 而且還要重花時間 compile kernel

## Basic Commands
```bash
# install and remove modules
$ modprobe
$ insmod <module_name>.ko
$ rmmod <module_name>

# see module info
$ modinfo <module_name>.ko

# module dependencies
$ depmod

# list modules
$ lsmod
$ cat /proc/modules

# we can pipe and grep the info we want
$ lsmod | grep <module_name>
```

## 先認識在編寫與使用 kernel modules 一些的概念、關鍵字、指令
1. `Modversioning(模組版本管控)`: 在編譯與安裝 modules 時，要注意 modules 所支援的 kernel 版本
    * `CONFIG_MODVERSIONS`
2. `Using the X Window System`: X Window 是指 Linux GUI 底層的系統。
    * 不建議使用 GUI 的方式，去查看 modules 的輸出
    * 因為 modules 的東西通常會輸出到 kernel log 當中
    ```bash
    $ dmesg          # to read kernel log ring buffer
    $ journalctl -k  # to check systemd journal for kernel message
    ```
3. `SecureBoot`: 
    * 有些 Linux distributions 的 kernel 會啟動 secure boot 的功能，裡面可能有 `UEFI secure boot` 之類的安全性功能，會擋下一些沒有簽章的 modules，以避免惡意程式裝進系統，也因此會造成無法安裝某些你想要加的 modules
    * 最簡單的解法就是進 BIOS/UEFI 裡面把 secure boot 關掉
    * 另一種方式就是要生 key 給 modules 使用

## Install Necessary Headers
```bash
$ uname -r                                   # what kernel your os is

$ sudo apt-get update
$ apt-cache search linux-headers-`uname -r`  # what header your os needs
$ sudo apt-get install linux-headers-`uname -r`
```

## Basics

### Basic Makefile Template
* Makefile
    ```make
    obj-m += hello-1.o   
    
    PWD := $(CURDIR)  
    
    all:  
        $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
    
    
    clean: 
        $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
    ```
* `Kbuild`: Linux kernel 的 build system，可以用來編譯 kernel、kernel modules
* `obj-m += <object_name>`: 用來告訴 `Kbuild` 要被編譯的目標是誰
* `CURDIR`: GNU Make 的內建變數，通常就是目前這份 Makefile 所在的絕對路徑
* `-C /lib/modules/$(shell uname -r)/build `: 切換目錄到 `Kbuild` 的所在處
* `M=$(PWD)`: 告訴 `Kbuild` 你的外部模組在哪裡
* `modules`: 只編譯模組
* [more info](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/modules.rst)

### Basic Source Code Structure
* hello-1.c
    ```C
    /*
    * hello-1.c - The simplest kernel module.
    */
    #include <linux/module.h> /* Needed by all modules */
    #include <linux/printk.h> /* Needed for pr_info() */

    int init_module(void)
    {
        pr_info("Hello world 1.\n");

        /* A nonzero return means init_module failed; module can't be loaded. */
        return 0;
    }

    void cleanup_module(void)
    {
        pr_info("Goodbye world 1.\n");
    }

    MODULE_LICENSE("GPL");
    ```
* How it works?
    * 撰寫自己的 kernel module source codes 時，一定要有兩個 functions:
        1. `init_module()`: 當下 `insmod` 命令時，就會呼叫這個函數，將此 module 初始化，即 allocate 一些 kernel 資源、註冊 handler 等
        2. `cleanup_module()`: 當下 `rmmod` 命令時，在其刪除掉此 module 前，會先 call 此函數，把資源釋放、取消註冊 handler 等
    * headers
        1. `<linux/module.h>`: 撰寫 module 相關的功能會需要他
        2. [`<linux/printk.h>`](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/printk.h): 常用到裡面的 logging 的 macro 去 print 訊息到 kernel log 中

## [TODO] lkmpg 4.2