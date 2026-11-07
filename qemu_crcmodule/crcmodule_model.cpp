// crcmodule_model - wraps the Verilator model in C functions
// eval() - updates the module 
// clock_cycle() - one clock cycle 

#include "Vcrcmodule.h"
#include "verilated.h"
#include "crcmodule_model.h"

static Vcrcmodule *model;

// one clock cycle: rising + falling edge
static void clock_cycle(void)
{
    model->clk = 1;
    model->eval();
    model->clk = 0;
    model->eval();
}

void crcmodule_model_init(void)
{
    model = new Vcrcmodule;
    model->clk = 0;
    model->n_reset = 1;
    model->eval();
}

void crcmodule_model_reset(void)
{
    model->n_reset = 0;
    model->eval();
    model->n_reset = 1;
    model->eval();
}

void crcmodule_model_write(uint16_t addr, uint32_t data)
{
    model->saddress = addr;
    model->sdata_in = data;
    model->swr = 1;
    clock_cycle();
    model->swr = 0;
}

uint32_t crcmodule_model_read(uint16_t addr)
{
    model->saddress = addr;
    model->srd = 1;
    clock_cycle();
    model->srd = 0;
    return model->sdata_out;
}

void crcmodule_model_tick(int cycles)
{
    while (cycles-- > 0) {
        clock_cycle();
    }
}
