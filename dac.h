#ifndef __DAC_H_ 
#define __DAC_H_

#include  <stdint.h>

void cs43l22_myinit(void);
void cs43l22_start(void);
void cs43l22_write_reg(uint8_t reg, uint8_t data);
uint8_t cs43l22_read_reg(uint8_t reg);
void cs43l22_write_sound_data(uint16_t data_l, uint16_t data_r);
void SPI3_IRQHandler(void);

#endif
