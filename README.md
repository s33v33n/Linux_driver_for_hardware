# Linux driver for hardware - CRC16 module in QEMU

A hardware CRC16-CDMA2000 module written in Verilog, added as a memory-mapped device to an emulated **RISC-V** computer (QEMU `virt` board) and controlled from **Linux** through a custom kernel driver.

## How it works
```
 user (shell)          
      |
 Linux driver         
      |
 QEMU (RISC-V virt)   
      |
 Verilator model       
      |
 Verilog module        
```

The module registers are at `0x100200-0x10021F`, on top of the QEMU device (`0x100000`). The driver can shut down QEMU with `rmmod crcmodule`.

| Register | Address | sysfs file | Description |
|---|---|---|---|
| IN | `0x100200` | `in` (write) | one data byte |
| STATE | `0x100208` | `state` (read) | module state bits |
| RESULT | `0x100210` | `result` (read) | CRC16 result, `0xFFFF` while not ready |
| CTRL | `0x100218` | `ctrl` (write) | `0x00` - calculate CRC |

Details of the registers and state bits: [verilog_module/README.md](verilog_module/README.md).

## Repository
| Folder | Content |
|---|---|
| `verilog_module/` | Verilog module, testbench, GTKWave layout |
| `qemu_crcmodule/` | Verilator wrapper (C interface), QEMU device, patch for QEMU 9.2.0 |
| `kernel_module/` | Linux driver (sysfs) and its Makefile |
| `rootfs_overlay/` | files copied into the Linux root filesystem: driver, boot script, test script |
| `config/` | Buildroot configuration `crcmodule_defconfig` |

### Verilog module - `verilog_module/`
- CRC16-CDMA2000: polynomial `0xC867`, initial value `0xFFFF`,
- buffer for up to 250 data bytes, one byte calculated per clock cycle (look-up table generated at start-up),
- one clock domain with asynchronous reset,
- verified with a self-checking testbench in Icarus Verilog, lint clean in Verilator.

### QEMU device - `qemu_crcmodule/`
| File | Description |
|---|---|
| `crcmodule_model.cpp/.h` | C functions around the Verilator model: init, reset, bus write, bus read, clock cycles |
| `crcmodule.c` | QEMU device: memory window `0x20` bytes, every CPU access = one bus cycle of the module |
| `Makefile` | Verilog -> C++ (Verilator) -> `libcrcmodule.a` |
| `patches/qemu/` | patch for QEMU: adds the device to the build and places it on the RISC-V `virt` board |

QEMU is built only from C, so the C++ model is linked as a ready static library `libcrcmodule.a`. In QEMU the module clock does not run by itself - each register access gives one clock cycle and a write to CTRL gives extra 260 cycles, so the result is ready before the CPU reads STATE.

### Linux driver - `kernel_module/`
- `ioremap` of the physical address `0x100000`,
- sysfs directory `/sys/kernel/crcmodule_sysfs/` with files `in`, `ctrl`, `state`, `result`,
- `rmmod crcmodule` removes the sysfs files and shuts down QEMU.

### Linux - Buildroot
- Buildroot **2025.02.19 LTS**, kernel **6.12.27**, OpenSBI 1.6, ext4 root filesystem, QEMU 9.2.0 built by Buildroot,
- `rootfs_overlay/` adds to the system:
  - `/lib/modules/crcmodule.ko` - the driver,
  - `/etc/init.d/S90crcmodule` - loads the driver at boot,
  - `/usr/bin/crcmodule_test` - test script.

## Build


The Buildroot folder must be `build/buildroot-2025.02.19` inside the repository - relative paths in the configuration and in the QEMU patch depend on it.

```bash
# 1. Verilator model -> libcrcmodule.a (needed by QEMU, build it first)
make -C qemu_crcmodule

# 2. Buildroot
mkdir build && cd build
wget https://buildroot.org/downloads/buildroot-2025.02.19.tar.gz
tar xf buildroot-2025.02.19.tar.gz
cd buildroot-2025.02.19

# 3. configuration from the repository and full build (takes long the first time)
make defconfig BR2_DEFCONFIG=../../config/crcmodule_defconfig
make

# 4. run
output/images/start-qemu.sh
```

## Usage
Log in as `root`. The driver is already loaded.

Test of all functions:
```bash
crcmodule_test           
```

Shut down:
```bash
rmmod crcmodule           # QEMU exits
```