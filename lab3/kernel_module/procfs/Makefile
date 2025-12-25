obj-m += procfs-1.o
obj-m += procfs-2.o
   
PWD := $(CURDIR)  
   
all:  
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
  
clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

myinfo:
	@echo $(CURDIR)
	@echo $(MAKE)
	@echo $(PWD)