#   make              - build the kernel module (cross-compiled for RISC-V)
#   make clean        - remove artifacts
#   make serve        - share directory over HTTP for QEMU
#
#   --- INSIDE QEMU ---
#   wget http://10.0.2.2:8000/crcmodule.ko  	- download module from host
#   insmod crcmodule.ko                      	- load module
#   rmmod crcmodule                          	- unload module and shut down QEMU


# Module name
MODULE_NAME := crcmodule

# Source file
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-y := kernel_module.o

# Current working directory
PWD := $(shell pwd)

# Buildroot output (kernel sources and cross-compiler for RISC-V)
BR_OUT ?= $(PWD)/../build/buildroot-2025.02.19/output
KDIR ?= $(BR_OUT)/build/linux-6.12.27
ARCH ?= riscv
CROSS_COMPILE ?= $(BR_OUT)/host/bin/riscv64-buildroot-linux-gnu-

# Default target: build the module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

# Remove all build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) clean

# Share module over HTTP 
serve:
	python3 -m http.server 8000 --bind 127.0.0.1

# these commands are just information for make, not output files
.PHONY: all clean serve
