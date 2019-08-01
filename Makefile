obj-m += vmhook.o
CBDIR = src
vmhook-objs := ${CBDIR}/main.o ${CBDIR}/hook.o
MCFLAGS += -std=gnu11
ccflags-y += ${MCFLAGS}
CC += ${MCFLAGS}

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
