/*
 * nrf24l01.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Fadi Al Rasho
 */


#include "nrf24l01.h"

extern SPI_HandleTypeDef hspi1;
#define NRF24_SPI &hspi1

// -- Laag-niveau functies --
static void CS_Select(void)   { HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET); }
static void CS_Unselect(void) { HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET); }
static void CE_Enable(void)   { HAL_GPIO_WritePin(NRF24_CE_PORT,  NRF24_CE_PIN,  GPIO_PIN_SET); }
static void CE_Disable(void)  { HAL_GPIO_WritePin(NRF24_CE_PORT,  NRF24_CE_PIN,  GPIO_PIN_RESET); }

static void nrf24_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { (reg | 0x20), value };
    CS_Select();
    HAL_SPI_Transmit(NRF24_SPI, buf, 2, 100);
    CS_Unselect();
}

static void nrf24_WriteRegMulti(uint8_t reg, uint8_t* data, uint8_t len)
{
    uint8_t cmd = reg | 0x20;
    CS_Select();
    HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);
    HAL_SPI_Transmit(NRF24_SPI, data, len, 100);
    CS_Unselect();
}

// -- Init, zoals Arduino radio.begin --
void NRF24_Init(void)
{
    CE_Disable();
    CS_Unselect();

    // Zet nRF24 registers: 5-byte adres, kanaal, 1Mbps, geen auto-ack, enz
    nrf24_WriteReg(0x00, 0x0A); // CONFIG: CRC aan, power down
    nrf24_WriteReg(0x01, 0x00); // EN_AA: auto-ack uit
    nrf24_WriteReg(0x02, 0x01); // EN_RXADDR: pipe0 aan (irrelevant als alleen TX)
    nrf24_WriteReg(0x03, 0x03); // SETUP_AW: 5 bytes adres
    nrf24_WriteReg(0x04, 0x00); // SETUP_RETR: retransmit uit
    nrf24_WriteReg(0x05, 2);    // RF_CH: kanaal 2
    nrf24_WriteReg(0x06, 0x06); // RF_SETUP: 1Mbps, 0dBm
    // ... flush tx/rx
    uint8_t flush_tx = 0xE1, flush_rx = 0xE2;
    CS_Select(); HAL_SPI_Transmit(NRF24_SPI, &flush_tx, 1, 100); CS_Unselect();
    CS_Select(); HAL_SPI_Transmit(NRF24_SPI, &flush_rx, 1, 100); CS_Unselect();
}

// -- TX mode setup (adres/kanaal) --
void NRF24_TxMode(uint8_t* address, uint8_t channel)
{
    CE_Disable();
    nrf24_WriteReg(0x05, channel); // kanaal
    nrf24_WriteRegMulti(0x10, address, 5); // TX_ADDR
    nrf24_WriteRegMulti(0x0A, address, 5); // RX_ADDR_P0
    nrf24_WriteReg(0x11, 32); // RX_PW_P0: 32 bytes
    nrf24_WriteReg(0x00, 0x0A); // PRIM_RX=0 (TX), CRC aan, power up (set_mode_TX hieronder)
}

// -- Zet in TX mode (alleen PWR_UP en PRIM_RX bit) --
static void NRF24_set_mode_TX(void)
{
    nrf24_WriteReg(0x00, 0x0A); // PRIM_RX=0 (TX), PWR_UP=1, CRC=1
}

// -- Zenden: minimalistisch! --
uint8_t NRF24_Transmit(uint8_t* data, uint8_t len)
{
    NRF24_set_mode_TX();

    CS_Select();
    uint8_t cmd = W_TX_PAYLOAD;
    HAL_SPI_Transmit(NRF24_SPI, &cmd, 1, 100);
    HAL_SPI_Transmit(NRF24_SPI, data, len, 1000);
    CS_Unselect();

    CE_Enable();
    HAL_Delay(1); // >10us pulse, 1ms = safe
    CE_Disable();

    return 1; // niet wachten op status, gewoon fire & forget
}
