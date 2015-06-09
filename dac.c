#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"
#include "misc.h"

#include "queue.h"
#include "codec.h"

#define FS 16000

static uint8_t lr;
static uint16_t data;

void cs43l22_start(void){
    SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);
}

void cs43l22_write_sound_data(uint16_t data_l, uint16_t data_r){
    while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI3, data_l);
    while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI3, data_r);
}

void cs43l22_myinit(void){
		uint32_t j;
    GPIO_InitTypeDef gpio;
    NVIC_InitTypeDef nvic;
	    I2S_InitTypeDef i2s;
	    I2C_InitTypeDef i2c;
	    uint8_t reg = 0xFF;
			uint8_t CodecCommandBuffer[5];
	
    nvic.NVIC_IRQChannel = SPI3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    //CS43L22 /RESET(PD4)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_4;
    gpio.GPIO_Mode=GPIO_Mode_OUT;
    gpio.GPIO_OType=GPIO_OType_PP;
    gpio.GPIO_PuPd=GPIO_PuPd_DOWN;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpio);

    //CS43L22 I2C SDA(PB9) in SCL(PB6)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_9;
    gpio.GPIO_Mode=GPIO_Mode_AF;
    gpio.GPIO_OType=GPIO_OType_OD;
    gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

    //CS43L22 I2S3 WS(PA4);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_4;
    gpio.GPIO_Mode=GPIO_Mode_AF;
    gpio.GPIO_OType=GPIO_OType_PP;
    gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3);

    //CS43L22 I2S3 MCK(PC7), SCK(PC10), SD(PC12)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin=GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_12;
    gpio.GPIO_Mode=GPIO_Mode_AF;
    gpio.GPIO_OType=GPIO_OType_PP;
    gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

    //I2S config
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    SPI_DeInit(SPI3);

    i2s.I2S_AudioFreq=FS;
    i2s.I2S_MCLKOutput=I2S_MCLKOutput_Enable;
    i2s.I2S_Mode=I2S_Mode_MasterTx;
    i2s.I2S_DataFormat=I2S_DataFormat_16b;
    i2s.I2S_Standard=I2S_Standard_Phillips;
    i2s.I2S_CPOL=I2S_CPOL_Low;
    I2S_Init(SPI3, &i2s);
    I2S_Cmd(SPI3, ENABLE);

    //I2C1 config
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    i2c.I2C_ClockSpeed=100000;
    i2c.I2C_Mode=I2C_Mode_I2C;
    i2c.I2C_Ack=I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
    i2c.I2C_DutyCycle=I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1=99;
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);

		//codec_ctrl_init(); // depending on using <--- function or following config i hear different scratches
		
    //CS43L22 config
    //Recommended Power-up sequence (page 31)

    //bring reset high
    GPIO_SetBits(GPIOD, GPIO_Pin_4);

    //wait
    for(j=0; j<20000000; j++);

    //Required initialization settings (page 32)

		CodecCommandBuffer[0] = 0x00;
		CodecCommandBuffer[1] = 0x99;
		send_codec_ctrl(CodecCommandBuffer, 2);
				
		CodecCommandBuffer[0] = 0x47;
		CodecCommandBuffer[1] = 0x80;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		reg = read_codec_register(0x32);
		CodecCommandBuffer[0] = 0x32;
		CodecCommandBuffer[1] = reg | 0x80;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		reg = read_codec_register(0x32);
		CodecCommandBuffer[0] = 0x32;
		CodecCommandBuffer[1] = reg & 0x7F;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
    CodecCommandBuffer[0] = 0x00;
		CodecCommandBuffer[1] = 0x00;
		send_codec_ctrl(CodecCommandBuffer, 2);
		//---
		
		CodecCommandBuffer[0] = 0x02;
		CodecCommandBuffer[1] = 0x01;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x04;
		CodecCommandBuffer[1] = 0xAF;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x05;
		CodecCommandBuffer[1] = 0x80;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x06;
		CodecCommandBuffer[1] = 0x07;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x02;
		CodecCommandBuffer[1] = 0x9E;
		send_codec_ctrl(CodecCommandBuffer, 2);
		//---
		
		CodecCommandBuffer[0] = 0x0A;
		CodecCommandBuffer[1] = 0x00;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x0E;
		CodecCommandBuffer[1] = 0x04;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x27;
		CodecCommandBuffer[1] = 0x00;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x1F;
		CodecCommandBuffer[1] = 0x0F;
		send_codec_ctrl(CodecCommandBuffer, 2);
		//---
		
		CodecCommandBuffer[0] = 0x1A;
		CodecCommandBuffer[1] = 0x7F;
		send_codec_ctrl(CodecCommandBuffer, 2);
		
		CodecCommandBuffer[0] = 0x1B;
		CodecCommandBuffer[1] = 0x7F;
		send_codec_ctrl(CodecCommandBuffer, 2);		
		
		GPIOD->ODR |= 0x00008000; // check blue led
}


void SPI3_IRQHandler(void){
    if(SPI_GetITStatus(SPI3, SPI_I2S_IT_TXE)){

        if(lr == 0){
            if(IsFifoEmpty()){
                SPI_I2S_SendData(SPI3, 0);
            }
            else{
                data=dequeue();
                //left channel
                SPI_I2S_SendData(SPI3, data);
                lr=1;
    }
        }
        else{
            //right channel
            SPI_I2S_SendData(SPI3, data);
            lr=0;
        }
    }
}
