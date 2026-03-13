# Linux hardware driver 
This project is designed to be tested on an external Linux system with a custom-built QEMU module for hardware.

### gpioemu.v - Verilog Module 
This module performs hardware-based CRC16-CDMA2000 calculations

### kernel_module.c - Linux Kernel Module 
This driver maps the simulated hardware's physical memory into the kernel space to enable CPU-to-hardware communication. It exposes the hardware registers through the SYSFS filesystem.

### main.c -  Userspace Application
The application provides a tool to access various system functionalities. 