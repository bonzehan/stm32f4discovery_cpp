#include "LIS302DL.h"

LIS302DL::LIS302DL()
{
  configAccelSPI();
}

void LIS302DL::configAccelSPI()
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOE, &GPIO_InitStruct);
  GPIO_SetBits(GPIOE, GPIO_Pin_3);

  SPI_InitTypeDef SPI_InitStructure;
  SPI_I2S_DeInit(SPI1);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

void LIS302DL::initAccel()
{
  GPIOE->BSRRH = 0x0008;
  accelSendByte(CTRL_REG1);
  accelSendByte(0x47);
  GPIOE->BSRRL = 0x0008;
}

LIS302DL::~LIS302DL()
{

}

int32_t LIS302DL::readAccelID()
{
  accelId = 0x00;

  SELECT_ACCEL;

  accelSendByte(WHO_AM_I | 0x80);

  accelId = accelSendByte(DUMMY_BYTE);

  DESELECT_ACCEL;

  return accelId;
}

int32_t LIS302DL::readAccelX()
{

}

int32_t LIS302DL::readAccelY()
{

}

int32_t LIS302DL::readAccelZ()
{

}

int32_t LIS302DL::readAccelXYZ()
{

}

void LIS302DL::configAccelClick()
{

}

void LIS302DL::configAccelDoubleClick()
{

}

uint8_t LIS302DL::accelSendByte(uint8_t byte)
{ 
  uint32_t timeout = LIS302DL_FLAG_TIMEOUT;
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET)
  {
    if((timeout--) == 0x00) return 0x00;
  }

  SPI_I2S_SendData(SPI1, byte);

  return accelGetByte();
}

uint8_t LIS302DL::accelGetByte()
{
  uint32_t timeout = LIS302DL_FLAG_TIMEOUT;

  while(SPI_I2S_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET)
  {
    if((timeout--) == 0x00) return 0x00;
  }

  return (uint8_t) SPI_I2S_ReceiveData(SPI1);
}

void LIS302DL::accelWrite(uint8_t reg, uint8_t *data, uint8_t count)
{

}

void LIS302DL::accelRead(uint8_t reg, uint8_t *data, uint8_t count)
{
  SELECT_ACCEL;

  reg |= 0x80;

  if( count > 1 )
  {
    reg |= 0x40;
  }

  accelSendByte( reg );

  for( uint8_t i = 0; i < count; i++ )
  {
    *data = accelSendByte( (uint8_t)DUMMY_BYTE );
    data++;
  }

  DESELECT_ACCEL;
}