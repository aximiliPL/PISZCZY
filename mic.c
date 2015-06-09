#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "pdm_filter.h"
#include "mic.h"
#include "misc.h"
#include "queue.h"

//sampling frequency
#define FS 16000

//PDM decimation factor
#define DECIMATION 64

//i2s clock is clock for mic
//clock for mic is calculated as Fs*decimation_factor
//so we have to divide with 32 (frame_length*num_channels)
//to get i2s sampling freq
#define I2S_FS ((FS*DECIMATION)/(16*2))

//uint16_t array length for filter input buffer
#define MIC_IN_BUF_SIZE ((((FS/1000)*DECIMATION)/8)/2)

//uint16_t array length for filter output buffer
#define MIC_OUT_BUF_SIZE (FS/1000)

static PDMFilter_InitStruct pdm_filter;
static uint16_t mic_in_buf[MIC_IN_BUF_SIZE];
static uint16_t mic_out_buf[MIC_OUT_BUF_SIZE];
static uint32_t mic_buf_index;
uint16_t sample;

void mp45dt02_init(void){
	GPIO_InitTypeDef gpio;
	NVIC_InitTypeDef nvic;
	I2S_InitTypeDef i2s;
	
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
    pdm_filter.LP_HZ=8000;
    pdm_filter.HP_HZ=10;
    pdm_filter.Fs=FS;
    pdm_filter.Out_MicChannels=1;
    pdm_filter.In_MicChannels=1;
    PDM_Filter_Init(&pdm_filter);

    //MP45DT02 CLK-PB10
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_10;
    gpio.GPIO_Mode=GPIO_Mode_AF;
    gpio.GPIO_OType=GPIO_OType_PP;
    gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);

    //MP45DT02 DOUT-PC3
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_3;
    gpio.GPIO_Mode=GPIO_Mode_AF;
    gpio.GPIO_OType=GPIO_OType_PP;
    gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

		//__enable_irq();
    nvic.NVIC_IRQChannel = SPI2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
		//NVIC_EnableIRQ(SPI2_IRQn);

    //I2S2 config
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    i2s.I2S_AudioFreq=I2S_FS;
    i2s.I2S_Standard=I2S_Standard_LSB;
    i2s.I2S_DataFormat=I2S_DataFormat_16b;
    i2s.I2S_CPOL=I2S_CPOL_High;
    i2s.I2S_Mode=I2S_Mode_MasterRx;
    i2s.I2S_MCLKOutput=I2S_MCLKOutput_Disable;
    I2S_Init(SPI2, &i2s);
		
		SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
    I2S_Cmd(SPI2, ENABLE);
}    

void mp45dt02_start(void){
    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}

void SPI2_IRQHandler(void){
	uint32_t i;
	
	  if(SPI_GetITStatus(SPI2, SPI_I2S_IT_RXNE)){
			 
			GPIOD->ODR |= 0x00002000; // check orange led
			 
			  sample = SPI_I2S_ReceiveData(SPI2);
        mic_in_buf[mic_buf_index++] = HTONS(sample);
        if(mic_buf_index == MIC_IN_BUF_SIZE){
            mic_buf_index=0;
            PDM_Filter_64_LSB((uint8_t *)mic_in_buf, mic_out_buf, 40, &pdm_filter);
            for(i=0; i<MIC_OUT_BUF_SIZE; i++){
                enqueue(mic_out_buf[i]);
            }
        }

    }
}

