#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "stm32f1xx_hal.h"

// ---- Pinnen aanpassen aan jouw hardware ----
#define NRF24_CE_PORT   GPIOB
#define NRF24_CE_PIN    GPIO_PIN_0
#define NRF24_CSN_PORT  GPIOB
#define NRF24_CSN_PIN   GPIO_PIN_1

// ---- nRF24 instructies ----
#define W_TX_PAYLOAD    0xA0

void NRF24_Init(void);
void NRF24_TxMode(uint8_t* address, uint8_t channel);
uint8_t NRF24_Transmit(uint8_t* data, uint8_t len);

#endif
