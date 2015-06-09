#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "misc.h"

#include "mic.h"
#include "pdm_filter.h"

#include "dac.h"
#include "codec.h"

#include "queue.h"

uint16_t audio;
int buff;
int temp;
PDMFilter_InitStruct Filter;


int main(void)
{
volatile uint32_t j;
volatile uint32_t i;
RCC_ClocksTypeDef  rcc_clocks;

// leds
RCC->AHB1ENR|=0x00000009;
GPIOD->MODER|=0x55000000;
	
GPIOD->BSRRL=0x1000; //green test

// check all clocks
RCC_GetClocksFreq(&rcc_clocks);
RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; 
RCC->CR |= RCC_CR_PLLI2SON;

cs43l22_myinit();
fifo_init();	
mp45dt02_init();
mp45dt02_start();
	
//fill queve a little
for(j=0; j<1000000; j++);
	
cs43l22_start();

while(1){
	
	while(IsFifoEmpty()){
		}
    audio=dequeue();
    cs43l22_write_sound_data(audio, audio);
	}
}
