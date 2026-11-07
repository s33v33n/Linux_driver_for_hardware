/*
 * crcmodule_model - interface to the Verilator model of crcmodule.v
 *
 * Bus contract (same as crcmodule_tb.v):
 *   write: saddress + sdata_in, swr = 1 for one clock cycle
 *   read:  saddress, srd = 1 for one clock cycle, then sdata_out
 */

#ifndef CRCMODULE_MODEL_H
#define CRCMODULE_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     crcmodule_model_init(void);                            // create the model
void     crcmodule_model_reset(void);                           // n_reset pulse
void     crcmodule_model_write(uint16_t addr, uint32_t data);   // one bus write
uint32_t crcmodule_model_read(uint16_t addr);                   // one bus read
void     crcmodule_model_tick(int cycles);                      // extra clock cycles

#ifdef __cplusplus
}
#endif

#endif // CRCMODULE_MODEL_H
