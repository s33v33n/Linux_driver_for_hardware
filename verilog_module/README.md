# Verilog module 

The module provides a hardware-based implementation of CRC16CDMA2000 algorithm, utilizing an internal 250-byte storage buffer. Look-up table (lut.mem) was also implemented to improve performance.
Communication with module is trough dedicated registers:
- IN: byte-data input 
- CTRL: register to control module 
- STATE: module feedback (left buffer slots, counting, etc.)
- RESULT: result   


## Testing: 
Functionalities were verified using a testbench (gpioemu_tb.v) in the Icarus Verilog program. 

To access waveform analysis run following commands 
1. Compile: `iverilog -o gpioemu.vvp gpioemu.v gpioemu_tb.v`
2. Simulate: `vvp gpioemu.vvp`
3. Waveform: `<YOUR_PATH>\gtkwave.exe gpioemu.vcd gpioemu.gtkw`