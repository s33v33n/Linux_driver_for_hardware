#   make         - Verilog -> C++ (Verilator) -> libcrcmodule.a
#   make clean   - remove all files


# Verilog source
VERILOG_SRC := ../verilog_module/crcmodule.v

# Verilator output directory and final library (linked into QEMU)
OBJ_DIR := obj_dir
LIB := libcrcmodule.a

VERILATOR ?= verilator
VERILATOR_ROOT ?= $(shell $(VERILATOR) --getenv VERILATOR_ROOT)

# -fPIC: the library is linked into QEMU
CXXFLAGS += -O2 -Wall -fPIC -I$(OBJ_DIR) -I$(VERILATOR_ROOT)/include -I$(VERILATOR_ROOT)/include/vltstd

# objects built by the Verilator makefile: model + Verilator runtime
MODEL_OBJS := $(OBJ_DIR)/Vcrcmodule__ALL.o $(OBJ_DIR)/verilated.o $(OBJ_DIR)/verilated_threads.o

# Default target
all: $(LIB)

# 1. Verilog -> C++
$(OBJ_DIR)/Vcrcmodule.mk: $(VERILOG_SRC)
	$(VERILATOR) --cc $< --Mdir $(OBJ_DIR) -CFLAGS -fPIC

# 2. compile generated C++ and Verilator runtime
$(MODEL_OBJS): $(OBJ_DIR)/Vcrcmodule.mk
	$(MAKE) -C $(OBJ_DIR) -f Vcrcmodule.mk $(notdir $(MODEL_OBJS))

# 3. C interface (wrapper)
crcmodule_model.o: crcmodule_model.cpp crcmodule_model.h $(OBJ_DIR)/Vcrcmodule.mk
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. library for QEMU
$(LIB): crcmodule_model.o $(MODEL_OBJS)
	$(AR) rcs $@ $^

# Remove all generated files
clean:
	rm -rf $(OBJ_DIR) *.o $(LIB)

# these commands are just information for make, not output files
.PHONY: all clean
