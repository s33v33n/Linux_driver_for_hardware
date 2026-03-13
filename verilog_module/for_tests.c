#include <stdio.h> // input output operations printf, scranf...  
#include <stdlib.h> // memory management malloc, free... 
#include <stdint.h> // variables types uint8_t....

#define INIT 0xFFFF
#define POLY 0xC867

void load_data(uint16_t bytes_num, uint8_t *data_in);
uint16_t calc_crc(uint16_t bytes_num, uint8_t *data_in, uint16_t *lut_table);

int main(void) {
    uint16_t LUT_TABLE[256];
    uint16_t result = 0;

    // LUT table generation
    for (uint16_t i = 0; i < 256; i++) {
        uint16_t crc = i << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ POLY;
            } else {
                crc <<= 1;
            }
        }
        LUT_TABLE[i] = crc;
        //printf("CRC dla %u: 0x%04X\n", i, LUT_TABLE[i]);
    }

    // INPUT data
    uint16_t bytes_num = 250;
    uint8_t *data_in = malloc(bytes_num * sizeof(uint8_t));
    if (data_in == NULL) {
        return 1;
    }

    load_data(bytes_num, data_in);
    result = calc_crc(bytes_num, data_in, LUT_TABLE);

    printf("CRC dla %u liczb to: 0x%04X\n", bytes_num, result);

    free(data_in);
    return 0;
}

void load_data(uint16_t bytes_num, uint8_t *data_in) {
    //uint16_t check = 0x31;
    for (uint16_t i = 0; i < bytes_num; i++) {
        data_in[i] = (uint8_t)(i);
        printf("liczba %u: 0x%02X\n", i, data_in[i]);
    }
}

uint16_t calc_crc(uint16_t bytes_num, uint8_t *data_in, uint16_t *lut_table) {
    uint16_t crc = INIT;

    for (uint16_t i = 0; i < bytes_num; i++) {
        uint8_t index = (uint8_t)((crc >> 8) ^ data_in[i]);
        crc = (crc << 8) ^ lut_table[index];
    }

    return crc;
}