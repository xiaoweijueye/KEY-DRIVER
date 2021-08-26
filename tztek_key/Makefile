ifeq (${KERNELRELEASE}, )

KERNELDIR=/home/linux/work/nvidia_jetpack_4_4/source/kernel_src/kernel/kernel-4.9
#KERNELDIR=/lib/modules/$(shell uname -r)/build
PWD=$(shell pwd)

modules:
	make -C ${KERNELDIR} M=${PWD} modules

clean:
	make -C ${KERNELDIR} M=${PWD} clean

else

obj-m += tztek_key.o

endif
